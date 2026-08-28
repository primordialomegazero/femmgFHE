// ============================================
// φ-FHE FINAL V2 — FIXED IMPLEMENTATION
//
// Multi-Ciphertext, Zero-Level, No Bootstrap
// Tamang decomposition at multiplication
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>
#include <algorithm>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class PhiFHEFinalV2 {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    
    vector<long long> lucas;
    vector<long long> class1;
    
    int GetLevel(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetLevel();
    }
    
    int GetTowers(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetElements()[0].GetNumOfElements();
    }
    
    // ============================================
    // ZERO-LEVEL MULTIPLY BY CONSTANT
    // Binary decomposition: ct × k = Σ ct × 2^i
    // ============================================
    
    Ciphertext<DCRTPoly> zero_level_multiply(
        const Ciphertext<DCRTPoly>& ct, 
        long long multiplier) {
        
        if (multiplier == 0) {
            vector<double> zeros(1, 0.0);
            Plaintext pt_zero = cc->MakeCKKSPackedPlaintext(zeros);
            return cc->Encrypt(keyPair.publicKey, pt_zero);
        }
        if (multiplier == 1) return ct;
        if (multiplier < 0) {
            auto result = zero_level_multiply(ct, -multiplier);
            return cc->EvalNegate(result);
        }
        
        // Binary decomposition
        vector<int> binary_bits;
        long long remaining = multiplier;
        while (remaining > 0) {
            binary_bits.push_back(remaining & 1);
            remaining >>= 1;
        }
        
        // Doubling chain: ct, 2ct, 4ct, 8ct, ...
        vector<Ciphertext<DCRTPoly>> doublings;
        doublings.push_back(ct);
        
        for (size_t i = 1; i < binary_bits.size(); i++) {
            auto doubled = cc->EvalAdd(doublings[i-1], doublings[i-1]);
            doublings.push_back(doubled);
        }
        
        // Sum ang required doublings
        Ciphertext<DCRTPoly> result;
        bool first = true;
        
        for (size_t i = 0; i < binary_bits.size(); i++) {
            if (binary_bits[i]) {
                if (first) {
                    result = doublings[i];
                    first = false;
                } else {
                    result = cc->EvalAdd(result, doublings[i]);
                }
            }
        }
        
        return result;
    }
    
    // ============================================
    // CLASS 1 DECOMPOSITION
    // Decompose n sa Lucas Class 1 terms
    // ============================================
    
    vector<long long> decompose_class1(long long n) {
        vector<long long> terms;
        long long remaining = n;
        
        // Greedy: largest Class 1 first
        for (int i = class1.size() - 1; i >= 0 && remaining > 0; i--) {
            if (class1[i] <= remaining) {
                terms.push_back(class1[i]);
                remaining -= class1[i];
            }
        }
        
        // Fallback: kung may natira, gamitin ang 1
        while (remaining > 0) {
            terms.push_back(1);
            remaining--;
        }
        
        return terms;
    }
    
    // ============================================
    // ENCRYPT / DECRYPT SINGLE VALUE
    // ============================================
    
    Ciphertext<DCRTPoly> encrypt_value(double value) {
        vector<double> val(1, value);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(val);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    double decrypt_value(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return result_pt->GetCKKSPackedValue()[0].real();
    }
    
    // ============================================
    // φ-MULTIPLY: a × b (zero-level)
    // I-decompose b sa Class 1, then i-multiply
    // si a sa bawat term via binary doubling
    // ============================================
    
    Ciphertext<DCRTPoly> phi_multiply(
        const Ciphertext<DCRTPoly>& ct_a, 
        long long b) {
        
        vector<long long> decomp = decompose_class1(b);
        
        Ciphertext<DCRTPoly> result;
        bool first = true;
        
        for (long long term : decomp) {
            auto partial = zero_level_multiply(ct_a, term);
            
            if (first) {
                result = partial;
                first = false;
            } else {
                result = cc->EvalAdd(result, partial);
            }
        }
        
        return result;
    }
    
public:
    PhiFHEFinalV2() {
        cout << "========================================\n";
        cout << "  φ-FHE FINAL V2 — FIXED\n";
        cout << "========================================\n\n";
        
        uint32_t multDepth = 50;
        uint32_t scaleModSize = 50;
        uint32_t batchSize = 1;
        
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(multDepth);
        parameters.SetScalingModSize(scaleModSize);
        parameters.SetBatchSize(batchSize);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        // Generate Lucas
        lucas = {2, 1};
        for (int i = 2; i <= 40; i++) {
            lucas.push_back(lucas[i-1] + lucas[i-2]);
        }
        
        // Extract Class 1: L_1, L_4, L_7, L_10, ...
        for (int i = 0; i <= 40; i++) {
            if (i % 3 == 1) {
                class1.push_back(lucas[i]);
            }
        }
        
        cout << "  ✅ CKKS initialized (single slot)\n";
        cout << "  ✅ Class 1 terms: " << class1.size() << "\n";
        cout << "  ✅ Class 1 values: ";
        for (size_t i = 0; i < min(class1.size(), size_t(6)); i++) {
            cout << class1[i] << " ";
        }
        cout << "...\n\n";
    }
    
    // ============================================
    // TEST 1: BASIC MULTIPLICATION
    // ============================================
    
    void test_basic() {
        cout << "========================================\n";
        cout << "  TEST 1: BASIC MULTIPLICATION\n";
        cout << "========================================\n\n";
        
        vector<pair<long long, long long>> tests = {
            {7, 11}, {13, 17}, {25, 30}, {100, 125}
        };
        
        int success = 0;
        
        cout << "  a × b | Result | Expected | Level | Match?\n";
        cout << "  ------|--------|----------|-------|-------\n";
        
        for (auto& [a, b] : tests) {
            auto ct_a = encrypt_value((double)a);
            auto ct_result = phi_multiply(ct_a, b);
            
            double result = decrypt_value(ct_result);
            double expected = (double)(a * b);
            bool match = abs(result - expected) < max(1.0, expected * 0.001);
            
            if (match) success++;
            
            cout << "  " << setw(4) << a << "×" << setw(4) << b << " | "
                 << setw(6) << fixed << setprecision(1) << result << " | "
                 << setw(8) << expected << " | "
                 << setw(5) << GetLevel(ct_result) << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
        
        cout << "\n  Basic: " << success << "/" << tests.size() << "\n\n";
    }
    
    // ============================================
    // TEST 2: LARGE MULTIPLICATION
    // ============================================
    
    void test_large() {
        cout << "========================================\n";
        cout << "  TEST 2: LARGE MULTIPLICATION\n";
        cout << "========================================\n\n";
        
        vector<pair<long long, long long>> tests = {
            {500, 750}, {1000, 1500}, {5000, 7000}
        };
        
        int success = 0;
        
        cout << "  a × b | Result | Expected | Level | Match?\n";
        cout << "  ------|--------|----------|-------|-------\n";
        
        for (auto& [a, b] : tests) {
            auto ct_a = encrypt_value((double)a);
            auto ct_result = phi_multiply(ct_a, b);
            
            double result = decrypt_value(ct_result);
            double expected = (double)(a * b);
            bool match = abs(result - expected) < max(1.0, expected * 0.001);
            
            if (match) success++;
            
            cout << "  " << setw(5) << a << "×" << setw(5) << b << " | "
                 << setw(8) << fixed << setprecision(0) << result << " | "
                 << setw(10) << expected << " | "
                 << setw(5) << GetLevel(ct_result) << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
        
        cout << "\n  Large: " << success << "/" << tests.size() << "\n\n";
    }
    
    // ============================================
    // TEST 3: FLOATING POINT
    // ============================================
    
    void test_float() {
        cout << "========================================\n";
        cout << "  TEST 3: FLOATING POINT\n";
        cout << "========================================\n\n";
        
        vector<pair<double, double>> tests = {
            {2.5, 3.0}, {1.5, 2.5}, {0.5, 0.75}, {10.5, 15.5}
        };
        
        int success = 0;
        
        cout << "  a × b | Result | Expected | Level | Match?\n";
        cout << "  ------|--------|----------|-------|-------\n";
        
        for (auto& [a, b] : tests) {
            long long a_scaled = (long long)round(a * 100.0);
            long long b_scaled = (long long)round(b * 100.0);
            
            auto ct_a = encrypt_value((double)a_scaled);
            auto ct_result = phi_multiply(ct_a, b_scaled);
            
            double result = decrypt_value(ct_result) / 10000.0;
            double expected = a * b;
            bool match = abs(result - expected) < max(0.01, expected * 0.001);
            
            if (match) success++;
            
            cout << "  " << setw(4) << fixed << setprecision(1) << a
                 << "×" << setw(4) << b << " | "
                 << setw(6) << setprecision(2) << result << " | "
                 << setw(8) << expected << " | "
                 << setw(5) << GetLevel(ct_result) << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
        
        cout << "\n  Float: " << success << "/" << tests.size() << "\n\n";
    }
    
    // ============================================
    // TEST 4: CHAINED OPERATIONS
    // ============================================
    
    void test_chained() {
        cout << "========================================\n";
        cout << "  TEST 4: CHAINED OPERATIONS\n";
        cout << "========================================\n\n";
        
        vector<long long> multipliers = {3, 5, 7, 11, 13};
        auto ct = encrypt_value(2.0);
        long long expected = 2;
        bool all_valid = true;
        
        cout << "  Step | Multiplier | Value | Level | Match?\n";
        cout << "  -----|-----------|-------|-------|-------\n";
        
        for (size_t i = 0; i < multipliers.size(); i++) {
            ct = phi_multiply(ct, multipliers[i]);
            expected *= multipliers[i];
            
            double result = decrypt_value(ct);
            bool match = abs(result - expected) < 1.0;
            
            if (!match) all_valid = false;
            
            cout << "  " << setw(4) << i + 1 << " | "
                 << setw(9) << multipliers[i] << " | "
                 << setw(5) << fixed << setprecision(0) << result << " | "
                 << setw(5) << GetLevel(ct) << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
        
        cout << "\n  Chained: " << (all_valid ? "✅ ALL VALID" : "❌ FAILED") << "\n";
        cout << "  Final value: " << expected << "\n";
        cout << "  Level: " << GetLevel(ct) << "\n";
        cout << "  Towers: " << GetTowers(ct) << "\n\n";
    }
    
    // ============================================
    // TEST 5: BATCH (MULTI-CT)
    // ============================================
    
    void test_batch() {
        cout << "========================================\n";
        cout << "  TEST 5: BATCH (MULTI-CT)\n";
        cout << "========================================\n\n";
        
        vector<double> a = {1, 2, 3, 4, 5, 6, 7, 8};
        vector<double> b = {2, 3, 5, 7, 11, 13, 17, 19};
        
        int success = 0;
        
        cout << "  # | a × b | Result | Expected | Level | Match?\n";
        cout << "  --|-------|--------|----------|-------|-------\n";
        
        for (size_t i = 0; i < a.size(); i++) {
            auto ct_a = encrypt_value(a[i]);
            auto ct_result = phi_multiply(ct_a, (long long)b[i]);
            
            double result = decrypt_value(ct_result);
            double expected = a[i] * b[i];
            bool match = abs(result - expected) < 0.01;
            
            if (match) success++;
            
            cout << "  " << setw(2) << i << " | "
                 << setw(3) << fixed << setprecision(0) << a[i] << "×"
                 << setw(3) << b[i] << " | "
                 << setw(6) << result << " | "
                 << setw(8) << expected << " | "
                 << setw(5) << GetLevel(ct_result) << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
        
        cout << "\n  Batch: " << success << "/" << a.size() << "\n\n";
    }

public:
    void run_all() {
        test_basic();
        test_large();
        test_float();
        test_chained();
        test_batch();
        
        cout << "========================================\n";
        cout << "  φ-FHE FINAL V2 COMPLETE\n";
        cout << "========================================\n\n";
    }
};

int main() {
    PhiFHEFinalV2 framework;
    framework.run_all();
    return 0;
}
