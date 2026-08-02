// CYCLIC MODULUS: Test φ^L ≡ 1 mod m for CKKS
// Using m=199 where order of φ = 22
// After 22 mulY operations, ratio should return to original

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}

PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                   const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double get_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = decrypt_val(cc, kp, s.a);
    double b = decrypt_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}

int main() {
    std::cout << "\n";
    std::cout << "  CYCLIC MODULUS: Testing φ^L ≡ 1 in CKKS\n";
    std::cout << "  m=199, order of φ=22\n\n";

    // Test 1: Verify the cycle in plain math first
    std::cout << "  Plain math verification (mod 199):\n";
    long long a = 1, b = 0; // φ^0 = 1 = (1, 0)
    long long m = 199;
    
    for (int i = 0; i <= 25; i++) {
        std::cout << "  φ^" << std::setw(2) << i << " mod " << m 
                  << " = (" << a << ", " << b << ")";
        if (a == 1 && b == 0 && i > 0) {
            std::cout << " *** CYCLE AT " << i << "!";
        }
        std::cout << "\n";
        
        // mulY: (a, b) → (b, a+b mod m)
        long long new_a = b % m;
        long long new_b = (a + b) % m;
        a = new_a;
        b = new_b;
    }

    // Test 2: CKKS with modulus that includes 199 as a factor
    std::cout << "\n  Testing CKKS with φ-cycle modulus...\n";
    
    // Standard CKKS with custom parameters
    // We can't directly set moduli to include 199, but we can test
    // whether the φ-cycle property manifests in the encrypted domain
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(60);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(16384);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    auto enc_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
    auto enc_phi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.618033988749895}));
    auto enc_psi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{0.6180339887498949}));
    
    const double PHI = 1.618033988749895;
    const double PSI = 0.6180339887498949;

    // Test: apply mulY repeatedly and track the ratio
    std::cout << "\n  Applying mulY repeatedly, tracking ratio:\n";
    std::cout << "  Step | Level | Ratio         | Drift from ψ\n";
    std::cout << "  --------------------------------------------\n";
    
    PE state = {enc_phi, enc_one};
    double orig_ratio = get_ratio(cc, kp, state);
    std::cout << "  " << std::setw(4) << 0 
              << " | " << std::setw(5) << (int)state.a->GetLevel()
              << " | " << std::fixed << std::setprecision(10) << orig_ratio
              << " | " << std::scientific << std::abs(orig_ratio - PSI) << "\n";
    
    bool cycled = false;
    for (int i = 1; i <= 50; i++) {
        try {
            state = mulY(cc, state);
            double ratio = get_ratio(cc, kp, state);
            double drift = std::abs(ratio - orig_ratio);
            
            if (i <= 10 || i == 22 || i == 44 || i % 10 == 0) {
                std::cout << "  " << std::setw(4) << i 
                          << " | " << std::setw(5) << (int)state.a->GetLevel()
                          << " | " << std::fixed << std::setprecision(10) << ratio
                          << " | " << std::scientific << drift;
                
                if (drift < 0.001 && i > 1 && !cycled) {
                    std::cout << " *** CYCLE!";
                    cycled = true;
                }
                std::cout << "\n";
            }
        } catch (const std::exception& e) {
            std::cout << "  CRASHED at step " << i << ": " << e.what() << "\n";
            break;
        }
    }
    
    // Check if φ^n approaches ψ (the attractor) or cycles
    std::cout << "\n  Final ratio after 50 mulY: " 
              << std::fixed << std::setprecision(10) 
              << get_ratio(cc, kp, state) << "\n";
    std::cout << "  ψ = " << PSI << "\n";
    std::cout << "  The ratio should converge to ψ (attractor) in real arithmetic,\n";
    std::cout << "  but in a finite modulus where φ^L ≡ 1, it would cycle.\n\n";

    // Test 3: Check mulY cycle in CKKS by doing mulY then mulY_inv
    std::cout << "  Testing cycle: apply mulY^22 then check if recoverable\n";
    
    PE cycle_test = {enc_phi, enc_one};
    
    // Apply 22 mulY
    for (int i = 0; i < 22; i++) {
        cycle_test = mulY(cc, cycle_test);
    }
    double after_22 = get_ratio(cc, kp, cycle_test);
    
    // Apply 22 mulY_inv
    for (int i = 0; i < 22; i++) {
        cycle_test = mulY_inv(cc, cycle_test);
    }
    double recovered = get_ratio(cc, kp, cycle_test);
    
    std::cout << "  Original:     " << PHI << "\n";
    std::cout << "  After mulY^22: " << after_22 << "\n";
    std::cout << "  After inv^22:  " << recovered << "\n";
    std::cout << "  Recovery error: " << std::scientific << std::abs(recovered - PHI) << "\n\n";

    // Test 4: The key insight - φ^n mod m has period
    // If CKKS modulus chain uses primes with known φ-orders,
    // we can predict and exploit the cycle
    std::cout << "  KEY INSIGHT:\n";
    std::cout << "  In real numbers, mulY^n converges to ψ.\n";
    std::cout << "  In Z_m[φ] where φ^L=1, mulY^L = identity.\n";
    std::cout << "  CKKS operates on real approximations of Z_m elements.\n";
    std::cout << "  If modulus chain primes have φ-order dividing L,\n";
    std::cout << "  then the CKKS computation has a natural period.\n\n";

    return 0;
}
