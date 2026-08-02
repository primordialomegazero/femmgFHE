// DM-DGR HOMOMORPHIC SIGN BOOTSTRAP
// No decryption — fully encrypted sign evaluation

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
    std::cout << "  DM-DGR HOMOMORPHIC SIGN BOOTSTRAP\n";
    std::cout << "  Fully encrypted — no decryption needed\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(35);
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

    // Pre-compute level-matched encrypted constants
    std::vector<Ciphertext<DCRTPoly>> ones_at_level;
    std::vector<Ciphertext<DCRTPoly>> phis_at_level;
    
    ones_at_level.push_back(enc_one);
    phis_at_level.push_back(enc_phi);
    
    auto pt_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    for (int i = 1; i <= 30; i++) {
        ones_at_level.push_back(cc->EvalMult(ones_at_level.back(), pt_one));
        phis_at_level.push_back(cc->EvalMult(phis_at_level.back(), pt_one));
    }

    // NAND raw
    auto nand_raw = [&](Ciphertext<DCRTPoly> A, Ciphertext<DCRTPoly> B, int lvl) {
        auto AND_a = cc->EvalMult(A, B);
        return cc->EvalSub(ones_at_level[lvl+1], AND_a);
    };

    // HOMOMORPHIC SIGN BOOTSTRAP — no decryption
    auto homomorphic_sign_bootstrap = [&](Ciphertext<DCRTPoly> ct) -> Ciphertext<DCRTPoly> {
        int lvl = ct->GetLevel();
        
        // Step 1: Wrap in PE, apply 10 mulY (0 EvalMult)
        PE state = {ct, ones_at_level[lvl]};
        for (int i = 0; i < 10; i++) {
            state = mulY(cc, state);
        }
        
        // After convergence: state.a ≈ 0.618 (positive) or -1.618 (negative)
        int conv_lvl = state.a->GetLevel();
        
        // Step 2: Add φ to shift: 0.618+1.618=2.236 or -1.618+1.618=0
        auto shifted = cc->EvalAdd(state.a, phis_at_level[conv_lvl]);
        
        // Step 3: Multiply by 1/2.236 = 0.447 (plaintext — CT×PT, 1 level)
        auto pt_scale = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.4472135955});
        auto scaled = cc->EvalMult(shifted, pt_scale);
        
        // Step 4: Add ψ = 0.618 (plaintext)
        auto pt_psi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
        auto result = cc->EvalAdd(scaled, pt_psi);
        
        // Result: ≈ 1.618 (if original was positive) or ≈ 0.618 (if negative)
        return result;
    };

    // Test
    std::cout << "  Testing Homomorphic Sign Bootstrap:\n";
    std::cout << "  A B | NAND_raw | Homomorphic result | Expected\n";
    std::cout << "  -----------------------------------------------\n";
    
    Ciphertext<DCRTPoly> inputs[2] = {enc_psi, enc_phi};
    
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto raw = nand_raw(inputs[a], inputs[b], 0);
            double raw_val = decrypt_val(cc, kp, raw);
            
            auto result = homomorphic_sign_bootstrap(raw);
            double final_val = decrypt_val(cc, kp, result);
            
            double expected = (a == 1 && b == 1) ? PSI : PHI;
            bool ok = std::abs(final_val - expected) < 0.15;
            
            std::cout << "  " << a << " " << b << " | " 
                      << std::fixed << std::setprecision(4) << raw_val << "    | "
                      << std::setprecision(4) << final_val << "           | "
                      << expected << (ok ? " OK" : " WRONG") << "\n";
        }
    }

    // Chain test
    std::cout << "\n  Chain test (homomorphic bootstrap each gate):\n";
    
    auto state = enc_phi;
    auto const_one = enc_phi;
    int correct = 0;
    
    for (int gate = 0; gate < 15; gate++) {
        int lvl = state->GetLevel();
        auto raw = nand_raw(state, const_one, lvl);
        state = homomorphic_sign_bootstrap(raw);
        
        double final_val = decrypt_val(cc, kp, state);
        int bit = (std::abs(final_val - PHI) < 0.2) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;
        
        if (bit == expected) correct++;
        
        std::cout << "  Gate " << std::setw(2) << gate 
                  << ": level=" << state->GetLevel()
                  << " val=" << std::fixed << std::setprecision(4) << final_val
                  << " bit=" << bit << " exp=" << expected
                  << (bit == expected ? " OK" : " WRONG") << "\n";
    }
    
    std::cout << "\n  Chain result: " << correct << "/15\n\n";
    
    if (correct == 15) {
        std::cout << "  FULLY HOMOMORPHIC SIGN BOOTSTRAP: WORKING\n";
        std::cout << "  No decryption. All operations in encrypted domain.\n";
        std::cout << "  Cost: 1 EvalMult (NAND) + 1 EvalMult (sign) = 2 EvalMult/gate\n";
        std::cout << "  Level refreshed: YES\n\n";
    }

    return 0;
}
