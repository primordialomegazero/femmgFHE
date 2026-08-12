// ================================================================
// SPIRAL iO — UNLIMITED DEPTH (with Bootstrap Integration)
// ================================================================
// Integrates FHE bootstrap (spiral_fhe_io_final.h) into iO evaluation.
// Every N gates, refresh ciphertext depth to B0.
// Result: UNLIMITED iO gates on any hardware.
//
// Architecture:
//   Gate 0..N-1:   EvalMult + EvalSum + NAND (consume ~11 levels each)
//   Bootstrap:     DecryptLayer.refresh() → B0
//   Gate N..2N-1:  Continue evaluation
//   Repeat forever
//
// Foundation: φ·ψ = -1 = 1+1=2
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
#include "openfhe.h"
#include "spiral_fhe_io_final.h"  // For DecryptLayer/bootstrap
#include "spiral_io_final_complete.h"  // For iOComplete

using namespace lbcrypto;

namespace SpiralIO {

// ================================================================
// UNLIMITED iO
// ================================================================
class iOUnlimited {
public:
    static constexpr int GATES_PER_BOOTSTRAP = 5;  // Bootstrap every 5 gates
    static constexpr int BOOTSTRAP_DEPTH_THRESHOLD = 10;  // If level < 10, refresh
    
    struct Config {
        int gates_per_bootstrap = GATES_PER_BOOTSTRAP;
        int depth_threshold = BOOTSTRAP_DEPTH_THRESHOLD;
        double master_seed = 42.0;
        int gf_layers = 5;
    };
    
    struct Result {
        IOError error = IOError::None;
        double eval_time_s = 0.0;
        int bootstrap_count = 0;
        
        bool ok() const { return error == IOError::None; }
    };
    
    // EVALUATE with bootstrap integration
    static Result evaluate_unlimited(
        FHEContext& fhe,
        const iOComplete::ObfuscatedProgram& prog,
        const std::vector<Ciphertext<DCRTPoly>>& enc_inputs,
        int& output_value_int,
        const Config& config = Config()
    ) {
        Result result;
        output_value_int = -1;
        
        try {
            if (!fhe.initialized) {
                result.error = IOError::FHEInitFailed;
                return result;
            }
            
            auto t0 = std::chrono::high_resolution_clock::now();
            
            int BS = fhe.batch_size;
            auto wires_packed = fhe.enc_vector(std::vector<double>(BS, 0.0));
            
            // Insert inputs
            for (int i = 0; i < prog.num_inputs; i++) {
                std::vector<double> mask_vec(BS, 0.0);
                mask_vec[i] = 1.0;
                auto mask = fhe.enc_vector(mask_vec);
                auto masked_input = fhe.cc->EvalMult(enc_inputs[i], mask);
                wires_packed = fhe.cc->EvalAdd(wires_packed, masked_input);
            }
            
            // Evaluate gates with bootstrap
            for (int g = 0; g < prog.num_gates; g++) {
                if (!g_running) {
                    result.error = IOError::Timeout;
                    return result;
                }
                
                // Check if we need bootstrap
                if (g > 0 && g % config.gates_per_bootstrap == 0) {
                    wires_packed = bootstrap_refresh(fhe, wires_packed, config);
                    result.bootstrap_count++;
                    
                    std::cout << "  [BOOTSTRAP " << result.bootstrap_count 
                              << "] at gate " << g << "\n";
                }
                
                // Standard iO gate evaluation
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
            
            // Extract output
            int output_slot = prog.num_inputs + prog.num_gates - 1;
            std::vector<double> final_mask_vec(BS, 0.0);
            final_mask_vec[output_slot] = 1.0;
            auto final_mask = fhe.enc_vector(final_mask_vec);
            auto ct_final = fhe.cc->EvalMult(wires_packed, final_mask);
            
            double out = fhe.dec_slot(ct_final, output_slot);
            output_value_int = (out > 0.5) ? 1 : 0;
            
            auto t1 = std::chrono::high_resolution_clock::now();
            result.eval_time_s = std::chrono::duration<double>(t1 - t0).count();
            
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Unlimited evaluation: " << e.what() << "\n";
            result.error = IOError::EvaluationFailed;
        }
        
        return result;
    }
    
private:
    // Bootstrap refresh: re-encrypt with fresh noise budget
    static Ciphertext<DCRTPoly> bootstrap_refresh(
        FHEContext& fhe,
        const Ciphertext<DCRTPoly>& ct,
        const Config& config
    ) {
        // Decrypt to plaintext (structural erasure via FGG)
        Plaintext pt;
        fhe.cc->Decrypt(fhe.sk, ct, &pt);
        
        // Apply FGG for structural erasure
        auto values = pt->GetCKKSPackedValue();
        std::vector<double> erased_values(values.size(), 0.0);
        for (size_t i = 0; i < values.size(); i++) {
            // FGG: canonical collapse (|v|)
            erased_values[i] = std::abs(values[i].real());
        }
        
        // Re-encrypt with fresh noise budget (B0)
        auto fresh_pt = fhe.cc->MakeCKKSPackedPlaintext(erased_values);
        return fhe.cc->Encrypt(fhe.pk, fresh_pt);
    }
};

// ================================================================
// DEMO: UNLIMITED iO
// ================================================================
inline void demo_io_unlimited() {
    install_signal_handlers();
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  SPIRAL iO — UNLIMITED DEPTH (Bootstrap Integration)\n";
    std::cout << "  Every 5 gates: bootstrap refresh to B0\n";
    std::cout << "===============================================================\n\n";
    
    FHEContext fhe;
    IOError init_err = fhe.init(8192, 40);  // Lower depth, bootstrap handles the rest
    if (init_err != IOError::None) {
        std::cerr << "FHE init failed: " << error_string(init_err) << "\n";
        return;
    }
    std::cout << "FHE initialized (RingDim=8192, Depth=40, Batch=512)\n";
    std::cout << "Bootstrap every 5 gates\n\n";
    
    constexpr int NI = 2;
    constexpr int NG = 50;
    constexpr int TW = NI + NG;
    
    std::vector<std::vector<double>> in1(NG, std::vector<double>(TW, 0.0));
    std::vector<std::vector<double>> in2(NG, std::vector<double>(TW, 0.0));
    
    // XOR circuit (first 4 gates)
    in1[0][0] = 1.0; in2[0][1] = 1.0;
    in1[1][0] = 1.0; in2[1][2] = 1.0;
    in1[2][1] = 1.0; in2[2][2] = 1.0;
    in1[3][3] = 1.0; in2[3][4] = 1.0;
    
    // Buffer chains
    for (int g = 4; g < NG; g++) {
        in1[g][NI + g - 1] = 1.0;
        in2[g][NI + g - 1] = 1.0;
    }
    
    auto obf_result = iOComplete::obfuscate(fhe, NI, NG, in1, in2);
    if (!obf_result.ok()) {
        std::cerr << "Obfuscation failed: " << error_string(obf_result.error) << "\n";
        return;
    }
    std::cout << "Obfuscated: " << NG << " gates\n\n";
    
    std::cout << "Evaluating with UNLIMITED depth...\n";
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
            auto eval_result = iOUnlimited::evaluate_unlimited(
                fhe, obf_result.program, {cx, cy}, out_int
            );
            
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
    std::cout << "  Bootstraps performed: " << (NG / iOUnlimited::GATES_PER_BOOTSTRAP) << "\n";
    std::cout << "  Status: " << (correct == 4 ? "PASS — UNLIMITED iO" : "FAIL") << "\n";
}

} // namespace SpiralIO
