// DEBUG VERSION — Print intermediate values
#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

namespace SpiralIO {

struct FHEContext {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> kp;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int batch_size;
    
    void init(uint32_t ringDim = 8192, uint32_t depth = 60) {
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
    }
    
    Ciphertext<DCRTPoly> enc_all(double v) {
        auto pt = cc->MakeCKKSPackedPlaintext(std::vector<double>(batch_size, v));
        return cc->Encrypt(pk, pt);
    }
    
    Ciphertext<DCRTPoly> enc_vector(const std::vector<double>& vec) {
        auto pt = cc->MakeCKKSPackedPlaintext(vec);
        return cc->Encrypt(pk, pt);
    }
    
    double dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    }
    
    std::vector<double> dec_slots(const Ciphertext<DCRTPoly>& ct, int n = 10) {
        Plaintext pt; cc->Decrypt(sk, ct, &pt);
        auto cv = pt->GetCKKSPackedValue();
        std::vector<double> r;
        for (int i = 0; i < n && i < (int)cv.size(); i++)
            r.push_back(cv[i].real());
        return r;
    }
    
    Ciphertext<DCRTPoly> nand(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto m = cc->EvalMult(a, b);
        auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>(batch_size, 1.0));
        return cc->EvalSub(one, m);
    }
};

class iODebug {
public:
    static constexpr int MAX_GATES = 4;
    static constexpr int MAX_INPUTS = 2;
    
    struct ObfuscatedProgram {
        Ciphertext<DCRTPoly> coeff_in1[MAX_GATES];
        Ciphertext<DCRTPoly> coeff_in2[MAX_GATES];
        int num_gates, num_inputs, total_wires;
    };
    
    static ObfuscatedProgram obfuscate(
        FHEContext& fhe,
        int num_inputs, int num_gates,
        const std::vector<std::vector<double>>& gate_in1,
        const std::vector<std::vector<double>>& gate_in2
    ) {
        ObfuscatedProgram prog;
        prog.num_inputs = num_inputs;
        prog.num_gates = num_gates;
        prog.total_wires = num_gates + num_inputs;
        int BS = fhe.batch_size;
        
        for (int g = 0; g < num_gates; g++) {
            std::vector<double> p1(BS, 0.0), p2(BS, 0.0);
            for (int w = 0; w < prog.total_wires && w < BS; w++) {
                p1[w] = gate_in1[g][w];
                p2[w] = gate_in2[g][w];
            }
            prog.coeff_in1[g] = fhe.enc_vector(p1);
            prog.coeff_in2[g] = fhe.enc_vector(p2);
        }
        return prog;
    }
    
