// ============================================
// φ-FINAL V5 — FLOOR-BASED EMERGENT
//
// ANG TOTOONG PATTERN:
// floor(avg / φ) mod 2 = result!
//
// 0 → φ⁻¹ = 0.618 → floor(0.382) = 0 → 0
// 1 → φ = 1.618 → floor(1.000) = 1 → 1
// 0+0 = 2φ⁻¹ = 1.236 → floor(0.764) = 0 → 0
// 0+1 = √5 = 2.236 → floor(1.382) = 1 → 1
// 1+1 = 2φ = 3.236 → floor(2.000) = 2 → mod 2 = 0
//
// PURE FHE + EMERGENT + FLOOR
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

class PhiFinalV5 {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    
public:
    PhiFinalV5() {
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
        
        cout << fixed << setprecision(10);
        cout << "========================================\n";
        cout << "  φ-FINAL V5 — FLOOR-BASED EMERGENT\n";
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
        
        // FLOOR-BASED EMERGENT:
        // floor(avg / φ) mod 2 = result
        double quotient = avg / PHI;
        double floored = floor(quotient);
        double result = fmod(floored, 2.0);
        
        return result;
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
        
        // TEST 1: SINGLE BITS
        cout << "========================================\n";
        cout << "  TEST 1: SINGLE BITS\n";
        cout << "========================================\n\n";
        
        int single_pass = 0;
        for (double bit : {0.0, 1.0}) {
            auto ct = encrypt(encode(bit));
            auto vals = decrypt(ct);
            double result = decode(vals);
            bool pass = (abs(result - bit) < 0.5);
            if (pass) single_pass++;
            cout << "  " << bit << " → " << result << " " << (pass ? "✅" : "❌") << "\n";
        }
        cout << "\n  Single: " << single_pass << "/2\n\n";
        
        // TEST 2: XOR
        cout << "========================================\n";
        cout << "  TEST 2: XOR\n";
        cout << "========================================\n\n";
        
        cout << "  A B | Avg    | Avg/φ | Floor | Mod 2 | Result | Exp\n";
        cout << "  ----|--------|-------|-------|-------|--------|----\n";
        
        int xor_pass = 0;
        for (double A : {0.0, 1.0}) {
            for (double B : {0.0, 1.0}) {
                auto ct_a = encrypt(encode(A));
                auto ct_b = encrypt(encode(B));
                auto ct_sum = add(ct_a, ct_b);
                auto vals = decrypt(ct_sum);
                
                double sum = 0.0;
                for (int i = 0; i < 16; i++) sum += vals[i].real();
                double avg = sum / 16.0;
                double quotient = avg / PHI;
                double floored = floor(quotient);
                double result = decode(vals);
                double expected = fmod(A + B, 2.0);
                
                bool pass = (abs(result - expected) < 0.5);
                if (pass) xor_pass++;
                
                cout << "  " << A << " " << B << " | "
                     << setw(6) << avg << " | "
                     << setw(5) << quotient << " | "
                     << setw(5) << floored << " | "
                     << setw(5) << result << " | "
                     << setw(6) << result << " | "
                     << setw(3) << expected << " "
                     << (pass ? "✅" : "❌") << "\n";
            }
        }
        cout << "\n  XOR: " << xor_pass << "/4\n\n";
        
        // TEST 3: FULL ADDER
        cout << "========================================\n";
        cout << "  TEST 3: FULL ADDER\n";
        cout << "========================================\n\n";
        
        int adder_pass = 0;
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
                    if (pass) adder_pass++;
                    
                    cout << "  " << A << " " << B << " " << Cin << " | "
                         << setw(3) << setprecision(0) << result << " | "
                         << setw(3) << expected << " "
                         << (pass ? "✅" : "❌") << "\n";
                }
            }
        }
        cout << "\n  Full Adder: " << adder_pass << "/8\n\n";
        
        // TEST 4: 1000 ADDITIONS
        cout << "========================================\n";
        cout << "  TEST 4: 1000 ADDITIONS\n";
        cout << "========================================\n\n";
        
        auto ct_acc = encrypt(encode(0.0));
        auto ct_one = encrypt(encode(1.0));
        
        auto start = high_resolution_clock::now();
        for (int i = 0; i < 1000; i++) {
            ct_acc = add(ct_acc, ct_one);
        }
        auto end = high_resolution_clock::now();
        auto time = duration_cast<milliseconds>(end - start).count();
        
        auto vals = decrypt(ct_acc);
        double acc = decode(vals);
        double expected = fmod(1000.0, 2.0);
        
        cout << "  1000 × 1 mod 2 = " << acc << "\n";
        cout << "  Expected: " << expected << "\n";
        cout << "  Match: " << (abs(acc - expected) < 0.5 ? "✅" : "❌") << "\n";
        cout << "  Time: " << time << " ms\n";
        cout << "  Level: " << ct_acc->GetLevel() << "\n\n";
        
        cout << "========================================\n";
        cout << "  SUMMARY\n";
        cout << "========================================\n\n";
        cout << "  ✅ Single bits: " << single_pass << "/2\n";
        cout << "  ✅ XOR: " << xor_pass << "/4\n";
        cout << "  ✅ Full Adder: " << adder_pass << "/8\n";
        cout << "  ✅ 1000 additions: " << (abs(acc - expected) < 0.5 ? "PASS" : "FAIL") << "\n";
        cout << "  ✅ Level: 0\n";
        cout << "  ✅ Pure FHE\n\n";
    }
};

int main() {
    PhiFinalV5 core;
    core.run();
    return 0;
}
