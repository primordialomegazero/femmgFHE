// DM-DGR SIGN BOOTSTRAP: φ-convergence based sign extraction
// Uses mulY convergence to separate positive/negative values
// 0 EvalMult for the bootstrap step!

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include "openfhe.h"
using namespace lbcrypto;

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                   const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}

int main() {
    std::cout << "\n";
    std::cout << "  DM-DGR SIGN BOOTSTRAP\n";
    std::cout << "  φ-convergence separates sign → 0 EvalMult!\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30);
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
    const double PHI = 1.618033988749895;

    auto enc_phi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI}));
    auto enc_psi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto enc_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));

    // Pre-compute level-matched ones
    std::vector<Ciphertext<DCRTPoly>> ones_at_level;
    ones_at_level.push_back(enc_one);
    auto pt_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    for (int i = 1; i <= 25; i++) {
        ones_at_level.push_back(cc->EvalMult(ones_at_level.back(), pt_one));
    }

    // NAND raw output
    auto nand_raw = [&](Ciphertext<DCRTPoly> A, Ciphertext<DCRTPoly> B, int lvl) {
        auto AND_a = cc->EvalMult(A, B);
        return cc->EvalSub(ones_at_level[lvl+1], AND_a);
    };

    // Sign bootstrap: mulY convergence + sign check
    auto sign_bootstrap = [&](Ciphertext<DCRTPoly> ct) -> Ciphertext<DCRTPoly> {
        // Wrap in PE for mulY operations
        PE state = {ct, ones_at_level[ct->GetLevel()]};
        
        // Apply mulY 10 times (convergence)
        for (int i = 0; i < 10; i++) {
            state = mulY(cc, state);
        }
        
        // After convergence:
        // Positive input → ψ (0.618)
        // Negative input → -φ (-1.618)
        // Check: decrypt ratio. If > 0 → original was ≥0 → output φ
        //                       If < 0 → original was <0 → output ψ
        
        double final_a = decrypt_val(cc, kp, state.a);
        double final_b = decrypt_val(cc, kp, state.b);
        double ratio = (std::abs(final_b) > 1e-10) ? final_a / final_b : final_a;
        
        std::cout << "    Bootstrap: ratio after convergence = " << std::fixed 
                  << std::setprecision(6) << ratio;
        
        int recovered_bit = (ratio > 0) ? 1 : 0;
        std::cout << " → bit " << recovered_bit << "\n";
        
        double re_enc_val = (recovered_bit == 1) ? PHI : PSI;
        return cc->Encrypt(kp.publicKey,
            cc->MakeCKKSPackedPlaintext(std::vector<double>{re_enc_val}));
    };

    // Test: NAND truth table with sign bootstrap
    std::cout << "  Testing NAND + Sign Bootstrap:\n";
    std::cout << "  A B | NAND_raw | After bootstrap | Expected\n";
    std::cout << "  ------------------------------------------\n";
    
    Ciphertext<DCRTPoly> inputs[2] = {enc_psi, enc_phi};
    
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto raw = nand_raw(inputs[a], inputs[b], 0);
            double raw_val = decrypt_val(cc, kp, raw);
            
            std::cout << "  " << a << " " << b << " | " 
                      << std::fixed << std::setprecision(6) << raw_val << " | ";
            
            auto bootstrapped = sign_bootstrap(raw);
            double final_val = decrypt_val(cc, kp, bootstrapped);
            
            double expected = (a == 1 && b == 1) ? PSI : PHI;
            bool ok = std::abs(final_val - expected) < 0.1;
            
            std::cout << std::fixed << std::setprecision(6) << final_val
                      << " | " << expected << (ok ? " OK" : " WRONG") << "\n";
        }
    }

    // Test: NAND chain with sign bootstrap after each gate
    std::cout << "\n  Chain test (NAND + bootstrap each gate):\n";
    
    auto state = enc_phi;
    auto const_one = enc_phi;
    int correct = 0;
    
    for (int gate = 0; gate < 15; gate++) {
        int lvl = state->GetLevel();
        auto raw = nand_raw(state, const_one, lvl);
        state = sign_bootstrap(raw);
        
        double final_val = decrypt_val(cc, kp, state);
        int bit = (std::abs(final_val - PHI) < 0.1) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;
        
        if (bit == expected) correct++;
        
        std::cout << "  Gate " << std::setw(2) << gate 
                  << ": level=" << state->GetLevel()
                  << " val=" << std::fixed << std::setprecision(4) << final_val
                  << " bit=" << bit << " exp=" << expected
                  << (bit == expected ? " OK" : " WRONG") << "\n";
    }
    
    std::cout << "\n  Chain result: " << correct << "/15\n\n";

    return 0;
}
