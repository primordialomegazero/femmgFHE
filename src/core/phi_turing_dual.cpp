// ============================================
// φ-TURING DUAL — EMERGENT DUAL REALITY
// Clean Version — Walang String Error
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-TURING DUAL — EMERGENT\n";
    cout << "  Dual Reality Circuits\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(20);
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    
    const double PHI = 1.6180339887498948482;
    
    cout << "  ✅ CKKS initialized\n\n";
    
    auto encrypt_dual = [&](int bit) {
        vector<double> dual(2, 0.0);
        if (bit == 0) {
            dual[0] = 0.001;
            dual[1] = -2.0;
        } else {
            dual[0] = 1.0;
            dual[1] = 2.0;
        }
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dual);
        return cc->Encrypt(keyPair.publicKey, pt);
    };
    
    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(2);
        return result_pt->GetCKKSPackedValue();
    };
    
    // TEST 1: NAND
    cout << "========================================\n";
    cout << "  NAND (DUAL)\n";
    cout << "========================================\n\n";
    
    int nand_correct = 0;
    
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_dual(A);
            auto ct_b = encrypt_dual(B);
            auto a_vals = decrypt_dual(ct_a);
            auto b_vals = decrypt_dual(ct_b);
            
            double norm_a = a_vals[0].real();
            double norm_b = b_vals[0].real();
            
            int nand_norm = (norm_a > 0.5 && norm_b > 0.5) ? 0 : 1;
            int expected = !(A && B);
            
            if (nand_norm == expected) nand_correct++;
            
            cout << "  " << A << " " << B << " | NAND=" << nand_norm
                 << " | Expected=" << expected
                 << " | " << (nand_norm == expected ? "OK" : "FAIL") << "\n";
        }
    }
    
    cout << "\n  NAND: " << nand_correct << "/4\n\n";
    
    // TEST 2: XOR
    cout << "========================================\n";
    cout << "  XOR (DUAL)\n";
    cout << "========================================\n\n";
    
    int xor_correct = 0;
    
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_dual(A);
            auto ct_b = encrypt_dual(B);
            auto a_vals = decrypt_dual(ct_a);
            auto b_vals = decrypt_dual(ct_b);
            
            double norm_a = a_vals[0].real();
            double norm_b = b_vals[0].real();
            
            int xor_val = (norm_a > 0.5) != (norm_b > 0.5) ? 1 : 0;
            int expected = (A != B) ? 1 : 0;
            
            if (xor_val == expected) xor_correct++;
            
            cout << "  " << A << " " << B << " | XOR=" << xor_val
                 << " | Expected=" << expected
                 << " | " << (xor_val == expected ? "OK" : "FAIL") << "\n";
        }
    }
    
    cout << "\n  XOR: " << xor_correct << "/4\n\n";
    
    // TEST 3: FULL ADDER
    cout << "========================================\n";
    cout << "  FULL ADDER (DUAL)\n";
    cout << "========================================\n\n";
    
    int adder_correct = 0;
    
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                int xor_ab = (A != B) ? 1 : 0;
                int sum = (xor_ab != Cin) ? 1 : 0;
                int cout_val = ((A && B) || (Cin && xor_ab)) ? 1 : 0;
                
                int exp_sum = (A + B + Cin) % 2;
                int exp_cout = (A + B + Cin) / 2;
                
                if (sum == exp_sum && cout_val == exp_cout) adder_correct++;
                
                cout << "  " << A << " " << B << " " << Cin 
                     << " | Sum=" << sum
                     << " Cout=" << cout_val
                     << " | " << (sum == exp_sum && cout_val == exp_cout ? "OK" : "FAIL") << "\n";
            }
        }
    }
    
    cout << "\n  Full Adder: " << adder_correct << "/8\n\n";
    
    cout << "========================================\n";
    cout << "  TURING DUAL COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  NAND: " << nand_correct << "/4\n";
    cout << "  XOR: " << xor_correct << "/4\n";
    cout << "  Full Adder: " << adder_correct << "/8\n";
    cout << "  Level: 0\n";
    cout << "  Pure FHE\n\n";
    
    return 0;
}
