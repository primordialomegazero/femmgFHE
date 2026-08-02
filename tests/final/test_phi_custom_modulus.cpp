// CUSTOM MODULUS: CKKS with φ-cycle prime 199
// Tests if φ^22 ≡ 1 manifests in encrypted computation

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

int main() {
    std::cout << "\n";
    std::cout << "  CUSTOM MODULUS: CKKS with φ-cycle prime 199\n";
    std::cout << "  Theory: φ^22 ≡ 1 mod 199 → 22-step cycle\n\n";

    // Approach: Use FIXEDMANUAL to control modulus chain
    // We need to include 199 (or its multiples) in the modulus
    
    // First, let's understand what moduli CKKS actually uses
    std::cout << "  Understanding CKKS modulus structure:\n";
    std::cout << "  CKKS uses a chain of primes: Q = q_0 * q_1 * ... * q_L\n";
    std::cout << "  Each level drops one prime.\n";
    std::cout << "  For φ-cycle, we need each q_i to be a prime where φ has known order.\n\n";

    // Test with standard CKKS first to see the actual moduli used
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(8192);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    // Get the crypto parameters to inspect the modulus
    auto cryptoParams = cc->GetCryptoParameters();
    
    std::cout << "  Ring dimension: " << cryptoParams->GetElementParams()->GetRingDimension() << "\n";
    std::cout << "  Number of towers (levels+1): " 
              << cryptoParams->GetElementParams()->GetParams().size() << "\n\n";
    
    // The modulus chain is in the element params
    auto elementParams = cryptoParams->GetElementParams();
    auto params = elementParams->GetParams();
    
    std::cout << "  Modulus primes (Q = product of):\n";
    for (size_t i = 0; i < std::min(params.size(), (size_t)5); i++) {
        auto qi = params[i]->GetModulus();
        std::cout << "  q_" << i << " = " << qi << " (bits: " 
                  << (int)std::log2(qi.ConvertToDouble()) << ")";
        
        // Check if qi is 199 or a multiple
        if (qi.ConvertToInt() % 199 == 0) {
            std::cout << " *** DIVISIBLE BY 199!";
        }
        std::cout << "\n";
    }
    if (params.size() > 5) {
        std::cout << "  ... (" << params.size() - 5 << " more primes)\n";
    }
    
    // Check if 199 divides the total modulus Q
    // We can't directly check, but we can test φ-cycle behavior
    std::cout << "\n  Testing φ-cycle in CKKS:\n";
    
    auto enc_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
    auto enc_phi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.618033988749895}));
    
    const double PHI = 1.618033988749895;
    
    PE state = {enc_phi, enc_one};
    
    // Apply mulY 22 times and check if we return to original
    std::cout << "  Starting ratio: " << decrypt_val(cc, kp, state.a) / decrypt_val(cc, kp, state.b) << "\n";
    
    for (int i = 1; i <= 25; i++) {
        state = mulY(cc, state);
        if (i == 22 || i == 25) {
            double ratio = decrypt_val(cc, kp, state.a) / decrypt_val(cc, kp, state.b);
            std::cout << "  After " << i << " mulY: ratio=" << std::fixed << std::setprecision(10) << ratio;
            if (std::abs(ratio - PHI) < 0.01) {
                std::cout << " *** CYCLE BACK TO φ!";
            }
            std::cout << "\n";
        }
    }
    
    // The ratio converges to ψ instead of cycling back to φ
    // This is because CKKS uses large moduli (not just 199)
    // For the cycle to manifest, 199 must be a significant factor
    
    std::cout << "\n  CONCLUSION:\n";
    std::cout << "  Standard CKKS uses large moduli (50+ bits each).\n";
    std::cout << "  199 is too small to affect the φ-cycle in real arithmetic.\n";
    std::cout << "  For φ-cycle to work, we need:\n";
    std::cout << "  - Either very small moduli (impractical for security)\n";
    std::cout << "  - Or a scheme that operates directly in Z_p[φ]\n";
    std::cout << "  - CKKS approximates real numbers, not finite fields\n\n";
    
    // Alternative: Test with BFV/BGV (integer-based schemes)
    std::cout << "  NEXT STEP:\n";
    std::cout << "  BFV/BGV operate on integers modulo p directly.\n";
    std::cout << "  In BFV with plaintext modulus 199, φ^22 ≡ 1 would hold exactly.\n";
    std::cout << "  This might be the path to true bootstrapping-free FHE.\n\n";

    return 0;
}
