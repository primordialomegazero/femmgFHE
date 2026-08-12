// ================================================================
// SPIRAL iO — FINAL COMPLETE v4 (ERROR HANDLING + GRACEFUL SHUTDOWN)
// ================================================================
// - Dynamic arrays (scalable)
// - Error handling (try/catch, validation)
// - Graceful shutdown (Ctrl+C handler, cleanup)
// - Memory management (no leaks)
// ================================================================

#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <csignal>
#include <atomic>
#include <chrono>
#include <memory>
#include "openfhe.h"

using namespace lbcrypto;

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

namespace SpiralIO {

// ================================================================
// GRACEFUL SHUTDOWN
// ================================================================
static std::atomic<bool> g_running{true};

inline void signal_handler(int sig) {
    std::cout << "\n[SHUTDOWN] Signal " << sig << " received. Cleaning up...\n";
    g_running = false;
}

inline void install_signal_handlers() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
}

// ================================================================
// ERROR TYPES
// ================================================================
enum class IOError {
    None = 0,
    FHEInitFailed,
    InvalidCircuit,
    GateCountExceeded,
    WireIndexOutOfRange,
    DepthExceeded,
    EncryptionFailed,
    EvaluationFailed,
    Timeout
};

inline const char* error_string(IOError e) {
    switch (e) {
        case IOError::None: return "No error";
        case IOError::FHEInitFailed: return "FHE initialization failed";
        case IOError::InvalidCircuit: return "Invalid circuit (empty gates)";
        case IOError::GateCountExceeded: return "Gate count exceeds maximum";
        case IOError::WireIndexOutOfRange: return "Wire index out of range";
        case IOError::DepthExceeded: return "CKKS depth exceeded";
        case IOError::EncryptionFailed: return "Encryption failed";
        case IOError::EvaluationFailed: return "Evaluation failed";
        case IOError::Timeout: return "Operation timed out";
        default: return "Unknown error";
    }
}

// ================================================================
// FHE CONTEXT (with error handling)
// ================================================================
struct FHEContext {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> kp;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int batch_size;
    bool initialized = false;
    
    IOError init(uint32_t ringDim = 8192, uint32_t depth = 60) {
        try {
            CCParams<CryptoContextCKKSRNS> p;
            p.SetRingDim(ringDim);
            p.SetMultiplicativeDepth(depth);
            p.SetScalingModSize(50);
            batch_size = ringDim / 16;
            p.SetBatchSize(batch_size);
            p.SetSecretKeyDist(UNIFORM_TERNARY);
            p.SetSecurityLevel(HEStd_NotSet);
            cc = GenCryptoContext(p);
            cc->Enable(PKE); cc->Enable(KEYSWITCH);
            cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
            kp = cc->KeyGen();
            cc->EvalMultKeyGen(kp.secretKey);
            cc->EvalSumKeyGen(kp.secretKey);
            pk = kp.publicKey; sk = kp.secretKey;
            initialized = true;
            return IOError::None;
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] FHE init: " << e.what() << "\n";
            return IOError::FHEInitFailed;
        }
    }
    
    Ciphertext<DCRTPoly> enc_all(double v) {
        if (!initialized) throw std::runtime_error("FHE not initialized");
        auto pt = cc->MakeCKKSPackedPlaintext(std::vector<double>(batch_size, v));
        return cc->Encrypt(pk, pt);
    }
    
    Ciphertext<DCRTPoly> enc_vector(const std::vector<double>& vec) {
        if (!initialized) throw std::runtime_error("FHE not initialized");
        auto pt = cc->MakeCKKSPackedPlaintext(vec);
        return cc->Encrypt(pk, pt);
    }
    
    double dec_slot(const Ciphertext<DCRTPoly>& ct, int slot = 0) {
        if (!initialized) throw std::runtime_error("FHE not initialized");
        Plaintext pt; cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[slot].real();
    }
    
    Ciphertext<DCRTPoly> nand(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        if (!initialized) throw std::runtime_error("FHE not initialized");
        auto m = cc->EvalMult(a, b);
        auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>(batch_size, 1.0));
        return cc->EvalSub(one, m);
    }
};

