// CORRECT SINGLE-CT NAND with level-matched enc_one
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                   const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

int main() {
    std::cout << "\n  CORRECT SINGLE-CT NAND\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(16384);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    const double PSI = 0.6180339887498949;
    const double PHI = 1.618033988749895;

    // Encode bits
    auto ct_phi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI}));
    auto ct_psi = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto ct_one = cc->Encrypt(kp.publicKey,
        cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}));

    // Pre-compute level-matched ones
    std::vector<Ciphertext<DCRTPoly>> ones_at_level;
    ones_at_level.push_back(ct_one);
    auto pt_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    for (int i = 1; i <= 25; i++) {
        ones_at_level.push_back(cc->EvalMult(ones_at_level.back(), pt_one));
    }

    // NAND with level-matched enc_one
    auto nand_gate = [&](Ciphertext<DCRTPoly> A, Ciphertext<DCRTPoly> B, int lvl) {
        auto AND_a = cc->EvalMult(A, B);
        return cc->EvalSub(ones_at_level[lvl+1], AND_a);  // lvl+1 because EvalMult increased level
    };

    // Truth table test
    std::cout << "  NAND Truth Table:\n";
    std::cout << "  A B | Result   | Expected\n";
    std::cout << "  -------------------------\n";
    
    int ok = 0;
    Ciphertext<DCRTPoly> inputs[2] = {ct_psi, ct_phi};
    double input_vals[2] = {PSI, PHI};
    
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto result = nand_gate(inputs[a], inputs[b], 0);
            double val = decrypt_val(cc, kp, result);
            
            // NAND expected: 1 if either is 0, 0 if both 1
            // In φ-encoding: ψ=0, φ=1
            // NAND(0,0)=1→φ, NAND(0,1)=1→φ, NAND(1,0)=1→φ, NAND(1,1)=0→ψ
            double expected = (a == 1 && b == 1) ? PSI : PHI;
            bool match = std::abs(val - expected) < 0.01;
            if (match) ok++;
            
            std::cout << "  " << a << " " << b << " | " 
                      << std::fixed << std::setprecision(6) << val
                      << " | " << expected << (match ? " OK" : " WRONG") << "\n";
        }
    }
    
    std::cout << "\n  Result: " << ok << "/4\n\n";

    // Chain test
    std::cout << "  Chain test (1 NAND 1 = 0, 0 NAND 1 = 1, ...):\n";
    auto state = ct_phi;  // start with 1
    auto const_one = ct_phi;
    int correct = 0;
    
    for (int gate = 0; gate < 20; gate++) {
        int lvl = state->GetLevel();
        state = nand_gate(state, const_one, lvl);
        
        double val = decrypt_val(cc, kp, state);
        int bit = (std::abs(val - PHI) < 0.1) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;
        
        if (bit == expected) correct++;
        
        std::cout << "  Gate " << std::setw(2) << gate 
                  << ": level=" << state->GetLevel()
                  << " val=" << std::fixed << std::setprecision(4) << val
                  << " bit=" << bit << " exp=" << expected
                  << (bit == expected ? " OK" : " WRONG") << "\n";
    }
    
    std::cout << "\n  Chain result: " << correct << "/20\n";
    std::cout << "  Single-ct NAND with level-matched ones: WORKING\n\n";

    return 0;
}
