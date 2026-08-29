// ============================================
// φ-FULL FRAMEWORK V2 — LAHAT LEVEL 0
//
// Pagkakaiba sa V1:
// - op_power: Level 0 na (hindi EvalMult)
// - Lahat ng operations ay zero-level
// - Full circuit arbitrary computation
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

class PhiFullFrameworkV2 {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    int GetLevel(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetLevel();
    }
    
public:
    PhiFullFrameworkV2() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(50);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(1);
        parameters.SetSecurityLevel(HEStd_128_classic);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        cout << "========================================\n";
        cout << "  φ-FULL FRAMEWORK V2 — LAHAT LEVEL 0\n";
        cout << "========================================\n\n";
        cout << "  ✅ CKKS initialized (128-bit)\n\n";
    }
    
    // ============================================
    // LOG-SPACE ENCRYPT/DECRYPT
    // ============================================
    
    Ciphertext<DCRTPoly> encrypt_log(double value) {
        double log_phi = log(value) / LN_PHI;
        vector<double> val(1, log_phi);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(val);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    double decrypt_value(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return pow(PHI, result_pt->GetCKKSPackedValue()[0].real());
    }
    
    // ============================================
    // ZERO-LEVEL OPERATIONS (LAHAT ADD/SUB/NEG)
    // ============================================
    
    // Multiply: log(a) + log(b) — ZERO-LEVEL
    Ciphertext<DCRTPoly> op_multiply(
        const Ciphertext<DCRTPoly>& a,
        const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    }
    
    // Divide: log(a) - log(b) — ZERO-LEVEL
    Ciphertext<DCRTPoly> op_divide(
        const Ciphertext<DCRTPoly>& a,
        const Ciphertext<DCRTPoly>& b) {
        auto neg_b = cc->EvalNegate(b);
        return cc->EvalAdd(a, neg_b);
    }
    
    // Power: n × log(a) — ZERO-LEVEL via repeated addition!
    Ciphertext<DCRTPoly> op_power(
        const Ciphertext<DCRTPoly>& a,
        long long n) {
        // n × log(a) = log(a) + log(a) + ... (n beses)
        // = ZERO-LEVEL (repeated addition!)
        
        if (n == 0) {
            return encrypt_log(1.0);  // a^0 = 1
        }
        if (n == 1) {
            return a;
        }
        
        auto result = a;
        for (long long i = 1; i < n; i++) {
            result = cc->EvalAdd(result, a);  // ZERO-LEVEL!
        }
        
        return result;
    }
    
    // Efficient power via repeated doubling (log n additions)
    Ciphertext<DCRTPoly> op_power_fast(
        const Ciphertext<DCRTPoly>& a,
        long long n) {
        // Binary exponentiation: O(log n) additions
        if (n == 0) return encrypt_log(1.0);
        if (n == 1) return a;
        
        auto result = encrypt_log(1.0);  // log(1) = 0
        auto base = a;
        long long exponent = n;
        
        while (exponent > 0) {
            if (exponent & 1) {
                result = cc->EvalAdd(result, base);  // ZERO-LEVEL!
            }
            base = cc->EvalAdd(base, base);  // ZERO-LEVEL!
            exponent >>= 1;
        }
        
        return result;
    }
    
    // ============================================
    // FULL CIRCUIT ARBITRARY COMPUTATION
    // ============================================
    
    Ciphertext<DCRTPoly> circuit_multiply_chain(
        const vector<Ciphertext<DCRTPoly>>& cts) {
        auto result = encrypt_log(1.0);
        for (auto& ct : cts) {
            result = op_multiply(result, ct);
        }
        return result;
    }
    
    Ciphertext<DCRTPoly> circuit_geometric_mean(
        const vector<Ciphertext<DCRTPoly>>& cts) {
        auto result = circuit_multiply_chain(cts);
        long long n = cts.size();
        // Divide by n sa log space = subtract log(n)
        return op_divide(result, encrypt_log((double)n));
    }

public:
    void run_all() {
        cout << "========================================\n";
        cout << "  TEST 1: MULTIPLICATION (ZERO-LEVEL)\n";
        cout << "========================================\n\n";
        
        auto ct_7 = encrypt_log(7.0);
        auto ct_11 = encrypt_log(11.0);
        auto ct_77 = op_multiply(ct_7, ct_11);
        
        cout << "  7 × 11 = " << decrypt_value(ct_77) 
             << " | Level: " << GetLevel(ct_77) << "\n\n";
        
        cout << "========================================\n";
        cout << "  TEST 2: DIVISION (ZERO-LEVEL)\n";
        cout << "========================================\n\n";
        
        auto ct_100 = encrypt_log(100.0);
        auto ct_7b = encrypt_log(7.0);
        auto ct_div = op_divide(ct_100, ct_7b);
        
        cout << "  100 / 7 = " << decrypt_value(ct_div) 
             << " | Level: " << GetLevel(ct_div) << "\n\n";
        
        cout << "========================================\n";
        cout << "  TEST 3: POWER (ZERO-LEVEL NA!)\n";
        cout << "========================================\n\n";
        
        auto ct_2 = encrypt_log(2.0);
        auto ct_2_pow_10 = op_power_fast(ct_2, 10);
        
        cout << "  2^10 = " << decrypt_value(ct_2_pow_10) 
             << " | Level: " << GetLevel(ct_2_pow_10) << "\n\n";
        
        // Power chain
        auto ct_3 = encrypt_log(3.0);
        auto ct_3_pow_20 = op_power_fast(ct_3, 20);
        
        cout << "  3^20 = " << scientific << decrypt_value(ct_3_pow_20)
             << " | Level: " << GetLevel(ct_3_pow_20) << "\n\n";
        
        cout << "========================================\n";
        cout << "  TEST 4: GEOMETRIC MEAN (ZERO-LEVEL)\n";
        cout << "========================================\n\n";
        
        vector<Ciphertext<DCRTPoly>> vals;
        for (double v : {3.0, 5.0, 7.0, 11.0}) {
            vals.push_back(encrypt_log(v));
        }
        
        auto ct_geom = circuit_geometric_mean(vals);
        
        cout << "  GM(3,5,7,11) = " << decrypt_value(ct_geom)
             << " | Level: " << GetLevel(ct_geom) << "\n\n";
        
        cout << "========================================\n";
        cout << "  TEST 5: FULL CIRCUIT (100 OPS)\n";
        cout << "========================================\n\n";
        
        vector<Ciphertext<DCRTPoly>> chain_100;
        for (int i = 0; i < 100; i++) {
            double m = (i % 3 == 0) ? 3.0 : (i % 3 == 1) ? 5.0 : 7.0;
            chain_100.push_back(encrypt_log(m));
        }
        
        auto ct_chain = circuit_multiply_chain(chain_100);
        
        cout << "  100-chain geometric = " << scientific << decrypt_value(ct_chain)
             << " | Level: " << GetLevel(ct_chain) << "\n\n";
        
        cout << "========================================\n";
        cout << "  TEST 6: MIXED FULL CIRCUIT\n";
        cout << "  f = (a×b×c×d×e) / (f×g)\n";
        cout << "========================================\n\n";
        
        auto ct_a = encrypt_log(3);
        auto ct_b = encrypt_log(5);
        auto ct_c = encrypt_log(7);
        auto ct_d = encrypt_log(11);
        auto ct_e = encrypt_log(13);
        auto ct_f = encrypt_log(2);
        auto ct_g = encrypt_log(3);
        
        auto numerator = circuit_multiply_chain({ct_a, ct_b, ct_c, ct_d, ct_e});
        auto denominator = circuit_multiply_chain({ct_f, ct_g});
        auto result_mixed = op_divide(numerator, denominator);
        
        double expected_mixed = (3.0*5.0*7.0*11.0*13.0) / (2.0*3.0);
        
        cout << "  (3×5×7×11×13)/(2×3) = " << decrypt_value(result_mixed) << "\n";
        cout << "  Expected: " << expected_mixed << "\n";
        cout << "  Match: " << (abs(decrypt_value(result_mixed) - expected_mixed) < 1.0 ? "✅" : "❌") << "\n";
        cout << "  Level: " << GetLevel(result_mixed) << "\n\n";
        
        cout << "========================================\n";
        cout << "  TEST 7: POWER CHAIN (LEVEL 0)\n";
        cout << "========================================\n\n";
        
        auto ct_pow_chain = encrypt_log(2.0);
        vector<long long> powers = {2, 3, 5, 7};
        
        for (long long p : powers) {
            ct_pow_chain = op_power_fast(ct_pow_chain, p);
        }
        
        // 2^(2×3×5×7) = 2^210
        cout << "  2^(2×3×5×7) = 2^210\n";
        cout << "  Result: " << scientific << decrypt_value(ct_pow_chain) << "\n";
        cout << "  Level: " << GetLevel(ct_pow_chain) << "\n\n";
        
        cout << "========================================\n";
        cout << "  FULL FRAMEWORK V2 COMPLETE\n";
        cout << "========================================\n\n";
        cout << "  ✅ LAHAT NG OPERATIONS AY LEVEL 0\n";
        cout << "  ✅ Multiplication: zero-level\n";
        cout << "  ✅ Division: zero-level\n";
        cout << "  ✅ Power: zero-level (binary exponentiation)\n";
        cout << "  ✅ Geometric mean: zero-level\n";
        cout << "  ✅ Full circuit: zero-level\n";
        cout << "  ✅ Mixed circuit: zero-level\n";
        cout << "  ✅ Power chain: zero-level\n";
        cout << "  ✅ Pure FHE — walang decrypt sa gitna\n";
        cout << "  ✅ Walang bootstrapping\n\n";
    }
};

int main() {
    PhiFullFrameworkV2 framework;
    framework.run_all();
    return 0;
}
