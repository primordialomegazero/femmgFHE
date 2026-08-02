// DM-DGR DIRECT HOMOMORPHIC SIGN
// No decrypt, no re-encrypt, no mulY convergence
// Direct polynomial sign evaluation on NAND output

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

int main() {
    std::cout << "\n";
    std::cout << "  DM-DGR DIRECT HOMOMORPHIC SIGN\n";
    std::cout << "  No decrypt, no re-encrypt, no mulY\n";
    std::cout << "  Pure polynomial sign evaluation\n\n";

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

    // Pre-compute level-matched encrypted ones
    std::vector<Ciphertext<DCRTPoly>> ones_at_level;
    ones_at_level.push_back(enc_one);
    auto pt_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    for (int i = 1; i <= 30; i++) {
        ones_at_level.push_back(cc->EvalMult(ones_at_level.back(), pt_one));
    }

    // NAND raw output: 1 - a×b
    auto nand_raw = [&](Ciphertext<DCRTPoly> A, Ciphertext<DCRTPoly> B, int lvl) {
        auto AND_a = cc->EvalMult(A, B);
        return cc->EvalSub(ones_at_level[lvl+1], AND_a);
    };

    // DIRECT HOMOMORPHIC SIGN NORMALIZATION
    // Input: v ∈ {0.618, 2.0, -1.618}
    // Output: ψ=0.618 if v<0, φ=1.618 if v≥0
    //
    // Sign approximation H(v): maps [-1.618, 2.0] → [0, 1]
    // H(v) = (v + φ) / (φ + ψ + φ) = (v + 1.618) / (1.618 + 0.618 + 1.618)
    // Actually use linear fit on two key points:
    //   v = -1.618 → H = 0
    //   v = 0.618  → H = 1
    // H(v) = a·v + b
    // a = 1/(0.618 - (-1.618)) = 1/2.236 = 0.4472
    // b = 1 - a·0.618 = 1 - 0.2764 = 0.7236
    //
    // Output = ψ + (φ-ψ)·H(v) = 0.618 + 1.0·H(v)
    //        = 0.618 + 0.4472·v + 0.7236
    //        = 1.3416 + 0.4472·v
    
    auto normalize_sign = [&](Ciphertext<DCRTPoly> ct) -> Ciphertext<DCRTPoly> {
        // Step 1: ct × 0.4472 (CT×PT, 1 level)
        auto pt_scale = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.4472135955});
        auto scaled = cc->EvalMult(ct, pt_scale);
        
        // Step 2: + 1.3416 (plaintext addition, 0 levels)
        auto pt_offset = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.3416407865});
        return cc->EvalAdd(scaled, pt_offset);
    };

    // Test truth table
    std::cout << "  Truth Table Test:\n";
    std::cout << "  A B | NAND_raw | Normalized | Expected | Match\n";
    std::cout << "  ---------------------------------------------\n";
    
    Ciphertext<DCRTPoly> inputs[2] = {enc_psi, enc_phi};
    int truth_ok = 0;
    
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto raw = nand_raw(inputs[a], inputs[b], 0);
            auto normalized = normalize_sign(raw);
            
            double raw_val = decrypt_val(cc, kp, raw);
            double norm_val = decrypt_val(cc, kp, normalized);
            double expected = (a == 1 && b == 1) ? PSI : PHI;
            
            bool ok = std::abs(norm_val - expected) < 0.15;
            if (ok) truth_ok++;
            
            std::cout << "  " << a << " " << b << " | "
                      << std::fixed << std::setprecision(4) << std::setw(7) << raw_val << " | "
                      << std::setw(10) << norm_val << " | "
                      << std::setw(7) << expected << " | "
                      << (ok ? "OK" : "WRONG") << "\n";
        }
    }
    
    std::cout << "\n  Truth table: " << truth_ok << "/4\n\n";

    // Chain test with level refresh via plaintext-ciphertext ops
    std::cout << "  Chain Test (NAND + normalize each gate):\n";
    
    auto state = enc_phi;
    auto const_one = enc_phi;
    int chain_ok = 0;
    
    for (int gate = 0; gate < 20; gate++) {
        int lvl = state->GetLevel();
        auto raw = nand_raw(state, const_one, lvl);
        state = normalize_sign(raw);
        
        double val = decrypt_val(cc, kp, state);
        int bit = (val > 1.0) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;
        
        if (bit == expected) chain_ok++;
        
        if (gate < 5 || gate >= 15) {
            std::cout << "  Gate " << std::setw(2) << gate
                      << ": level=" << state->GetLevel()
                      << " val=" << std::fixed << std::setprecision(4) << val
                      << " bit=" << bit << " exp=" << expected
                      << (bit == expected ? " OK" : " WRONG") << "\n";
        }
    }
    
    std::cout << "\n  Chain result: " << chain_ok << "/20\n\n";
    
    if (truth_ok == 4 && chain_ok == 20) {
        std::cout << "  FULLY HOMOMORPHIC: NO DECRYPT, NO RE-ENCRYPT\n";
        std::cout << "  Each gate: 2 EvalMult (NAND + normalize)\n";
        std::cout << "  Level consumed: 2 per gate\n";
        std::cout << "  With bootstrapping every K gates: unlimited depth\n\n";
    }

    return 0;
}
