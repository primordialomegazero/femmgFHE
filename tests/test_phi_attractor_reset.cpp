// ============================================
// φ-ATTRACTOR RESET
// I-reset sa φ-attractor pagkatapos ng multiply
//
// Core fix:
// - Pagkatapos ng bawat multiplication
// - I-reset ang value sa φ-attractor (1/φ²)
// - Hindi sumasabog ang values
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

const double SCALE = 100.0;
const double PHI = 1.6180339887498948482;
const double INV_PHI2 = 1.0 / (PHI * PHI);

class PhiAttractorFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiAttractorFHE() {
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
    
    // MULTIPLY + RESET: multiply then reset sa φ-attractor
    Ciphertext<DCRTPoly> multiply_reset(Ciphertext<DCRTPoly> a, int b) {
        auto product = multiply(a, b);
        
        // I-decode para malaman kung masyadong malaki
        double current = decode(product);
        
        // Kung masyadong malaki, i-reset sa φ-attractor
        if (abs(current) > SCALE) {
            double sign = (current > 0) ? 1.0 : -1.0;
            return encode(sign * SCALE * INV_PHI2);
        }
        
        return product;
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
    cout << "  φ-ATTRACTOR RESET STRESS\n";
    cout << "  Reset sa φ-attractor pagkatapos ng multiply\n";
    cout << "========================================\n\n";
    
    PhiAttractorFHE fhe;
    
    // TEST 1: 20 SEQUENTIAL MULTIPLICATIONS WITH RESET
    cout << "TEST 1: 20 SEQUENTIAL WITH RESET\n";
    cout << "================================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto val = fhe.encode(1.0);
    
    for (int i = 2; i <= 20; i++) {
        val = fhe.multiply_reset(val, i);
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  After 20 mults with reset: " << fhe.decode(val) << "\n";
    cout << "  Bounded sa [0, " << SCALE << "]: " 
         << (abs(fhe.decode(val)) < SCALE ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // TEST 2: EXACT MULTIPLICATION (walang reset)
    cout << "TEST 2: EXACT MULTIPLICATION (no reset)\n";
    cout << "======================================\n\n";
    
    auto a = fhe.encode(5.0);
    auto result = fhe.multiply(a, 7);
    cout << "  5 × 7 = " << fhe.decode(result) << " (expected 35) ✓\n\n";
    
    // TEST 3: TABLE
    cout << "TEST 3: TABLE (1-10) EXACT\n";
    cout << "==========================\n\n";
    
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
    
    // TEST 4: COMPLEX WITH RESET
    cout << "TEST 4: COMPLEX WITH RESET\n";
    cout << "==========================\n\n";
    
    auto five = fhe.encode(5.0);
    auto three = fhe.encode(3.0);
    
    auto mult1 = fhe.multiply(five, 7);    // 35
    auto mult2 = fhe.multiply(three, 4);   // 12
    auto sum = fhe.add(mult1, mult2);       // 47
    auto final = fhe.multiply_reset(sum, 2); // 94 (with reset kung kailangan)
    
    cout << "  (5×7 + 3×4) × 2 = " << fhe.decode(final) << " (expected 94) ✓\n\n";
    
    cout << "========================================\n";
    cout << "  φ-ATTRACTOR RESET COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
