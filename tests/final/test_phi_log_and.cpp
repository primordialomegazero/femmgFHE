// LOGARITHMIC AND: Add exponents via EvalAdd (0 EvalMult!)
// Theory: encode bits as exponent of φ
// Bit 0 = ψ = φ^{-1}, Bit 1 = φ = φ^1
// AND = multiply = add exponents = EvalAdd!

#include <iostream>
#include <iomanip>
#include <cmath>
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

// Encode bit: 0 → ψ (via mulY_inv chain), 1 → φ (via mulY chain)
PE encode_bit_log(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, 
                  double bit, PE& base) {
    PE result = base;
    if (bit < 0.5) {
        // Bit 0: go toward ψ (apply mulY_inv)
        result = mulY_inv(cc, result);
    } else {
        // Bit 1: go toward φ (apply mulY)
        result = mulY(cc, result);
    }
    return result;
}

// AND via exponent addition: EvalAdd (0 EvalMult!)
PE and_log(CryptoContext<DCRTPoly>& cc, const PE& A, const PE& B) {
    // Add the representations
    // A.a + B.a, A.b + B.b
    return {cc->EvalAdd(A.a, B.a), cc->EvalAdd(A.b, B.b)};
}

int main() {
    std::cout << "\n  LOGARITHMIC AND: Add exponents via EvalAdd (0 EvalMult!)\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(5);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(8192);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    // Base: encrypted (0, 1) = ratio 0... wait, ratio should be 1 (neutral)
    // φ^0 = 1. But we need base point for exponentiation.
    // Let's use (1, 1) which decodes to ratio = 1 (neutral for multiplication)
    auto pt_a = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto pt_b = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    PE base = {cc->Encrypt(kp.publicKey, pt_a), cc->Encrypt(kp.publicKey, pt_b)};
    
    std::cout << "  Base ratio: " << get_ratio(cc, kp, base) << "\n\n";

    // Test all 4 combinations
    std::cout << "  Truth table (AND):\n";
    std::cout << "  A B | ratio_A | ratio_B | ratio_AND | decoded | expected\n";
    std::cout << "  -----------------------------------------------------\n";

    int ok = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            PE A = encode_bit_log(cc, kp, (double)a, base);
            PE B = encode_bit_log(cc, kp, (double)b, base);
            PE result = and_log(cc, A, B);
            
            double ratio_A = get_ratio(cc, kp, A);
            double ratio_B = get_ratio(cc, kp, B);
            double ratio_R = get_ratio(cc, kp, result);
            
            // Decode: compare to known values
            // ψ ≈ 0.618, φ ≈ 1.618
            // Bit 0 → ψ, Bit 1 → φ
            double decoded = (ratio_R > 1.0) ? 1.0 : 0.0;
            int expected = a & b;
            
            std::cout << "  " << a << " " << b << " | " 
                      << std::fixed << std::setprecision(3) << ratio_A << " | "
                      << ratio_B << " | " << ratio_R << " | "
                      << (int)decoded << " | " << expected;
            
            if (decoded == expected) {
                std::cout << " | OK\n";
                ok++;
            } else {
                std::cout << " | MISMATCH\n";
            }
        }
    }
    
    std::cout << "\n  Result: " << ok << "/4\n";
    
    // Check levels
    PE A = encode_bit_log(cc, kp, 1.0, base);
    PE B = encode_bit_log(cc, kp, 1.0, base);
    PE R = and_log(cc, A, B);
    std::cout << "  Level after AND: " << R.a->GetLevel() 
              << " (initial: " << base.a->GetLevel() << ")\n";
    
    if (ok == 4) {
        std::cout << "\n  LOG AND: 0 EvalMult! Kung level preserved, solved!\n\n";
    }
    
    return 0;
}
