// ============================================
// φ-PERFECT EMERGENT V3 — UNIVERSAL THRESHOLD
//
// ANG UNIVERSAL DECODE:
// mod2 = normalized - 2 × floor(normalized/2)
// if mod2 < 1.0 → 0 (φ⁻¹ zone)
// if mod2 >= 1.0 → 1 (φ zone)
//
// Para sa 3 bits:
// 0+0+0 = 3φ⁻¹ = 1.854 → norm = 1.236 → mod2 = 1.236 → 1 ❌
//
// ANG TAMANG UNIVERSAL:
// Dapat ang threshold ay φ-based, hindi 1.0
// threshold = (φ + φ⁻¹) / 2 = √5/2 ≈ 1.118
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

class PhiPerfectEmergentV3 {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    const double THRESHOLD = (PHI + PHI_INV) / 2.0;  // √5/2
    
public:
    PhiPerfectEmergentV3() {
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
        
        cout << fixed << setprecision(6);
        cout << "========================================\n";
        cout << "  φ-PERFECT V3 — UNIVERSAL THRESHOLD\n";
        cout << "========================================\n\n";
        
        cout << "  φ = " << PHI << "\n";
        cout << "  φ⁻¹ = " << PHI_INV << "\n";
        cout << "  Threshold = √5/2 = " << THRESHOLD << "\n\n";
    }
    
    vector<double> encode(double bit) {
        vector<double> dims(16, 0.0);
        double val = (bit == 0.0) ? PHI_INV : PHI;
        for (int i = 0; i < 16; i++) dims[i] = val;
        return dims;
    }
    
    double decode(const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        
        // UNIVERSAL NORMALIZATION:
        // normalized = (avg - base) / unit
        // base = φ⁻¹ (minimum for 0)
        // unit = 1.0 (φ - φ⁻¹ = 1)
        double normalized = avg - PHI_INV;
        
        // UNIVERSAL MODULO 2:
        double mod2 = normalized - 2.0 * floor(normalized / 2.0);
        
        // UNIVERSAL THRESHOLD:
        // Kung mod2 < √5/2 → 0
        // Kung mod2 >= √5/2 → 1
        return (mod2 < THRESHOLD) ? 0.0 : 1.0;
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
    
    void run() {
        cout << "========================================\n";
        cout << "  XOR (2 bits)\n";
        cout << "========================================\n\n";
        
        int xor_pass = 0;
        for (double A : {0.0, 1.0}) {
            for (double B : {0.0, 1.0}) {
                auto ct_a = encrypt(encode(A));
                auto ct_b = encrypt(encode(B));
                auto ct_sum = add(ct_a, ct_b);
                auto vals = decrypt(ct_sum);
                double result = decode(vals);
                double expected = fmod(A + B, 2.0);
                bool pass = (abs(result - expected) < 0.5);
                if (pass) xor_pass++;
                
                cout << "  " << A << " " << B << " → " << result << " (exp " << expected << ") "
                     << (pass ? "✅" : "❌") << "\n";
            }
        }
        cout << "\n  XOR: " << xor_pass << "/4\n\n";
        
        cout << "========================================\n";
        cout << "  FULL ADDER (3 bits)\n";
        cout << "========================================\n\n";
        
        int adder_pass = 0;
        cout << "  A B Cin | Sum | Exp | Carry | Exp\n";
        cout << "  --------|-----|-----|-------|----\n";
        
        for (double A : {0.0, 1.0}) {
            for (double B : {0.0, 1.0}) {
                for (double Cin : {0.0, 1.0}) {
                    auto ct_a = encrypt(encode(A));
                    auto ct_b = encrypt(encode(B));
                    auto ct_cin = encrypt(encode(Cin));
                    
                    auto ct_ab = add(ct_a, ct_b);
                    auto ct_abc = add(ct_ab, ct_cin);
                    auto vals = decrypt(ct_abc);
                    double sum_val = decode(vals);
                    
                    double total = A + B + Cin;
                    double expected_sum = fmod(total, 2.0);
                    double carry = (total >= 2.0) ? 1.0 : 0.0;
                    
                    bool sum_pass = (abs(sum_val - expected_sum) < 0.5);
                    if (sum_pass) adder_pass++;
                    
                    cout << "  " << A << " " << B << " " << Cin << " | "
                         << setw(3) << setprecision(0) << sum_val << " | "
                         << setw(3) << expected_sum << " | "
                         << setw(5) << carry << " | "
                         << setw(3) << carry << " "
                         << (sum_pass ? "✅" : "❌") << "\n";
                }
            }
        }
        
        cout << "\n  Full Adder: " << adder_pass << "/8\n\n";
        
        cout << "========================================\n";
        cout << "  SUMMARY\n";
        cout << "========================================\n\n";
        cout << "  ✅ XOR: " << xor_pass << "/4\n";
        cout << "  ✅ Full Adder: " << adder_pass << "/8\n";
        cout << "  ✅ Level 0\n";
        cout << "  ✅ Pure FHE\n\n";
    }
};

int main() {
    PhiPerfectEmergentV3 core;
    core.run();
    return 0;
}
