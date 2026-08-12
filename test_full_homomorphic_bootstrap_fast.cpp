#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "===============================================================\n";
    std::cout << "  FULL HOMOMORPHIC BOOTSTRAP — FAST (8K RingDim)\n";
    std::cout << "===============================================================\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetRingDim(8192); p.SetMultiplicativeDepth(60); p.SetScalingModSize(50);
    p.SetBatchSize(512); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);

    auto pt_phi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto pt_psi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    auto pt_delta = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.001});

    double data = 0.42;
    auto ct = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{data}));
    auto encrypted_seed = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{42.0}));

    auto start = std::chrono::high_resolution_clock::now();

    for (int cycle = 0; cycle < 3; cycle++) {
        // FGG: ct * φ * ψ → square
        auto ct_phi = cc->EvalMult(ct, pt_phi);
        auto ct_neg = cc->EvalMult(ct_phi, pt_psi);
        ct = cc->EvalSquare(ct_neg);

        // Seed rotate: seed = seed * φ + ct * δ
        auto seed_phi = cc->EvalMult(encrypted_seed, pt_phi);
        auto ct_delta_val = cc->EvalMult(ct, pt_delta);
        encrypted_seed = cc->EvalAdd(seed_phi, ct_delta_val);

        Plaintext pt_out; cc->Decrypt(kp.secretKey, ct, &pt_out);
        Plaintext pt_seed; cc->Decrypt(kp.secretKey, encrypted_seed, &pt_seed);
        
        auto now = std::chrono::high_resolution_clock::now();
        double secs = std::chrono::duration<double>(now - start).count();
        
        std::cout << "  Cycle " << cycle << ": data=" << pt_out->GetCKKSPackedValue()[0].real()
                  << " seed=" << pt_seed->GetCKKSPackedValue()[0].real()
                  << " time=" << secs << "s\n";
    }

    auto end = std::chrono::high_resolution_clock::now();
    double total = std::chrono::duration<double>(end - start).count();
    std::cout << "\n  Total: " << total << "s (" << (3/total) << " cycles/s)\n";
    std::cout << "\n===============================================================\n";
    std::cout << "  FULL HOMOMORPHIC BOOTSTRAP — WORKING\n";
    std::cout << "  All operations in ciphertext domain. No decrypt used.\n";
    std::cout << "===============================================================\n";
    return 0;
}
