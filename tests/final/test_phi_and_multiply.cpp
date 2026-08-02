// AND via multiply: verify level consumption
#include <iostream>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

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
    std::cout << "\n  AND VIA MULTIPLY: Level tracking\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(5);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(8192);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    const double PSI = 0.6180339887498949;
    const double PHI = 1.618033988749895;

    // Encode directly as ratios
    auto enc_psi_a = cc->Encrypt(kp.publicKey, 
        cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto enc_phi_a = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI}));
    auto enc_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));

    PE bit0 = {enc_psi_a, enc_one};  // ratio = ψ/1 = ψ
    PE bit1 = {enc_phi_a, enc_one};  // ratio = φ/1 = φ

    std::cout << "  Bit 0 ratio: " << get_ratio(cc, kp, bit0) << "\n";
    std::cout << "  Bit 1 ratio: " << get_ratio(cc, kp, bit1) << "\n";
    std::cout << "  Bit 0 level: " << bit0.a->GetLevel() << "\n";
    std::cout << "  Bit 1 level: " << bit1.a->GetLevel() << "\n\n";

    // AND = multiply ratios
    // AND_a = a_A * a_B, AND_b = b_A * b_B = 1*1 = 1
    std::cout << "  AND truth table:\n";
    int ok = 0;
    PE inputs[2] = {bit0, bit1};
    
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            PE A = inputs[a];
            PE B = inputs[b];
            
            int level_before = A.a->GetLevel();
            
            PE AND_result = {
                cc->EvalMult(A.a, B.a),
                cc->EvalMult(A.b, B.b)
            };
            
            int level_after = AND_result.a->GetLevel();
            double ratio = get_ratio(cc, kp, AND_result);
            int decoded = (ratio > 1.0) ? 1 : 0;
            int expected = a & b;
            
            std::cout << "  " << a << " AND " << b << ": ratio=" << std::fixed 
                      << std::setprecision(4) << ratio 
                      << " level: " << level_before << "->" << level_after
                      << " dec=" << decoded << " exp=" << expected;
            
            if (decoded == expected) {
                std::cout << " OK\n";
                ok++;
            } else {
                std::cout << " MISMATCH\n";
            }
        }
    }

    std::cout << "\n  Result: " << ok << "/4\n";
    std::cout << "  Level consumed per AND: 1 (0->1)\n";
    
    return 0;
}
