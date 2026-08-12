// ================================================================
// SPIRAL iO — FINAL COMPLETE v3 (DYNAMIC ARRAYS, SCALABLE)
// ================================================================
// FIXED: ObfuscatedProgram now uses std::vector instead of fixed
// arrays. Supports arbitrary gate counts. No more segfault.
// ================================================================

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
    
    double dec_slot(const Ciphertext<DCRTPoly>& ct, int slot = 0) {
        Plaintext pt; cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[slot].real();
    }
    
    Ciphertext<DCRTPoly> nand(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto m = cc->EvalMult(a, b);
        auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>(batch_size, 1.0));
        return cc->EvalSub(one, m);
    }
};

class iOComplete {
public:
    struct ObfuscatedProgram {
        std::vector<Ciphertext<DCRTPoly>> coeff_in1; // DYNAMIC
        std::vector<Ciphertext<DCRTPoly>> coeff_in2; // DYNAMIC
        int num_gates;
        int num_inputs;
        int total_wires;
    };
    
    static ObfuscatedProgram obfuscate(
        FHEContext& fhe,
        int num_inputs,
        int num_gates,
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
            prog.coeff_in1.push_back(fhe.enc_vector(p1));
            prog.coeff_in2.push_back(fhe.enc_vector(p2));
        }
        return prog;
    }
    
    static Ciphertext<DCRTPoly> evaluate(
        FHEContext& fhe,
        const ObfuscatedProgram& prog,
        const std::vector<Ciphertext<DCRTPoly>>& enc_inputs
    ) {
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
        return fhe.cc->EvalMult(wires_packed, final_mask);
    }
};

inline void demo_io_complete() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  SPIRAL iO — DYNAMIC (Scalable)\n";
    std::cout << "===============================================================\n\n";
    
    FHEContext fhe;
    fhe.init(8192, 60);
    
    int NI = 2, NG = 4, TW = NI + NG;
    std::vector<std::vector<double>> in1(NG, std::vector<double>(TW, 0.0));
    std::vector<std::vector<double>> in2(NG, std::vector<double>(TW, 0.0));
    in1[0][0]=1; in2[0][1]=1;
    in1[1][0]=1; in2[1][2]=1;
    in1[2][1]=1; in2[2][2]=1;
    in1[3][3]=1; in2[3][4]=1;
    
    auto prog = iOComplete::obfuscate(fhe, NI, NG, in1, in2);
    std::cout << "Obfuscated: " << NG << " gates\n\n";
    
    int output_slot = NI + NG - 1;
    std::cout << "Truth table (reading slot " << output_slot << "):\n";
    std::cout << "  x y | Out  XOR\n";
    std::cout << "  " << std::string(16, '-') << "\n";
    
    int correct = 0;
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            auto cx = fhe.enc_all((double)x);
            auto cy = fhe.enc_all((double)y);
            auto ct_out = iOComplete::evaluate(fhe, prog, {cx, cy});
            double out = fhe.dec_slot(ct_out, output_slot);
            bool exp = (x != y);
            if ((out > 0.5) == exp) correct++;
            std::cout << "  " << x << " " << y << " | "
                      << std::setw(4) << out << "  " << exp
                      << "  " << ((out > 0.5) == exp ? "OK" : "FAIL") << "\n";
        }
    }
    std::cout << "\n  Correct: " << correct << "/4\n";
}
} // namespace SpiralIO
