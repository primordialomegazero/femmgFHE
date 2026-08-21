// FIBONACCI ADDITION SA φ-DOMAIN
// Automatic carry propagation via golden ratio
// 0-level computation — walang bootstrapping!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FIBONACCI ADDITION SA φ-DOMAIN\n";
    std::cout << "  Automatic Carry via Golden Ratio\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;
    const double inv_phi = 1.0 / phi;
    const double sqrt5 = std::sqrt(5.0);

    // Fibonacci numbers
    auto fib = [](int n) -> double {
        double a = 0, b = 1;
        for (int i = 0; i < n; i++) {
            double temp = a + b;
            a = b;
            b = temp;
        }
        return a;
    };

    // Binet's formula: F(n) = (φ^n - (-1/φ)^n) / √5
    auto fib_binet = [&](int n) -> double {
        return (std::pow(phi, n) - std::pow(-inv_phi, n)) / sqrt5;
    };

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalSumKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2, 3, -3, 5, -5});
    
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto make_vector_ct = [&](std::vector<double> vals) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (size_t i = 0; i < vals.size() && i < slots; i++) {
            vec[i] = {vals[i], 0.0};
        }
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct, size_t idx = 0) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[idx].real();
    };

    auto ct_phi = make_ct(phi);
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(1.0);

    // ============================================
    // FIBONACCI IDENTITIES
    // ============================================
    std::cout << "FIBONACCI IDENTITIES SA φ-DOMAIN:\n";
    std::cout << "=================================\n\n";

    std::cout << "1. φ² = φ + 1\n";
    std::cout << "   " << phi_sq << " = " << phi + 1 << "\n\n";

    std::cout << "2. φ³ = 2φ + 1\n";
    std::cout << "   " << phi_cu << " = " << 2*phi + 1 << "\n\n";

    std::cout << "3. 1/φ = φ - 1\n";
    std::cout << "   " << inv_phi << " = " << phi - 1 << "\n\n";

    std::cout << "4. F(n) = (φ^n - (-1/φ)^n) / √5\n";
    for (int n = 0; n <= 10; n++) {
        std::cout << "   F(" << n << ") = " << fib(n) << " = " << fib_binet(n) << "\n";
    }
    std::cout << "\n";

    // ============================================
    // FIBONACCI CARRY PROPAGATION
    // ============================================
    std::cout << "FIBONACCI CARRY PROPAGATION:\n";
    std::cout << "============================\n\n";

    // Key identity: F(n) + F(n) = F(n+1) + F(n-2)
    std::cout << "Key identity: F(n) + F(n) = F(n+1) + F(n-2)\n";
    for (int n = 2; n <= 8; n++) {
        std::cout << "  F(" << n << ") + F(" << n << ") = " << 2*fib(n) 
                  << " = F(" << n+1 << ") + F(" << n-2 << ") = " 
                  << fib(n+1) + fib(n-2) << "\n";
    }
    std::cout << "\n";

    // ============================================
    // ZECKENDORF REPRESENTATION
    // ============================================
    std::cout << "ZECKENDORF REPRESENTATION:\n";
    std::cout << "==========================\n\n";

    auto zeckendorf = [&](int n) {
        std::vector<int> result;
        int remaining = n;
        int max_fib = 0;
        while (fib(max_fib + 2) <= remaining) max_fib++;
        
        for (int i = max_fib; i >= 0 && remaining > 0; i--) {
            if (fib(i + 2) <= remaining) {
                result.push_back(i + 2);
                remaining -= fib(i + 2);
                i--; // Skip next (non-consecutive)
            }
        }
        return result;
    };

    for (int n = 1; n <= 20; n++) {
        auto z = zeckendorf(n);
        std::cout << "  " << n << " = ";
        for (size_t i = 0; i < z.size(); i++) {
            std::cout << "F(" << z[i] << ")";
            if (i < z.size() - 1) std::cout << " + ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    // ============================================
    // φ-DOMAIN ADDITION TEST
    // ============================================
    std::cout << "φ-DOMAIN ADDITION TEST:\n";
    std::cout << "=======================\n\n";

    // Test 1: Simple addition sa φ-domain
    auto ct_a = make_ct(phi);
    auto ct_b = make_ct(phi);
    
    // a + b = 2φ (in φ-domain)
    auto ct_sum = cc->EvalAdd(ct_a, ct_b);
    
    std::cout << "Test 1: φ + φ = 2φ\n";
    std::cout << "  Result: " << decrypt_val(ct_sum) << " (expected " << 2*phi << ")\n";
    std::cout << "  Level: " << ct_sum->GetLevel() << "\n\n";

    // Test 2: φ² = φ + 1 (automatic carry)
    auto ct_phi_sq_test = make_ct(phi_sq);
    auto ct_phi_plus_one = cc->EvalAdd(ct_phi, ct_one);
    
    std::cout << "Test 2: φ² = φ + 1 (carry propagation)\n";
    std::cout << "  φ² = " << decrypt_val(ct_phi_sq_test) << "\n";
    std::cout << "  φ + 1 = " << decrypt_val(ct_phi_plus_one) << "\n";
    std::cout << "  Difference: " << std::abs(decrypt_val(ct_phi_sq_test) - decrypt_val(ct_phi_plus_one)) << "\n";
    std::cout << "  Level: " << ct_phi_plus_one->GetLevel() << "\n\n";

    // Test 3: Fibonacci addition via φ-powers
    auto ct_f4 = make_ct(fib(4));  // F(4) = 3
    auto ct_f5 = make_ct(fib(5));  // F(5) = 5
    
    // F(4) + F(5) = 3 + 5 = 8 = F(6)
    auto ct_fib_sum = cc->EvalAdd(ct_f4, ct_f5);
    
    std::cout << "Test 3: F(4) + F(5) = F(6)\n";
    std::cout << "  F(4) + F(5) = " << decrypt_val(ct_fib_sum) << "\n";
    std::cout << "  F(6) = " << fib(6) << "\n";
    std::cout << "  Match: " << (std::abs(decrypt_val(ct_fib_sum) - fib(6)) < 0.001 ? "YES" : "NO") << "\n";
    std::cout << "  Level: " << ct_fib_sum->GetLevel() << "\n\n";

    // ============================================
    // AUTOMATIC CARRY VIA OSCILLATION
    // ============================================
    std::cout << "AUTOMATIC CARRY VIA OSCILLATION:\n";
    std::cout << "================================\n\n";

    // Ang period-2 oscillator ay maaaring mag-propagate ng carry
    // φ² - x = oscillation between φ² and 0
    // Ito ay parang "bit flip" sa φ-domain
    
    auto osc = ct_zero;
    std::cout << "Oscillation sequence:\n";
    for (int i = 0; i < 8; i++) {
        osc = cc->EvalSub(ct_phi_sq, osc);
        double v = decrypt_val(osc);
        int bit = (v > phi_sq / 2) ? 1 : 0;
        std::cout << "  Step " << i << ": " << bit << " (" << v << ")\n";
    }
    std::cout << "\n";

    // ============================================
    // MULTI-SLOT FIBONACCI ADDITION
    // ============================================
    std::cout << "MULTI-SLOT FIBONACCI ADDITION:\n";
    std::cout << "==============================\n\n";

    // Fill slots with Fibonacci numbers
    std::vector<double> fib_vec(slots, 0.0);
    for (int i = 0; i < 10; i++) {
        fib_vec[i] = fib(i + 2);  // F(2), F(3), F(4), ...
    }

    auto ct_fib_vec = make_vector_ct(fib_vec);
    
    // Add with shifted version (Fibonacci addition)
    auto ct_fib_shifted = cc->EvalRotate(ct_fib_vec, 1);
    auto ct_fib_result = cc->EvalAdd(ct_fib_vec, ct_fib_shifted);
    
    std::cout << "Fibonacci vector + shifted = next Fibonacci numbers?\n";
    std::cout << "  Slot 0: " << decrypt_val(ct_fib_result, 0) << " (expected " << fib(2) + fib(3) << " = " << fib(4) << ")\n";
    std::cout << "  Slot 1: " << decrypt_val(ct_fib_result, 1) << " (expected " << fib(3) + fib(4) << " = " << fib(5) << ")\n";
    std::cout << "  Slot 2: " << decrypt_val(ct_fib_result, 2) << " (expected " << fib(4) + fib(5) << " = " << fib(6) << ")\n";
    std::cout << "  Level: " << ct_fib_result->GetLevel() << "\n\n";

    // ============================================
    // KEY INSIGHT
    // ============================================
    std::cout << "KEY INSIGHT:\n";
    std::cout << "============\n";
    std::cout << "1. Fibonacci addition ay 0-level (pure addition)\n";
    std::cout << "2. φ-domain ay may automatic carry propagation\n";
    std::cout << "3. φ² - x = period-2 oscillator = bit flip\n";
    std::cout << "4. Rotate + Add = Fibonacci convolution = 0-level\n";
    std::cout << "5. Ang combination ay maaaring mag-emulate ng Rule 110\n\n";

    std::cout << "NEXT STEP:\n";
    std::cout << "==========\n";
    std::cout << "Hanapin ang exact φ-domain polynomial na nagbibigay\n";
    std::cout << "ng Rule 110 transition na 0-level.\n";
    std::cout << "Baka: next = φ² - |sum - φ²| (absolute value trick)\n";
    std::cout << "o: next = sum - φ²·floor(sum/φ²) (mod operation)\n";
    std::cout << "o: next = φ²·sin²(π·sum/(2φ²)) (trig approximation)\n";

    return 0;
}