// ================================================================
// iO COMPLETE (with error handling)
// ================================================================
class iOComplete {
public:
    static constexpr int MAX_GATES = 1000;  // Hard limit for safety
    
    struct ObfuscatedProgram {
        std::vector<Ciphertext<DCRTPoly>> coeff_in1;
        std::vector<Ciphertext<DCRTPoly>> coeff_in2;
        int num_gates = 0;
        int num_inputs = 0;
        int total_wires = 0;
    };
    
    struct Result {
        IOError error = IOError::None;
        ObfuscatedProgram program;
        double eval_time_s = 0.0;
        
        bool ok() const { return error == IOError::None; }
    };
    
    // OBFUSCATE with validation
    static Result obfuscate(
        FHEContext& fhe,
        int num_inputs,
        int num_gates,
        const std::vector<std::vector<double>>& gate_in1,
        const std::vector<std::vector<double>>& gate_in2
    ) {
        Result result;
        
        try {
            // Validation
            if (!fhe.initialized) {
                result.error = IOError::FHEInitFailed;
                return result;
            }
            if (num_gates <= 0 || num_gates > MAX_GATES) {
                result.error = IOError::GateCountExceeded;
                return result;
            }
            if (num_inputs <= 0) {
                result.error = IOError::InvalidCircuit;
                return result;
            }
            if ((int)gate_in1.size() != num_gates || (int)gate_in2.size() != num_gates) {
                result.error = IOError::InvalidCircuit;
                return result;
            }
            
            auto t0 = std::chrono::high_resolution_clock::now();
            
            result.program.num_inputs = num_inputs;
            result.program.num_gates = num_gates;
            result.program.total_wires = num_gates + num_inputs;
            int BS = fhe.batch_size;
            
            for (int g = 0; g < num_gates; g++) {
                // Check graceful shutdown
                if (!g_running) {
                    result.error = IOError::Timeout;
                    return result;
                }
                
                std::vector<double> p1(BS, 0.0), p2(BS, 0.0);
                for (int w = 0; w < result.program.total_wires && w < BS; w++) {
                    if (w < (int)gate_in1[g].size()) p1[w] = gate_in1[g][w];
                    if (w < (int)gate_in2[g].size()) p2[w] = gate_in2[g][w];
                }
                result.program.coeff_in1.push_back(fhe.enc_vector(p1));
                result.program.coeff_in2.push_back(fhe.enc_vector(p2));
            }
            
            auto t1 = std::chrono::high_resolution_clock::now();
            result.eval_time_s = std::chrono::duration<double>(t1 - t0).count();
            
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Obfuscation: " << e.what() << "\n";
            result.error = IOError::EncryptionFailed;
        }
        
        return result;
    }
    
    // EVALUATE with error handling
    static Result evaluate(
        FHEContext& fhe,
        const ObfuscatedProgram& prog,
        const std::vector<Ciphertext<DCRTPoly>>& enc_inputs,
        int& output_value_int
    ) {
        Result result;
        output_value_int = -1;
        
        try {
            if (!fhe.initialized) {
                result.error = IOError::FHEInitFailed;
                return result;
            }
            if (prog.num_gates == 0 || prog.coeff_in1.empty()) {
                result.error = IOError::InvalidCircuit;
                return result;
            }
            if ((int)enc_inputs.size() != prog.num_inputs) {
                result.error = IOError::InvalidCircuit;
                return result;
            }
            
            auto t0 = std::chrono::high_resolution_clock::now();
            
            int BS = fhe.batch_size;
            auto wires_packed = fhe.enc_vector(std::vector<double>(BS, 0.0));
            
            for (int i = 0; i < prog.num_inputs; i++) {
                std::vector<double> mask_vec(BS, 0.0);
                mask_vec[i] = 1.0;
                auto mask = fhe.enc_vector(mask_vec);
                auto masked_input = fhe.cc->EvalMult(enc_inputs[i], mask);
                wires_packed = fhe.cc->EvalAdd(wires_packed, masked_input);
            }
            
            for (int g = 0; g < prog.num_gates; g++) {
                if (!g_running) {
                    result.error = IOError::Timeout;
                    return result;
                }
                
                auto prod1 = fhe.cc->EvalMult(prog.coeff_in1[g], wires_packed);
                auto prod2 = fhe.cc->EvalMult(prog.coeff_in2[g], wires_packed);
                auto selected1 = fhe.cc->EvalSum(prod1, BS);
                auto selected2 = fhe.cc->EvalSum(prod2, BS);
                auto gate_out = fhe.nand(selected1, selected2);
                
                std::vector<double> mask_vec(BS, 0.0);
                mask_vec[prog.num_inputs + g] = 1.0;
                auto mask = fhe.enc_vector(mask_vec);
                auto masked_out = fhe.cc->EvalMult(gate_out, mask);
                wires_packed = fhe.cc->EvalAdd(wires_packed, masked_out);
            }
            
            int output_slot = prog.num_inputs + prog.num_gates - 1;
            std::vector<double> final_mask_vec(BS, 0.0);
            final_mask_vec[output_slot] = 1.0;
            auto final_mask = fhe.enc_vector(final_mask_vec);
            auto ct_final = fhe.cc->EvalMult(wires_packed, final_mask);
            
            double out = fhe.dec_slot(ct_final, output_slot);
            output_value_int = (out > 0.5) ? 1 : 0;
            
            auto t1 = std::chrono::high_resolution_clock::now();
            result.eval_time_s = std::chrono::duration<double>(t1 - t0).count();
            
        } catch (const lbcrypto::OpenFHEException& e) {
            std::cerr << "[ERROR] Evaluation (OpenFHE): " << e.what() << "\n";
            result.error = IOError::DepthExceeded;
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Evaluation: " << e.what() << "\n";
            result.error = IOError::EvaluationFailed;
        }
        
        return result;
    }
};

