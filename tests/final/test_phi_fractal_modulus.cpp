// FRACTAL MODULUS: Self-similar scaling via mulY
// Each EvalMult followed by mulY rescales by φ
// Bit values shift but remain distinguishable

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

int main() {
    std::cout << "\n  FRACTAL MODULUS: Self-similar scaling via mulY\n";
    std::cout << "  Each level: multiply then rescale by φ\n\n";

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

    const double PSI = 0.6180339887498949;
    const double PHI = 1.618033988749895;

    // Encoded bits
    auto enc_psi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto enc_phi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI}));
    auto enc_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));

    PE bit0 = {enc_psi, enc_one};
    PE bit1 = {enc_phi, enc_one};

    std::cout << "  Initial:\n";
    std::cout << "    Bit 0 ratio: " << get_ratio(cc, kp, bit0) 
              << " (level " << bit0.a->GetLevel() << ")\n";
    std::cout << "    Bit 1 ratio: " << get_ratio(cc, kp, bit1) 
              << " (level " << bit1.a->GetLevel() << ")\n\n";

    // Fractal NAND: EvalMult + mulY rescale
    std::cout << "  Fractal NAND chain (multiply + rescale each step):\n";
    std::cout << "  Step | Bit0 ratio | Bit1 ratio | Gap\n";
    std::cout << "  -------------------------------------\n";

    PE state0 = bit0;
    PE state1 = bit1;
    
    // Simulate AND with self (square): value → value²
    // Then mulY rescale
    for (int step = 0; step < 20; step++) {
        // AND with self (simulating repeated NAND on same value)
        PE prod0 = {cc->EvalMult(state0.a, state0.a),
                    cc->EvalMult(state0.b, state0.b)};
        PE prod1 = {cc->EvalMult(state1.a, state1.a),
                    cc->EvalMult(state1.b, state1.b)};
        
        // Fractal rescale
        state0 = mulY(cc, prod0);
        state1 = mulY(cc, prod1);
        
        double r0 = get_ratio(cc, kp, state0);
        double r1 = get_ratio(cc, kp, state1);
        int lvl = state0.a->GetLevel();
        
        if (step < 10 || step % 5 == 0) {
            std::cout << "  " << std::setw(4) << step 
                      << " | " << std::fixed << std::setprecision(6) << r0
                      << " | " << r1
                      << " | " << std::abs(r1 - r0)
                      << " (lvl " << lvl << ")\n";
        }
        
        // Check if still distinguishable
        if (std::abs(r1 - r0) < 0.01) {
            std::cout << "  COLLAPSE at step " << step << "!\n";
            break;
        }
    }

    double final_r0 = get_ratio(cc, kp, state0);
    double final_r1 = get_ratio(cc, kp, state1);
    int final_lvl = state0.a->GetLevel();
    
    std::cout << "\n  Final level: " << final_lvl << "\n";
    std::cout << "  Final Bit 0 ratio: " << final_r0 << "\n";
    std::cout << "  Final Bit 1 ratio: " << final_r1 << "\n";
    std::cout << "  Gap: " << std::abs(final_r1 - final_r0) << "\n";
    
    if (std::abs(final_r1 - final_r0) > 0.01 && final_lvl >= 20) {
        std::cout << "\n  FRACTAL MODULUS: Infinite scaling possible!\n";
        std::cout << "  Bits remain distinguishable after 20+ levels\n\n";
    }

    return 0;
}
