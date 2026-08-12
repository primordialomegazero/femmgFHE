#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

CryptoContext<DCRTPoly> init_ckks() {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetRingDim(8192); p.SetMultiplicativeDepth(30); p.SetScalingModSize(50);
    p.SetBatchSize(512); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    return cc;
}

// Homomorphic FGG: ct * (-1), then |ct| via x^2 approximation
Ciphertext<DCRTPoly> homomorphic_fgg(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct) {
    auto pt_neg = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    auto ct_neg = cc->EvalMult(ct, pt_neg);
    auto ct_sq = cc->EvalSquare(ct_neg);
    auto pt_half = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.5});
    return cc->EvalAdd(pt_half, cc->EvalMult(ct_sq, pt_half));
}

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "===============================================================\n";
    std::cout << "  FGG DECAY ANALYSIS — CKKS Structural Collapse\n";
    std::cout << "  Testing: does homomorphic FGG create structural decay?\n";
    std::cout << "===============================================================\n\n";

    auto cc = init_ckks();
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    // Test values
    double test_vals[] = {-1.0, -0.5, -0.1, 0.0, 0.1, 0.42, 0.5, 1.0};
    int n_vals = 8;

    std::cout << "--- FGG DECAY OVER REPEATED APPLICATION ---\n\n";
    std::cout << "  " << std::setw(8) << "Initial" << std::setw(14) << "FGG^1" 
              << std::setw(14) << "FGG^3" << std::setw(14) << "FGG^5" 
              << std::setw(14) << "Converges\n";
    std::cout << "  " << std::string(64, '-') << "\n";

    for (int i = 0; i < n_vals; i++) {
        double init = test_vals[i];
        auto pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{init});
        auto ct = cc->Encrypt(kp.publicKey, pt);

        // Apply FGG 5 times
        std::vector<double> results;
        for (int f = 0; f < 5; f++) {
            ct = homomorphic_fgg(cc, ct);
            Plaintext pt_out;
            cc->Decrypt(kp.secretKey, ct, &pt_out);
            results.push_back(pt_out->GetCKKSPackedValue()[0].real());
        }

        std::cout << "  " << std::setw(8) << init 
                  << std::setw(14) << results[0]
                  << std::setw(14) << results[2]
                  << std::setw(14) << results[4]
                  << std::setw(14) << (fabs(results[4] - fabs(init)) < 0.1 ? "YES" : "no")
                  << "\n";
    }

    // Test: does the value converge to |init|?
    std::cout << "\n--- CONVERGENCE TO ABSOLUTE VALUE ---\n\n";
    std::cout << "  " << std::setw(8) << "Init" << std::setw(12) << "Expected" 
              << std::setw(12) << "FGG^3" << std::setw(12) << "Error\n";
    std::cout << "  " << std::string(44, '-') << "\n";

    for (int i = 0; i < n_vals; i++) {
        double init = test_vals[i];
        double expected = fabs(init);
        auto pt = cc->MakeCKKSPackedPlaintext(std::vector<double>{init});
        auto ct = cc->Encrypt(kp.publicKey, pt);
        
        for (int f = 0; f < 3; f++) ct = homomorphic_fgg(cc, ct);
        
        Plaintext pt_out;
        cc->Decrypt(kp.secretKey, ct, &pt_out);
        double result = pt_out->GetCKKSPackedValue()[0].real();
        double error = fabs(result - expected);
        
        std::cout << "  " << std::setw(8) << init 
                  << std::setw(12) << expected
                  << std::setw(12) << result
                  << std::setw(12) << error << "\n";
    }

    // Test: φ·ψ = -1 identity in ciphertext domain
    std::cout << "\n--- φ·ψ = -1 IN CIPHERTEXT DOMAIN ---\n\n";
    auto pt_phi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto pt_psi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    
    for (int i = 0; i < n_vals; i++) {
        double init = test_vals[i];
        auto ct = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{init}));
        auto ct_phi = cc->EvalMult(ct, pt_phi);
        auto ct_psi = cc->EvalMult(ct_phi, pt_psi);
        
        Plaintext pt_out;
        cc->Decrypt(kp.secretKey, ct_psi, &pt_out);
        double result = pt_out->GetCKKSPackedValue()[0].real();
        double expected = init * PHI * PSI; // = init * (-1)
        
        std::cout << "  " << init << " * φ * ψ = " << result << " (exp " << expected << ")\n";
    }

    std::cout << "\n===============================================================\n";
    std::cout << "  FGG DECAY ANALYSIS COMPLETE\n";
    std::cout << "===============================================================\n";
    return 0;
}
