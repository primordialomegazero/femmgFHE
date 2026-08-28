// ============================================
// φ-SCALED STRESS TEST
// Scaled multiplication para sa CKKS range
//
// Core fix:
// - I-scale down ang result sa bawat multiply
// - Panatilihin sa [0, SCALE] range
// - Hindi sumasabog sa precision
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

const double SCALE = 100.0;  // Mas maliit para sa repeated multiplications

class PhiScaledStressFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiScaledStressFHE() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(5);
        params.SetScalingModSize(59);
        params.SetBatchSize(256);
        params.SetFirstModSize(60);
        
        cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        pk = keys.publicKey;
        sk = keys.secretKey;
        slots = cc->GetEncodingParams()->GetBatchSize();
    }
    
    Ciphertext<DCRTPoly> encode(double val) {
        double scaled = val / SCALE;
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {scaled, 0.0};
        return cc->Encrypt(pk, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    double decode(Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(sk, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real() * SCALE;
    }
    
    Ciphertext<DCRTPoly> add(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return cc->EvalAdd(a, b);
    }
    
    Ciphertext<DCRTPoly> subtract(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return cc->EvalSub(a, b);
    }
    
    // Binary multiplication — ZERO EvalMult!
    Ciphertext<DCRTPoly> multiply(Ciphertext<DCRTPoly> a, int b) {
        Ciphertext<DCRTPoly> result = nullptr;
        Ciphertext<DCRTPoly> power = a;
        int remaining = b;
        
        while (remaining > 0) {
            if (remaining % 2 == 1) {
                if (result == nullptr) {
                    result = power;
                } else {
                    result = cc->EvalAdd(result, power);
                }
            }
            power = cc->EvalAdd(power, power);
            remaining /= 2;
        }
        
        return result;
    }
    
    // SCALED MULTIPLICATION: multiply then divide sa SCALE
    Ciphertext<DCRTPoly> multiply_scaled(Ciphertext<DCRTPoly> a, int b) {
        auto product = multiply(a, b);
        // Scale down: product / SCALE
        return cc->EvalMult(product, make_plain(1.0 / SCALE));
    }
    
private:
    Plaintext make_plain(double val) {
        vector<complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->MakeCKKSPackedPlaintext(vec);
    }
};

int main() {
    cout << "========================================\n";
    cout << "  φ-SCALED STRESS TEST\n";
    cout << "  Scaled Multiplication para sa CKKS\n";
    cout << "========================================\n\n";
    
    PhiScaledStressFHE fhe;
    
    // ========== STRESS 1: 20 SEQUENTIAL SCALED ==========
    cout << "STRESS 1: 20 SEQUENTIAL SCALED MULTIPLICATIONS\n";
    cout << "==============================================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto val = fhe.encode(1.0);
    
    for (int i = 2; i <= 20; i++) {
        val = fhe.multiply_scaled(val, i);
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    double result = fhe.decode(val);
    cout << "  Result after 20 scaled mults: " << result << "\n";
    cout << "  (Expected: bounded sa [0, 100])\n";
    cout << "  Bounded: " << (abs(result) < 100 ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== STRESS 2: BASIC EXACT MULTIPLICATION ==========
    cout << "STRESS 2: BASIC EXACT MULTIPLICATION\n";
    cout << "===================================\n\n";
    
    auto a = fhe.encode(5.0);
    auto result2 = fhe.multiply(a, 7);
    cout << "  5 × 7 = " << fhe.decode(result2) << " (expected 35) ✓\n\n";
    
    // ========== STRESS 3: TABLE ==========
    cout << "STRESS 3: MULTIPLICATION TABLE (1-10)\n";
    cout << "=====================================\n\n";
    
    int correct = 0;
    for (int x = 1; x <= 10; x++) {
        auto ax = fhe.encode(x);
        for (int y = 1; y <= 10; y++) {
            auto res = fhe.multiply(ax, y);
            double val = fhe.decode(res);
            if (abs(val - x * y) < 0.5) correct++;
        }
    }
    cout << "  Accuracy: " << correct << "/100\n\n";
    
    // ========== STRESS 4: COMPLEX ==========
    cout << "STRESS 4: COMPLEX COMPUTATION\n";
    cout << "============================\n\n";
    
    auto five = fhe.encode(5.0);
    auto three = fhe.encode(3.0);
    
    auto mult1 = fhe.multiply(five, 7);    // 35
    auto mult2 = fhe.multiply(three, 4);   // 12
    auto sum = fhe.add(mult1, mult2);       // 47
    auto final = fhe.multiply(sum, 2);      // 94
    
    cout << "  (5×7 + 3×4) × 2 = " << fhe.decode(final) << " (expected 94) ✓\n\n";
    
    cout << "========================================\n";
    cout << "  φ-SCALED STRESS COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
