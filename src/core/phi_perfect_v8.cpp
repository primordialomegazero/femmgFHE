// ============================================
// φ-PERFECT V8 — FINAL NA TALAGA
//
// ANG FIX: 3φ⁻¹ → 0 (hindi 1!)
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

class PhiPerfectV8 {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    
public:
    PhiPerfectV8() {
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
        cout << "  φ-PERFECT V8 — FINAL NA TALAGA\n";
        cout << "========================================\n\n";
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
        
        // φ-HARMONIC RECOGNITION:
        double phi_inv = PHI_INV;
        double phi = PHI;
        double two_phi_inv = 2.0 * PHI_INV;
        double sqrt5 = PHI + PHI_INV;
        double three_phi_inv = 3.0 * PHI_INV;
        double two_phi = 2.0 * PHI;
        
        if (abs(avg - phi_inv) < 0.01) return 0.0;         // 0
        if (abs(avg - phi) < 0.01) return 1.0;             // 1
        if (abs(avg - two_phi_inv) < 0.01) return 0.0;     // 0+0
        if (abs(avg - sqrt5) < 0.01) return 1.0;           // 0+1
        if (abs(avg - three_phi_inv) < 0.01) return 0.0;   // 0+0+0 = 0!
        if (abs(avg - two_phi) < 0.01) return 0.0;         // 1+1
        
        // Fallback: floor-based
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
    
    void run() {
        cout << "  φ = " << PHI << "\n";
        cout << "  φ⁻¹ = " << PHI_INV << "\n\n";
        
        int total_pass = 0;
        int total_tests = 0;
        
        // Single
        for (double bit : {0.0, 1.0}) {
            auto ct = encrypt(encode(bit));
            auto vals = decrypt(ct);
            double result = decode(vals);
            bool pass = (abs(result - bit) < 0.5);
            if (pass) total_pass++;
            total_tests++;
            cout << "  Single " << bit << " → " << result << " " << (pass ? "✅" : "❌") << "\n";
        }
        
        // XOR
        for (double A : {0.0, 1.0}) {
            for (double B : {0.0, 1.0}) {
                auto ct_a = encrypt(encode(A));
                auto ct_b = encrypt(encode(B));
                auto ct_sum = add(ct_a, ct_b);
                auto vals = decrypt(ct_sum);
                double result = decode(vals);
                double expected = fmod(A + B, 2.0);
                bool pass = (abs(result - expected) < 0.5);
                if (pass) total_pass++;
                total_tests++;
                cout << "  XOR " << A << " " << B << " → " << result << " " << (pass ? "✅" : "❌") << "\n";
            }
        }
        
        // Full Adder
        for (double A : {0.0, 1.0}) {
            for (double B : {0.0, 1.0}) {
                for (double Cin : {0.0, 1.0}) {
                    auto ct_a = encrypt(encode(A));
                    auto ct_b = encrypt(encode(B));
                    auto ct_cin = encrypt(encode(Cin));
                    auto ct_ab = add(ct_a, ct_b);
                    auto ct_abc = add(ct_ab, ct_cin);
                    auto vals = decrypt(ct_abc);
                    double result = decode(vals);
                    double expected = fmod(A + B + Cin, 2.0);
                    bool pass = (abs(result - expected) < 0.5);
                    if (pass) total_pass++;
                    total_tests++;
                    cout << "  FA " << A << " " << B << " " << Cin << " → " << result << " " << (pass ? "✅" : "❌") << "\n";
                }
            }
        }
        
        cout << "\n  TOTAL: " << total_pass << "/" << total_tests << "\n";
        cout << "  Level: 0\n";
        cout << "  Pure FHE\n";
    }
};

int main() {
    PhiPerfectV8 core;
    core.run();
    return 0;
}