    static Ciphertext<DCRTPoly> evaluate_debug(
        FHEContext& fhe,
        const ObfuscatedProgram& prog,
        const std::vector<Ciphertext<DCRTPoly>>& enc_inputs,
        bool verbose = true
    ) {
        int BS = fhe.batch_size;
        
        // Build initial wires_packed
        auto wires_packed = fhe.enc_vector(std::vector<double>(BS, 0.0));
        for (int i = 0; i < prog.num_inputs; i++) {
            std::vector<double> mask_vec(BS, 0.0);
            mask_vec[i] = 1.0;
            auto mask = fhe.enc_vector(mask_vec);
            auto masked_input = fhe.cc->EvalMult(enc_inputs[i], mask);
            wires_packed = fhe.cc->EvalAdd(wires_packed, masked_input);
        }
        
        if (verbose) {
            std::cout << "Initial wires_packed (first 10 slots):\n  ";
            auto dv = fhe.dec_slots(wires_packed, 10);
            for (int i = 0; i < 10; i++) std::cout << dv[i] << " ";
            std::cout << "\n\n";
        }
        
        for (int g = 0; g < prog.num_gates; g++) {
            // Check coefficient slots
            if (verbose) {
                std::cout << "Gate " << g << " coeff_in1 (first 10): ";
                auto c1 = fhe.dec_slots(prog.coeff_in1[g], 10);
                for (int i = 0; i < 10; i++) std::cout << c1[i] << " ";
                std::cout << "\n";
                
                std::cout << "Gate " << g << " coeff_in2 (first 10): ";
                auto c2 = fhe.dec_slots(prog.coeff_in2[g], 10);
                for (int i = 0; i < 10; i++) std::cout << c2[i] << " ";
                std::cout << "\n";
            }
            
            auto prod1 = fhe.cc->EvalMult(prog.coeff_in1[g], wires_packed);
            auto prod2 = fhe.cc->EvalMult(prog.coeff_in2[g], wires_packed);
            
            if (verbose) {
                std::cout << "  prod1 (first 10): ";
                auto p1 = fhe.dec_slots(prod1, 10);
                for (int i = 0; i < 10; i++) std::cout << p1[i] << " ";
                std::cout << "\n";
                
                std::cout << "  prod2 (first 10): ";
                auto p2 = fhe.dec_slots(prod2, 10);
                for (int i = 0; i < 10; i++) std::cout << p2[i] << " ";
                std::cout << "\n";
            }
            
            auto selected1 = fhe.cc->EvalSum(prod1, BS);
            auto selected2 = fhe.cc->EvalSum(prod2, BS);
            
            if (verbose) {
                std::cout << "  selected1[0] = " << fhe.dec(selected1) << "\n";
                std::cout << "  selected2[0] = " << fhe.dec(selected2) << "\n";
            }
            
            auto gate_out = fhe.nand(selected1, selected2);
            
            if (verbose) {
                std::cout << "  gate_out[0] = " << fhe.dec(gate_out) << "\n";
            }
            
            std::vector<double> mask_vec(BS, 0.0);
            mask_vec[prog.num_inputs + g] = 1.0;
            auto mask = fhe.enc_vector(mask_vec);
            auto masked_out = fhe.cc->EvalMult(gate_out, mask);
            wires_packed = fhe.cc->EvalAdd(wires_packed, masked_out);
            
            if (verbose) {
                std::cout << "  wires after gate " << g << " (first 10): ";
                auto wv = fhe.dec_slots(wires_packed, 10);
                for (int i = 0; i < 10; i++) std::cout << wv[i] << " ";
                std::cout << "\n\n";
            }
        }
        
        int output_slot = prog.num_inputs + prog.num_gates - 1;
        std::vector<double> final_mask_vec(BS, 0.0);
        final_mask_vec[output_slot] = 1.0;
        auto final_mask = fhe.enc_vector(final_mask_vec);
        auto result = fhe.cc->EvalMult(wires_packed, final_mask);
        
        if (verbose) {
            std::cout << "Final result[0] = " << fhe.dec(result) << "\n";
        }
        
        return result;
    }
};

inline void demo_debug() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  iO DEBUG — Print all intermediate values\n";
    std::cout << "===============================================================\n\n";
    
    FHEContext fhe;
    fhe.init(8192, 60);
    std::cout << "FHE initialized (Batch=" << fhe.batch_size << ")\n\n";
    
    int NI = 2, NG = 4, TW = NI + NG;
    std::vector<std::vector<double>> in1(NG, std::vector<double>(TW, 0.0));
    std::vector<std::vector<double>> in2(NG, std::vector<double>(TW, 0.0));
    in1[0][0]=1; in2[0][1]=1;
    in1[1][0]=1; in2[1][2]=1;
    in1[2][1]=1; in2[2][2]=1;
    in1[3][3]=1; in2[3][4]=1;
    
    auto prog = iODebug::obfuscate(fhe, NI, NG, in1, in2);
    
    // Test: x=0, y=1 (dapat output=1)
    std::cout << "═══ TEST: x=0, y=1 (expected output=1) ═══\n\n";
    auto cx = fhe.enc_all(0.0);
    auto cy = fhe.enc_all(1.0);
    
    std::cout << "Input x (first 10 slots): ";
    auto dx = fhe.dec_slots(cx, 10);
    for (int i = 0; i < 10; i++) std::cout << dx[i] << " ";
    std::cout << "\n\n";
    
    std::cout << "Input y (first 10 slots): ";
    auto dy = fhe.dec_slots(cy, 10);
    for (int i = 0; i < 10; i++) std::cout << dy[i] << " ";
    std::cout << "\n\n";
    
    auto ct_out = iODebug::evaluate_debug(fhe, prog, {cx, cy}, true);
    
    std::cout << "===============================================================\n";
}

} // namespace SpiralIO
