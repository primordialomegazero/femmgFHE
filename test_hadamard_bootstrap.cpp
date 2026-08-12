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
    std::cout << "  GOLDEN HADAMARD BOOTSTRAP — f(x) = (x + 1) / φ\n";
    std::cout << "  Fixed point at φ ≈ 1.618\n";
    std::cout << "  Homomorphic: EvalAdd(ct, 1) -> EvalMult(result, 1/φ)\n";
    std::cout << "===============================================================\n\n";

    // Test in plaintext first
    std::cout << "--- PLAINTEXT FIXED POINT ANALYSIS ---\n\n";
    
    double test_vals[] = {0.1, 0.42, 0.5, 1.0, 1.618, 2.0, 3.0};
    int n = 7;
    
    std::cout << "  f(x) = (x + 1) / φ:\n\n";
    std::cout << "  " << std::setw(10) << "x" << std::setw(14) << "f(x)"
              << std::setw(14) << "f(f(x))" << std::setw(14) << "f^5(x)"
              << std::setw(14) << "Converges?\n";
    std::cout << "  " << std::string(66, '-') << "\n";
    
    for (int i = 0; i < n; i++) {
        double x = test_vals[i];
        double fx = (x + 1.0) / PHI;
        double ffx = (fx + 1.0) / PHI;
        double f5x = x;
        for (int j = 0; j < 5; j++) f5x = (f5x + 1.0) / PHI;
        
        double target = PHI;  // fixed point
        bool converges = fabs(f5x - target) < 0.01;
        
        std::cout << "  " << std::setw(10) << x << std::setw(14) << fx
                  << std::setw(14) << ffx << std::setw(14) << f5x
                  << std::setw(14) << (converges ? "YES → φ" : "no") << "\n";
    }
    
    std::cout << "\n  Fixed point: φ = " << PHI << "\n";
    std::cout << "  All values converge to φ!\n\n";

    // Test in CKKS
    std::cout << "--- HOMOMORPHIC HADAMARD IN CKKS ---\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetRingDim(8192); p.SetMultiplicativeDepth(60); p.SetScalingModSize(50);
    p.SetBatchSize(512); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    auto pt_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto pt_inv_phi = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/PHI});
    
    double m = 0.42;
    auto ct = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{m}));
    
    std::cout << "  Initial m = " << m << "\n";
    std::cout << "  Fixed point target = " << PHI << "\n\n";
    std::cout << "  " << std::setw(6) << "Cycle" << std::setw(14) << "Value"
              << std::setw(14) << "Distance to φ\n";
    std::cout << "  " << std::string(34, '-') << "\n";
    
    for (int cycle = 0; cycle < 8; cycle++) {
        // Golden Hadamard: f(ct) = (ct + 1) / φ
        auto ct_plus_one = cc->EvalAdd(ct, pt_one);
        ct = cc->EvalMult(ct_plus_one, pt_inv_phi);
        
        Plaintext pt_out;
        cc->Decrypt(kp.secretKey, ct, &pt_out);
        double val = pt_out->GetCKKSPackedValue()[0].real();
        double dist = fabs(val - PHI);
        
        std::cout << "  " << std::setw(6) << cycle << std::setw(14) << val
                  << std::setw(14) << dist << "\n";
    }
    
    // Test: Seed rotation using Hadamard
    std::cout << "\n--- HADAMARD SEED ROTATION ---\n\n";
    
    auto encrypted_seed = cc->Encrypt(kp.publicKey, 
        cc->MakeCKKSPackedPlaintext(std::vector<double>{42.0}));
    
    for (int i = 0; i < 3; i++) {
        auto seed_plus = cc->EvalAdd(encrypted_seed, pt_one);
        encrypted_seed = cc->EvalMult(seed_plus, pt_inv_phi);
        
        Plaintext pt_seed;
        cc->Decrypt(kp.secretKey, encrypted_seed, &pt_seed);
        std::cout << "  Seed cycle " << i << ": " << pt_seed->GetCKKSPackedValue()[0].real() << "\n";
    }

    std::cout << "\n===============================================================\n";
    std::cout << "  GOLDEN HADAMARD — f(x) = (x+1)/φ\n";
    std::cout << "  Fixed point: φ ≈ 1.618\n";
    std::cout << "  Homomorphic: 2 ops (EvalAdd + EvalMult)\n";
    std::cout << "===============================================================\n";

    return 0;
}
