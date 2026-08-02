// FRACTAL 1-EM NAND CHAIN: Single EvalMult per gate + mulY rescale
// Tests how deep we can go with fractal modulus on RingDim 16384

#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
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

// 1-EM NAND: AND (1 EvalMult) + NOT (0 EvalMult) + mulY rescale (0 EvalMult)
PE nand_fractal(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    // AND: only multiply a-components (b=1 always, skip)
    auto AND_a = cc->EvalMult(A.a, B.a);  // 1 EvalMult
    
    // NOT: (1 - AND_a, 1) — but we need b=1 for the output
    // We have A.b and B.b both = 1 at same level
    // AND_b = A.b × B.b = 1 — but we need level-matched 1
    auto AND_b = cc->EvalMult(A.b, B.b);  // 1 EvalMult (level match for NOT)
    
    // NOT via subtraction
    PE nand_result = {cc->EvalSub(AND_b, AND_a), AND_b};
    
    // Fractal rescale: mulY shifts level-0 info into a
    PE rescaled = mulY(cc, nand_result);
    
    return rescaled;
}

int main() {
    std::cout << "\n  FRACTAL 1-EM NAND CHAIN: Single EvalMult per gate + mulY rescale\n";
    std::cout << "  RingDim=16384, testing maximum depth\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(500);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(16384);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    const double PSI = 0.6180339887498949;

    // Create encrypted 1 (always needed for NOT)
    auto enc_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
    
    // Test: chain of NANDs feeding into each other
    // Start with two known values: 1 AND 1 = 0, then 0 AND 1 = 1, etc.
    
    // Bit 1 encoded as φ
    auto enc_phi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.618033988749895}));
    
    PE A = {enc_phi, enc_one};  // Bit 1
    PE B = {enc_phi, enc_one};  // Bit 1
    
    std::cout << "  Initial A ratio: " << get_ratio(cc, kp, A) 
              << " (level " << A.a->GetLevel() << ")\n";
    std::cout << "  Initial B ratio: " << get_ratio(cc, kp, B) 
              << " (level " << B.a->GetLevel() << ")\n\n";
    
    std::cout << "  Chain: NAND(A,B) → becomes new A, B stays as Bit 1\n";
    std::cout << "  Expected: 1 NAND 1 = 0, 0 NAND 1 = 1, alternating\n\n";
    
    std::cout << "  Step | Level | Ratio   | Decoded | Expected\n";
    std::cout << "  -----------------------------------------\n";
    
    PE current = A;
    PE constant_one = {enc_phi, enc_one};  // always Bit 1
    
    int max_steps = 0;
    
    for (int step = 0; step < 100; step++) {
        try {
            // NAND(current, constant_one)
            PE result = nand_fractal(cc, current, constant_one);
            
            int lvl = result.a->GetLevel();
            double ratio = get_ratio(cc, kp, result);
            int decoded = (ratio > 1.0) ? 1 : 0;
            int expected = (step % 2 == 0) ? 0 : 1;  // alternating
            
            if (step < 15 || step % 10 == 0) {
                std::cout << "  " << std::setw(4) << step 
                          << " | " << std::setw(5) << lvl
                          << " | " << std::fixed << std::setprecision(6) << ratio
                          << " | " << decoded
                          << " | " << expected;
                
                if (decoded == expected) {
                    std::cout << " OK\n";
                } else {
                    std::cout << " MISMATCH\n";
                }
            }
            
            max_steps = step;
            current = result;
            
        } catch (const std::exception& e) {
            std::cout << "\n  CRASHED at step " << step << ": " << e.what() << "\n";
            break;
        }
    }
    
    std::cout << "\n  Maximum steps before crash: " << max_steps << "\n";
    std::cout << "  Total EvalMults: " << (max_steps + 1) * 2 << " (2 per fractal NAND)\n\n";
    
    return 0;
}
