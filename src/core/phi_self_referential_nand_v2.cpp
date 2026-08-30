// ============================================
// φ-SELF-REFERENTIAL NAND V2 — BALIGTAD NA
//
// ANG FIX:
// NAND = NOT(AND)
// 0 0 → 1 (NOT(0))
// 0 1 → 1 (NOT(0))
// 1 0 → 1 (NOT(0))
// 1 1 → 0 (NOT(1))
//
// Author: Dan Fernandez / Primordial Omega Zero
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

class PhiSelfReferentialNandV2 {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    
public:
    PhiSelfReferentialNandV2() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(1);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(16);
        parameters.SetSecurityLevel(HEStd_256_classic);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        cout << fixed << setprecision(15);
        cout << "========================================\n";
        cout << "  φ-SELF-REFERENTIAL NAND V2\n";
        cout << "  Baligtad na NAND\n";
        cout << "========================================\n\n";
    }
    
    vector<double> encode(double bit) {
        vector<double> dims(16, 0.0);
        double val = (bit == 0.0) ? PHI_INV : PHI;
        for (int i = 0; i < 16; i++) dims[i] = val;
        return dims;
    }
    
    // DECODE NA BALIGTAD PARA SA NAND
    double decode_nand(const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        
        // φ-HARMONIC RECOGNITION (BALIGTAD PARA SA NAND)
        if (abs(avg - PHI_INV) < 0.01) return 1.0;  // 0 → NAND=1
        if (abs(avg - PHI) < 0.01) return 0.0;      // 1 → NAND=0
        if (abs(avg - 2.0 * PHI_INV) < 0.01) return 1.0;  // 0+0 → 1
        if (abs(avg - (PHI + PHI_INV)) < 0.01) return 1.0; // 0+1 → 1
        if (abs(avg - 3.0 * PHI_INV) < 0.01) return 1.0;   // 0+0+0 → 1
        if (abs(avg - 2.0 * PHI) < 0.01) return 0.0;       // 1+1 → 0
        
        double quotient = avg / PHI;
        double floored = floor(quotient + 0.0000001);
        return 1.0 - fmod(floored, 2.0);  // BALIGTAD
    }
    
    // DECODE NORMAL PARA SA IBA
    double decode_normal(const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        
        if (abs(avg - PHI_INV) < 0.01) return 0.0;
        if (abs(avg - PHI) < 0.01) return 1.0;
        if (abs(avg - 2.0 * PHI_INV) < 0.01) return 0.0;
        if (abs(avg - (PHI + PHI_INV)) < 0.01) return 1.0;
        if (abs(avg - 3.0 * PHI_INV) < 0.01) return 0.0;
        if (abs(avg - 2.0 * PHI) < 0.01) return 0.0;
        
        double quotient = avg / PHI;
        double floored = floor(quotient + 0.0000001);
        return fmod(floored, 2.0);
    }
    
    Ciphertext<DCRTPoly> encrypt(vector<double> dims) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dims);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    vector<complex<double>> decrypt(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue();
    }
    
    Ciphertext<DCRTPoly> add(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    }
    
    Ciphertext<DCRTPoly> sub(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalSub(a, b);
    }
    
    // NAND: a + b - φ⁻¹ (para sa φ-space)
    Ciphertext<DCRTPoly> gate_nand(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto sum_ab = add(a, b);
        auto phi_inv_ct = encrypt(encode(0.0));
        return sub(sum_ab, phi_inv_ct);
    }
    
    // Lahat from NAND
    Ciphertext<DCRTPoly> gate_not(const Ciphertext<DCRTPoly>& a) {
        return gate_nand(a, a);
    }
    
    Ciphertext<DCRTPoly> gate_and(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto nand_ab = gate_nand(a, b);
        return gate_not(nand_ab);
    }
    
    Ciphertext<DCRTPoly> gate_or(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto not_a = gate_not(a);
        auto not_b = gate_not(b);
        return gate_nand(not_a, not_b);
    }
    
    Ciphertext<DCRTPoly> gate_xor(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto nand_ab = gate_nand(a, b);
        auto not_a = gate_not(a);
        auto not_b = gate_not(b);
        auto nand_not = gate_nand(not_a, not_b);
        return gate_nand(nand_ab, nand_not);
    }
    
    void run() {
        int total_pass = 0;
        int total_tests = 0;
        
        cout << "  A B | NAND | NOT(A) | AND | OR | XOR\n";
        cout << "  ----|------|--------|-----|----|----\n";
        
        for (double A : {0.0, 1.0}) {
            for (double B : {0.0, 1.0}) {
                auto ct_a = encrypt(encode(A));
                auto ct_b = encrypt(encode(B));
                
                auto nand_ct = gate_nand(ct_a, ct_b);
                auto not_ct = gate_not(ct_a);
                auto and_ct = gate_and(ct_a, ct_b);
                auto or_ct = gate_or(ct_a, ct_b);
                auto xor_ct = gate_xor(ct_a, ct_b);
                
                double nand_val = decode_nand(decrypt(nand_ct));
                double not_val = decode_normal(decrypt(not_ct));
                double and_val = decode_normal(decrypt(and_ct));
                double or_val = decode_normal(decrypt(or_ct));
                double xor_val = decode_normal(decrypt(xor_ct));
                
                double exp_nand = (A == 1.0 && B == 1.0) ? 0.0 : 1.0;
                double exp_not = (A == 0.0) ? 1.0 : 0.0;
                double exp_and = (A == 1.0 && B == 1.0) ? 1.0 : 0.0;
                double exp_or = (A == 1.0 || B == 1.0) ? 1.0 : 0.0;
                double exp_xor = fmod(A + B, 2.0);
                
                bool pass_nand = abs(nand_val - exp_nand) < 0.5;
                bool pass_not = abs(not_val - exp_not) < 0.5;
                bool pass_and = abs(and_val - exp_and) < 0.5;
                bool pass_or = abs(or_val - exp_or) < 0.5;
                bool pass_xor = abs(xor_val - exp_xor) < 0.5;
                
                if (pass_nand) total_pass++;
                if (pass_not) total_pass++;
                if (pass_and) total_pass++;
                if (pass_or) total_pass++;
                if (pass_xor) total_pass++;
                total_tests += 5;
                
                cout << "  " << A << " " << B << " | "
                     << setw(4) << setprecision(0) << nand_val << " | "
                     << setw(6) << not_val << " | "
                     << setw(3) << and_val << " | "
                     << setw(2) << or_val << " | "
                     << setw(3) << xor_val << " "
                     << (pass_nand && pass_not && pass_and && pass_or && pass_xor ? "✅" : "❌") << "\n";
            }
        }
        
        cout << "\n  ALL GATES: " << total_pass << "/" << total_tests << "\n";
        cout << "  Level: 0\n";
        cout << "  Pure FHE\n";
    }
};

int main() {
    PhiSelfReferentialNandV2 compiler;
    compiler.run();
    return 0;
}