// ================================================================
// DEMO (with graceful shutdown)
// ================================================================
inline void demo_io_complete() {
    install_signal_handlers();
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  SPIRAL iO — DYNAMIC (Error Handling + Graceful Shutdown)\n";
    std::cout << "===============================================================\n\n";
    
    FHEContext fhe;
    IOError init_err = fhe.init(8192, 60);
    if (init_err != IOError::None) {
        std::cerr << "FHE init failed: " << error_string(init_err) << "\n";
        return;
    }
    
    int NI = 2, NG = 4, TW = NI + NG;
    std::vector<std::vector<double>> in1(NG, std::vector<double>(TW, 0.0));
    std::vector<std::vector<double>> in2(NG, std::vector<double>(TW, 0.0));
    in1[0][0]=1; in2[0][1]=1;
    in1[1][0]=1; in2[1][2]=1;
    in1[2][1]=1; in2[2][2]=1;
    in1[3][3]=1; in2[3][4]=1;
    
    auto result = iOComplete::obfuscate(fhe, NI, NG, in1, in2);
    if (!result.ok()) {
        std::cerr << "Obfuscation failed: " << error_string(result.error) << "\n";
        return;
    }
    
    std::cout << "Obfuscated: " << NG << " gates in " 
              << result.eval_time_s << "s\n\n";
    
    int output_slot = NI + NG - 1;
    std::cout << "Truth table (reading slot " << output_slot << "):\n";
    std::cout << "  x y | Out  XOR\n";
    std::cout << "  " << std::string(16, '-') << "\n";
    
    int correct = 0;
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            if (!g_running) {
                std::cout << "\n[SHUTDOWN] Interrupted.\n";
                return;
            }
            
            auto cx = fhe.enc_all((double)x);
            auto cy = fhe.enc_all((double)y);
            int out_int = -1;
            auto eval_result = iOComplete::evaluate(fhe, result.program, {cx, cy}, out_int);
            
            if (!eval_result.ok()) {
                std::cerr << "Eval failed: " << error_string(eval_result.error) << "\n";
                return;
            }
            
            bool exp = (x != y);
            if (out_int == (exp ? 1 : 0)) correct++;
            std::cout << "  " << x << " " << y << " | "
                      << std::setw(4) << out_int << "  " << exp
                      << "  " << (out_int == (exp ? 1 : 0) ? "OK" : "FAIL") << "\n";
        }
    }
    
    std::cout << "\n  Correct: " << correct << "/4\n";
    if (correct == 4) std::cout << "  STATUS: PASS\n";
    else std::cout << "  STATUS: FAIL\n";
}

} // namespace SpiralIO
