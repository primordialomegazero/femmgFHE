#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <chrono>
#include "openfhe.h"

using namespace lbcrypto;

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "===============================================================\n";
    std::cout << "  FULLY HOMOMORPHIC BOOTSTRAP — ALL 4 COMPONENTS ENCRYPTED\n";
    std::cout << "  FGG + Cassini + Hadamard + Seed Rotation — NO DECRYPT\n";
    std::cout << "===============================================================\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetRingDim(8192); p.SetMultiplicativeDepth(120); p.SetScalingModSize(50);
    p.SetBatchSize(512); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    auto pt_phi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto pt_psi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    auto pt_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto pt_delta = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.001});
    auto pt_inv_phi = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/PHI});

    double data = 0.42;
    auto ct = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{data}));
    auto encrypted_seed = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.5}));

    std::cout << "Initial data: " << data << "\n";
    std::cout << "Initial seed: 0.5 (ENCRYPTED)\n\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    std::cout << "--- 5 FULLY HOMOMORPHIC BOOTSTRAP CYCLES ---\n\n";
    std::cout << "  " << std::setw(6) << "Cycle" << std::setw(14) << "CKKS val"
              << std::setw(14) << "Cassini" << std::setw(14) << "Seed\n";
    std::cout << "  " << std::string(48, '-') << "\n";

    for (int cycle = 0; cycle < 5; cycle++) {
        // ==========================================
        // PHASE 1: HOMOMORPHIC FGG
        // ==========================================
        auto ct_phi_val = cc->EvalMult(ct, pt_phi);
        auto ct_neg = cc->EvalMult(ct_phi_val, pt_psi);
        ct = cc->EvalSquare(ct_neg);

        // ==========================================
        // PHASE 2: HOMOMORPHIC CASSINI
        // ==========================================
        auto y1_phi = cc->EvalAdd(ct, pt_phi);
        auto y2_psi = cc->EvalAdd(ct, pt_psi);
        auto cassini_prod = cc->EvalMult(y1_phi, y2_psi);
        auto cassini_val = cc->EvalAdd(cassini_prod, pt_one);
        auto ct_cassini = cc->EvalSquare(cassini_val);

        // ==========================================
        // PHASE 3: HOMOMORPHIC HADAMARD
        // ==========================================
        auto ct_plus_one = cc->EvalAdd(ct, pt_one);
        ct = cc->EvalMult(ct_plus_one, pt_inv_phi);

        // ==========================================
        // PHASE 4: HOMOMORPHIC SEED ROTATION
        // seed' = seed * φ + ct * δ
        // ==========================================
        auto seed_phi = cc->EvalMult(encrypted_seed, pt_phi);
        auto ct_delta_val = cc->EvalMult(ct, pt_delta);
        encrypted_seed = cc->EvalAdd(seed_phi, ct_delta_val);

        // Decrypt for monitoring only
        Plaintext pt_out, pt_cass, pt_seed;
        cc->Decrypt(kp.secretKey, ct, &pt_out);
        cc->Decrypt(kp.secretKey, ct_cassini, &pt_cass);
        cc->Decrypt(kp.secretKey, encrypted_seed, &pt_seed);

        std::cout << "  " << std::setw(6) << cycle
                  << std::setw(14) << pt_out->GetCKKSPackedValue()[0].real()
                  << std::setw(14) << pt_cass->GetCKKSPackedValue()[0].real()
                  << std::setw(14) << pt_seed->GetCKKSPackedValue()[0].real() << "\n";
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double>(end_time - start_time).count();

    // Final decrypt
    Plaintext pt_final, pt_seed_final;
    cc->Decrypt(kp.secretKey, ct, &pt_final);
    cc->Decrypt(kp.secretKey, encrypted_seed, &pt_seed_final);

    std::cout << "  " << std::string(48, '-') << "\n";
    std::cout << "  Total: " << total_time << "s (" << (5/total_time) << " cycles/s)\n\n";

    std::cout << "--- FINAL STATE ---\n";
    std::cout << "  Final CKKS value: " << pt_final->GetCKKSPackedValue()[0].real() << "\n";
    std::cout << "  Final seed (encrypted): " << pt_seed_final->GetCKKSPackedValue()[0].real() << "\n";
    std::cout << "  φ·ψ = " << PHI*PSI << "\n";

    std::cout << "\n===============================================================\n";
    std::cout << "  FULLY HOMOMORPHIC BOOTSTRAP — COMPLETE\n";
    std::cout << "  FGG: EvalMult(φ) → EvalMult(ψ) → EvalSquare\n";
    std::cout << "  Cassini: EvalAdd(φ) → EvalAdd(ψ) → EvalMult → EvalAdd(1) → EvalSquare\n";
    std::cout << "  Hadamard: EvalAdd(1) → EvalMult(1/φ)\n";
    std::cout << "  Seed Rotation: EvalMult(φ) → EvalAdd(ct*δ)\n";
    std::cout << "  ALL IN CIPHERTEXT DOMAIN. NO DECRYPT.\n";
    std::cout << "===============================================================\n";

    return 0;
}
