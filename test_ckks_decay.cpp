#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include "openfhe.h"

using namespace lbcrypto;

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

CryptoContext<DCRTPoly> init_ckks() {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetRingDim(16384); p.SetMultiplicativeDepth(300); p.SetScalingModSize(50);
    p.SetBatchSize(1024); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    return cc;
}

// FGG collapse: ct * φ * ψ * ct → -ct² (sign erased)
Ciphertext<DCRTPoly> fgg_collapse(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& ct) {
    auto pt_phi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto pt_psi = cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    
    auto ct_phi = cc->EvalMult(ct, pt_phi);
    auto ct_neg = cc->EvalMult(ct_phi, pt_psi);
    return cc->EvalSquare(ct_neg);
}

int main() {
    std::cout << std::fixed << std::setprecision(8);
    std::cout << "===============================================================\n";
    std::cout << "  CKKS STRUCTURAL DECAY — CAN WE EXTRACT m WITHOUT sk?\n";
    std::cout << "  Testing: after repeated FGG, does plaintext dominate?\n";
    std::cout << "===============================================================\n\n";

    auto cc = init_ckks();
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    double test_vals[] = {0.1, 0.25, 0.42, 0.5, 0.75, 0.9};
    int n = 6;

    std::cout << "--- SIGNAL TRACKING OVER FGG COLLAPSE ---\n\n";
    
    for (int i = 0; i < n; i++) {
        double m = test_vals[i];
        auto ct = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{m}));
        
        std::cout << "  m=" << m << ":\n";
        std::cout << "    " << std::setw(6) << "FGG#" << std::setw(16) << "Decrypt(m)" 
                  << std::setw(16) << "Expected" << std::setw(14) << "Error\n";
        std::cout << "    " << std::string(52, '-') << "\n";
        
        double expected = m;
        for (int f = 0; f < 8; f++) {
            ct = fgg_collapse(cc, ct);
            expected = pow(expected, 2);
            
            Plaintext pt;
            cc->Decrypt(kp.secretKey, ct, &pt);
            double val = pt->GetCKKSPackedValue()[0].real();
            double error = fabs(val - expected);
            
            std::cout << "    " << std::setw(6) << f << std::setw(16) << val
                      << std::setw(16) << expected << std::setw(14) << error << "\n";
            
            if (val < 1e-10) break;
        }
        std::cout << "\n";
    }

    // Test: Can we use the convergence pattern?
    std::cout << "--- CONVERGENCE PATTERN ---\n\n";
    std::cout << "  Values < 1: converge to 0\n";
    std::cout << "  Values = 1: stay at 1\n";
    std::cout << "  Values > 1: diverge\n\n";
    std::cout << "  The rate of convergence is determined by the initial value.\n";
    std::cout << "  An attacker could potentially estimate |m| from convergence rate.\n";
    std::cout << "  But the sign is erased — only |m| is recoverable.\n\n";

    // Test: Ratio preservation
    std::cout << "--- RATIO BETWEEN TWO VALUES ---\n\n";
    double m1 = 0.3, m2 = 0.7;
    auto ct1 = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{m1}));
    auto ct2 = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{m2}));
    
    std::cout << "  Initial ratio m2/m1 = " << (m2/m1) << "\n";
    
    for (int f = 0; f < 5; f++) {
        ct1 = fgg_collapse(cc, ct1);
        ct2 = fgg_collapse(cc, ct2);
        
        Plaintext pt1, pt2;
        cc->Decrypt(kp.secretKey, ct1, &pt1);
        cc->Decrypt(kp.secretKey, ct2, &pt2);
        double v1 = pt1->GetCKKSPackedValue()[0].real();
        double v2 = pt2->GetCKKSPackedValue()[0].real();
        
        if (v1 > 0) {
            std::cout << "  FGG#" << f << " ratio = " << (v2/v1) 
                      << " (expected " << pow(m2/m1, pow(2, f+1)) << ")\n";
        }
    }

    std::cout << "\n===============================================================\n";
    std::cout << "  CKKS DECAY ANALYSIS COMPLETE\n";
    std::cout << "===============================================================\n";

    return 0;
}
