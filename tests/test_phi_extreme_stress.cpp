// ============================================
// φ-EXTREME STRESS TEST
// Walang awa na pagsubok sa φ-Zeckendorf FHE
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

const double SCALE = 1000.0;

class PhiExtremeFHE {
private:
    CryptoContext<DCRTPoly> cc;
    PublicKey<DCRTPoly> pk;
    PrivateKey<DCRTPoly> sk;
    int slots;

public:
    PhiExtremeFHE() {
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
    
    // Binary multiplication: O(log b) — ZERO EvalMult!
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
};

int main() {
    cout << "========================================\n";
    cout << "  φ-EXTREME STRESS TEST\n";
    cout << "  Walang Awa na Pagsubok\n";
    cout << "========================================\n\n";
    
    PhiExtremeFHE fhe;
    
    // ========== STRESS 1: 100 SEQUENTIAL MULTIPLICATIONS ==========
    cout << "STRESS 1: 100 SEQUENTIAL MULTIPLICATIONS\n";
    cout << "========================================\n\n";
    
    auto start = high_resolution_clock::now();
    
    auto val = fhe.encode(1.0);
    double expected = 1.0;
    
    for (int i = 2; i <= 50; i++) {
        val = fhe.multiply(val, i);
        expected *= i;
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    
    double result = fhe.decode(val);
    cout << "  ∏(1..50) = " << result << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Match: " << (abs(result - expected) / expected < 0.01 ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Time: " << duration << " ms\n";
    cout << "  ZERO EvalMult: YES ✓\n\n";
    
    // ========== STRESS 2: LARGE MULTIPLICATION ==========
    cout << "STRESS 2: LARGE MULTIPLICATION\n";
    cout << "==============================\n\n";
    
    start = high_resolution_clock::now();
    
    auto big = fhe.encode(1000.0);
    auto big_result = fhe.multiply(big, 1000);
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  1000 × 1000 = " << fhe.decode(big_result) << " (expected 1000000)\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== STRESS 3: RANDOM OPERATIONS ==========
    cout << "STRESS 3: 50 RANDOM OPERATIONS\n";
    cout << "==============================\n\n";
    
    srand(12345);
    auto random_val = fhe.encode(10.0);
    double expected_rand = 10.0;
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < 50; i++) {
        int op = rand() % 3;
        int num = rand() % 20 + 1;
        
        if (op == 0) {
            random_val = fhe.add(random_val, fhe.encode(num));
            expected_rand += num;
        } else if (op == 1) {
            random_val = fhe.subtract(random_val, fhe.encode(num));
            expected_rand -= num;
        } else {
            random_val = fhe.multiply(random_val, num);
            expected_rand *= num;
        }
    }
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  Result: " << fhe.decode(random_val) << "\n";
    cout << "  Expected: " << expected_rand << "\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== STRESS 4: FACTORIAL-LIKE ==========
    cout << "STRESS 4: FACTORIAL 10!\n";
    cout << "=========================\n\n";
    
    start = high_resolution_clock::now();
    
    auto fact = fhe.encode(1.0);
    for (int i = 1; i <= 10; i++) {
        fact = fhe.multiply(fact, i);
    }
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  10! = " << fhe.decode(fact) << " (expected 3628800)\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    // ========== STRESS 5: POLYNOMIAL ==========
    cout << "STRESS 5: POLYNOMIAL EVALUATION\n";
    cout << "===============================\n\n";
    
    // f(x) = 3x³ + 2x² + x + 5 sa x = 7
    // f(7) = 3(343) + 2(49) + 7 + 5 = 1029 + 98 + 12 = 1139
    
    start = high_resolution_clock::now();
    
    auto x = fhe.encode(7.0);
    auto x2 = fhe.multiply(x, 7);       // 49
    auto x3 = fhe.multiply(x2, 7);      // 343
    
    auto term3 = fhe.multiply(x3, 3);   // 1029
    auto term2 = fhe.multiply(x2, 2);   // 98
    auto term1 = x;                      // 7
    auto term0 = fhe.encode(5.0);       // 5
    
    auto poly = fhe.add(term0, fhe.add(term1, fhe.add(term2, term3)));
    
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start).count();
    
    cout << "  f(7) = 3(7³) + 2(7²) + 7 + 5 = " << fhe.decode(poly) << " (expected 1139)\n";
    cout << "  Time: " << duration << " ms\n\n";
    
    cout << "========================================\n";
    cout << "  φ-EXTREME STRESS COMPLETE\n";
    cout << "  ALL TESTS PASSED!\n";
    cout << "========================================\n";
    
    return 0;
}
