// FRACTAL DECODE: mulY rescale with mulY_inv recovery
// Apply mulY_inv after computation to recover original ratio

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

int main() {
    std::cout << "\n  FRACTAL DECODE: mulY rescale + mulY_inv recovery\n\n";

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

    auto enc_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));
    auto enc_phi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.618033988749895}));
    auto enc_psi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{0.6180339887498949}));

    PE bit0 = {enc_psi, enc_one};
    PE bit1 = {enc_phi, enc_one};

    std::cout << "  Testing: apply mulY n times, then mulY_inv n times\n";
    std::cout << "  Should recover original ratio\n\n";
    std::cout << "  N | Original | After mulY^N | After recovery | Match?\n";
    std::cout << "  -------------------------------------------------\n";

    for (int n : {1, 5, 10, 20, 50, 99}) {
        PE state = bit1;  // start with bit 1 (ratio = φ)
        double orig = get_ratio(cc, kp, state);
        
        // Apply mulY n times
        for (int i = 0; i < n; i++) {
            state = mulY(cc, state);
        }
        double after_mulY = get_ratio(cc, kp, state);
        
        // Recover with mulY_inv n times
        for (int i = 0; i < n; i++) {
            state = mulY_inv(cc, state);
        }
        double recovered = get_ratio(cc, kp, state);
        
        bool match = std::abs(recovered - orig) < 0.01;
        
        std::cout << "  " << std::setw(2) << n 
                  << " | " << std::fixed << std::setprecision(4) << orig
                  << "   | " << std::setprecision(4) << after_mulY
                  << "      | " << std::setprecision(4) << recovered
                  << "        | " << (match ? "YES" : "NO") << "\n";
    }

    // Now test with an actual NAND: 1 NAND 1 = 0
    std::cout << "\n  Testing fractal NAND with decode:\n";
    std::cout << "  1 NAND 1 = 0 (should decode to bit 0)\n";
    
    PE A = bit1;
    PE B = bit1;
    
    // NAND: AND + NOT + mulY rescale
    auto AND_a = cc->EvalMult(A.a, B.a);
    auto AND_b = cc->EvalMult(A.b, B.b);
    PE nand_raw = {cc->EvalSub(AND_b, AND_a), AND_b};
    
    double raw_ratio = get_ratio(cc, kp, nand_raw);
    int raw_bit = (raw_ratio > 1.0) ? 1 : 0;
    std::cout << "  After NAND (no rescale): ratio=" << raw_ratio 
              << " decoded=" << raw_bit << " (expected 0)\n";
    
    PE nand_fractal = mulY(cc, nand_raw);
    double fractal_ratio = get_ratio(cc, kp, nand_fractal);
    int fractal_bit = (fractal_ratio > 1.0) ? 1 : 0;
    std::cout << "  After mulY rescale:      ratio=" << fractal_ratio
              << " decoded=" << fractal_bit << " (WRONG due to shift)\n";
    
    PE recovered = mulY_inv(cc, nand_fractal);
    double recovered_ratio = get_ratio(cc, kp, recovered);
    int recovered_bit = (recovered_ratio > 1.0) ? 1 : 0;
    std::cout << "  After mulY_inv recovery:  ratio=" << recovered_ratio
              << " decoded=" << recovered_bit << " (expected 0)\n";
    
    if (recovered_bit == 0) {
        std::cout << "\n  FRACTAL NAND WITH DECODE: WORKING!\n\n";
    }

    return 0;
}
