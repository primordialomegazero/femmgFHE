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

// φ-PERTURBATION: state' = state * (1 + ε*φ)
// Small perturbation, minimal value change, noise redistributed
Ciphertext<DCRTPoly> phi_perturb(CryptoContext<DCRTPoly>& cc, const Ciphertext<DCRTPoly>& state, double epsilon = 0.001) {
    double perturb_factor = 1.0 + epsilon * PHI;
    auto pt_perturb = cc->MakeCKKSPackedPlaintext(std::vector<double>{perturb_factor});
    return cc->EvalMult(state, pt_perturb);
}

// Measure noise by checking decryption error over many cycles
double measure_error(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, 
                     const Ciphertext<DCRTPoly>& ct, double expected) {
    Plaintext pt;
    cc->Decrypt(kp.secretKey, ct, &pt);
    double val = pt->GetCKKSPackedValue()[0].real();
    return fabs(val - expected);
}

int main() {
    std::cout << std::fixed << std::setprecision(8);
    std::cout << "===============================================================\n";
    std::cout << "  φ-PERTURBATION — NOISE REDISTRIBUTION\n";
    std::cout << "  state' = state * (1 + ε*φ)\n";
    std::cout << "  Testing: does small φ-perturbation redistribute noise?\n";
    std::cout << "===============================================================\n\n";

    auto cc = init_ckks();
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    double data = 0.42;
    auto ct = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{data}));

    // Track value and error over many perturbations
    std::cout << "--- 100 φ-PERTURBATIONS (ε=0.001) ---\n\n";
    std::cout << "  " << std::setw(6) << "Cycle" << std::setw(16) << "Value" 
              << std::setw(16) << "Expected" << std::setw(14) << "Error\n";
    std::cout << "  " << std::string(52, '-') << "\n";

    double expected = data;
    double max_error = 0;
    double epsilon = 0.001;
    
    for (int i = 0; i < 100; i++) {
        ct = phi_perturb(cc, ct, epsilon);
        expected *= (1.0 + epsilon * PHI);
        
        if (i % 10 == 0 || i == 99) {
            double error = measure_error(cc, kp, ct, expected);
            if (error > max_error) max_error = error;
            
            Plaintext pt; cc->Decrypt(kp.secretKey, ct, &pt);
            double val = pt->GetCKKSPackedValue()[0].real();
            
            std::cout << "  " << std::setw(6) << i << std::setw(16) << val
                      << std::setw(16) << expected << std::setw(14) << error << "\n";
        }
    }

    std::cout << "  " << std::string(52, '-') << "\n";
    std::cout << "  Max error after 100 perturbations: " << max_error << "\n";
    std::cout << "  Value drift from " << data << " to " << expected << "\n\n";

    // Test: Different epsilon values
    std::cout << "--- EPSILON COMPARISON ---\n\n";
    std::cout << "  " << std::setw(10) << "Epsilon" << std::setw(16) << "Max Error" 
              << std::setw(16) << "Value After" << std::setw(16) << "Drift\n";
    std::cout << "  " << std::string(58, '-') << "\n";

    double epsilons[] = {0.0001, 0.0005, 0.001, 0.005, 0.01};
    for (double eps : epsilons) {
        auto ct_test = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{data}));
        double exp_val = data;
        double max_err = 0;
        
        for (int i = 0; i < 50; i++) {
            ct_test = phi_perturb(cc, ct_test, eps);
            exp_val *= (1.0 + eps * PHI);
            double err = measure_error(cc, kp, ct_test, exp_val);
            if (err > max_err) max_err = err;
        }
        
        double drift = fabs(exp_val - data);
        std::cout << "  " << std::setw(10) << eps << std::setw(16) << max_err
                  << std::setw(16) << exp_val << std::setw(16) << drift << "\n";
    }

    // Test: Does perturbation followed by inverse preserve value?
    std::cout << "\n--- ROUND-TRIP: PERTURB + INVERSE ---\n\n";
    auto ct_rt = cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{data}));
    
    // Perturb
    ct_rt = phi_perturb(cc, ct_rt, 0.001);
    // Inverse perturb
    double inv_factor = 1.0 / (1.0 + 0.001 * PHI);
    auto pt_inv = cc->MakeCKKSPackedPlaintext(std::vector<double>{inv_factor});
    ct_rt = cc->EvalMult(ct_rt, pt_inv);
    
    double rt_error = measure_error(cc, kp, ct_rt, data);
    Plaintext pt_rt; cc->Decrypt(kp.secretKey, ct_rt, &pt_rt);
    std::cout << "  Original: " << data << "\n";
    std::cout << "  Round-trip: " << pt_rt->GetCKKSPackedValue()[0].real() << "\n";
    std::cout << "  Error: " << rt_error << "\n";

    std::cout << "\n===============================================================\n";
    std::cout << "  φ-PERTURBATION ANALYSIS COMPLETE\n";
    std::cout << "===============================================================\n";

    return 0;
}
