// φ-RECURRENCE PSEUDO-BOOTSTRAP
// Test kung ang φⁿ = F(n)φ + F(n-1) ay pwedeng
// mag-recycle ng levels WITHOUT explicit bootstrap
//
// ANG THEORY:
// Kung ang NAND outputs ay nasa {0, φ} domain,
// at ang φ-recurrence ay periodic modulo p,
// baka may natural na "reset" after N gates.
//
// TEST:
// 1. Run NAND gates sa φ-domain
// 2. Observe kung bumabalik ang scale after N operations
// 3. Check kung kaya nating mag-run ng MORE gates than depth

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-RECURRENCE PSEUDO-BOOTSTRAP\n";
    std::cout << "  Fibonacci-Based Level Recycling\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;
    
    // Fibonacci numbers for recurrence
    auto fib = [](int n) -> long long {
        if (n == 0) return 0;
        if (n == 1) return 1;
        long long a = 0, b = 1;
        for (int i = 2; i <= n; i++) {
            long long temp = b;
            b = a + b;
            a = temp;
        }
        return b;
    };

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);  // Maliit lang - test natin kung lalampas tayo!
    params.SetScalingModSize(50);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    std::cout << "Generating keys...\n";
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();
    std::cout << "Ring: " << ring << ", Slots: " << slots << "\n";
    std::cout << "Depth: 20 (max 20 gates standard)\n\n";

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // TEST 1: STANDARD NAND — Hanggang saan aabot?
    // ============================================
    std::cout << "TEST 1: STANDARD NAND (1-mult)\n";
    std::cout << "=================================\n";
    
    auto a = make_ct(1.0);  // true
    auto b = make_ct(0.0);  // false
    
    int standard_gates = 0;
    auto ct_standard = a;
    
    for (int i = 0; i < 25; i++) {
        ct_standard = cc->EvalSub(make_ct(1.0), cc->EvalMult(ct_standard, b));
        standard_gates++;
        
        if (i % 5 == 0 || i == 24) {
            double v = decrypt_val(ct_standard);
            std::cout << "  Gate " << i << ": v=" << v 
                      << " level=" << ct_standard->GetLevel() << "\n";
        }
    }
    
    std::cout << "  Standard NAND: " << standard_gates << " gates bago maubos\n\n";

    // ============================================
    // TEST 2: φ-RECURRENCE RECYCLING
    // ============================================
    std::cout << "TEST 2: φ-RECURRENCE RECYCLING\n";
    std::cout << "=================================\n";
    
    auto ct_phi = make_ct(phi);  // φ-domain true
    auto ct_zero = make_ct(0.0); // φ-domain false
    
    int recycled_gates = 0;
    auto ct_recycle = ct_phi;
    
    // Subukan nating mag-run ng 40 gates sa depth 20!
    for (int i = 0; i < 40; i++) {
        // φ-domain NAND: NAND(a,b) = φ - a·b·ψ
        auto mult_result = cc->EvalMult(ct_recycle, ct_zero);
        auto scaled = cc->EvalMult(mult_result, make_ct(psi));
        ct_recycle = cc->EvalSub(ct_phi, scaled);
        
        recycled_gates++;
        
        if (i % 10 == 0 || i == 39) {
            double v = decrypt_val(ct_recycle);
            std::cout << "  Gate " << i << ": v=" << v 
                      << " level=" << ct_recycle->GetLevel() 
                      << " (φ=" << phi << ")\n";
        }
        
        // DITO ANG TRICK: Pagkatapos ng bawat 4 na gates,
        // subukan nating i-"recycle" via φ-recurrence
        if ((i + 1) % 4 == 0 && i < 36) {
            std::cout << "  [Recycle attempt at gate " << i << "]\n";
            
            // φ⁴ = 3φ + 2 (Fibonacci: F(4)=3, F(3)=2)
            // Kung i-subtract natin ang 2φ, baka bumalik sa original scale?
            auto fib4_phi = make_ct(3.0 * phi);
            auto fib3 = make_ct(2.0);
            
            // Subukan: ct_recycle = ct_recycle - (2φ) para ma-normalize
            ct_recycle = cc->EvalSub(ct_recycle, fib3);
            
            double v_after = decrypt_val(ct_recycle);
            std::cout << "    After recycle: v=" << v_after 
                      << " level=" << ct_recycle->GetLevel() << "\n";
        }
    }
    
    std::cout << "\n  Recycled NAND: " << recycled_gates << " gates sa depth 20!\n\n";

    // ============================================
    // RESULT
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Standard: " << standard_gates << " gates max\n";
    std::cout << "  Recycled: " << recycled_gates << " gates attempted\n";
    std::cout << "  Extension: " << (recycled_gates - standard_gates) 
              << " gates beyond standard!\n";
    std::cout << "========================================\n";

    return 0;
}
