#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <cstdlib>
#include <random>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;
const double PSI = (1.0 - std::sqrt(5.0)) / 2.0;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
struct SecureContext { CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> kp; };

SecureContext create_context(uint32_t ring_dim, uint32_t depth, uint32_t batch_size) {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(depth);
    p.SetScalingModSize(50);
    p.SetBatchSize(batch_size);
    p.SetRingDim(ring_dim);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    return {cc, kp};
}

DualGate encrypt_input(SecureContext& sc, double val) {
    return {sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

double decrypt_value(SecureContext& sc, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; sc.cc->Decrypt(sc.kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double reveal(DualGate& dg, SecureContext& sc, double root) {
    return decrypt_value(sc, dg.a) + decrypt_value(sc, dg.b) * root;
}

double purify(double val, double threshold) { 
    return (val > threshold) ? 1.0 : 0.0; 
}

DualGate gate_not(SecureContext& sc, DualGate& X) {
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto neg_one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {sc.cc->EvalSub(one, X.a), sc.cc->EvalMult(neg_one, X.b)};
}

DualGate gate_nand(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a_out = sc.cc->EvalMult(X.a, Y.a);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    DualGate and_result = {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
    return gate_not(sc, and_result);
}

DualGate gate_and(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate nand_result = gate_nand(sc, X, Y);
    return gate_not(sc, nand_result);
}

DualGate gate_or(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate not_x = gate_not(sc, X);
    DualGate not_y = gate_not(sc, Y);
    return gate_nand(sc, not_x, not_y);
}

// LAYER 1: iO CORE — Two circuits, one structure
DualGate layer1_iO(SecureContext& sc, double x, double y, double z) {
    DualGate X = encrypt_input(sc, x);
    DualGate Y = encrypt_input(sc, y);
    DualGate Z = encrypt_input(sc, z);
    
    DualGate and_xy = gate_and(sc, X, Y);
    DualGate or_xz  = gate_or(sc, X, Z);
    DualGate or_yz  = gate_or(sc, Y, Z);
    
    DualGate pathA = gate_or(sc, and_xy, Z);
    DualGate pathB = gate_and(sc, or_xz, or_yz);
    
    auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto inv_denom = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0 / (PHI - PSI)});
    
    DualGate result;
    auto diff_a = sc.cc->EvalSub(pathA.a, pathB.a);
    auto diff_b = sc.cc->EvalSub(pathA.b, pathB.b);
    result.b = sc.cc->EvalAdd(sc.cc->EvalMult(diff_a, inv_denom), sc.cc->EvalMult(diff_b, inv_denom));
    result.a = sc.cc->EvalSub(pathA.a, sc.cc->EvalMult(result.b, phi_pt));
    
    return result;
}

// LAYER 2: CHAOS ENGINE — Avalanche, entropy, divergence
DualGate chaos_engine(SecureContext& sc, DualGate& input) {
    auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto psi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    
    // Chaotic map: x → φ·x·(1-x)  (logistic map with golden ratio)
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto one_minus_a = sc.cc->EvalSub(one, input.a);
    auto logistic_a = sc.cc->EvalMult(input.a, one_minus_a);
    auto chaos_a = sc.cc->EvalMult(logistic_a, phi_pt);
    
    auto one_minus_b = sc.cc->EvalSub(one, input.b);
    auto logistic_b = sc.cc->EvalMult(input.b, one_minus_b);
    auto chaos_b = sc.cc->EvalMult(logistic_b, psi_pt);
    
    return {chaos_a, chaos_b};
}

// LAYER 2: STABILITY ENGINE — Convergence, fixed point, dampening
DualGate stability_engine(SecureContext& sc, DualGate& input) {
    auto inv_phi = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/PHI});
    auto inv_psi = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/PSI});
    
    // Stable map: x → x/φ  (contraction mapping)
    auto stable_a = sc.cc->EvalMult(input.a, inv_phi);
    auto stable_b = sc.cc->EvalMult(input.b, inv_psi);
    
    return {stable_a, stable_b};
}

// COMBINED FRACTAL LAYER 2: Chaos + Stability, iO-hidden
DualGate layer2_fractal(SecureContext& sc, DualGate& input) {
    // ALWAYS compute BOTH engines
    DualGate chaos_result = chaos_engine(sc, input);
    DualGate stability_result = stability_engine(sc, input);
    
    // Encode into (a,b) — same structure as Layer 1
    auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto inv_denom = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0 / (PHI - PSI)});
    
    DualGate result;
    auto diff_a = sc.cc->EvalSub(chaos_result.a, stability_result.a);
    auto diff_b = sc.cc->EvalSub(chaos_result.b, stability_result.b);
    result.b = sc.cc->EvalAdd(sc.cc->EvalMult(diff_a, inv_denom), sc.cc->EvalMult(diff_b, inv_denom));
    result.a = sc.cc->EvalSub(chaos_result.a, sc.cc->EvalMult(result.b, phi_pt));
    
    return result;
}

// FRACTAL iO: Layer 1 → Layer 2 → Output
DualGate fractal_iO(SecureContext& sc, double x, double y, double z) {
    DualGate layer1_output = layer1_iO(sc, x, y, z);
    return layer2_fractal(sc, layer1_output);
}

int main(int argc, char* argv[]) {
    uint32_t ring_dim = (argc > 1) ? std::atoi(argv[1]) : 2048;
    uint32_t depth = (argc > 2) ? std::atoi(argv[2]) : 50;
    int trials = (argc > 3) ? std::atoi(argv[3]) : 1000;
    double threshold = (argc > 4) ? std::atof(argv[4]) : 0.5;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> bit_dist(0, 1);
    
    time_t st = time(0);
    std::cout << "\n";
    std::cout << "  Fractal iO — Layer 1 (iO Core) + Layer 2 (Chaos/Stability)\n";
    std::cout << "  Layer 1 φ: Circuit A  |  Layer 1 ψ: Circuit B\n";
    std::cout << "  Layer 2 φ: Chaos      |  Layer 2 ψ: Stability\n";
    std::cout << "  Same structure, encrypted routing, fractal depth\n\n";

    SecureContext sc = create_context(ring_dim, depth, 256);
    
    std::cout << "  Functional Verification:\n";
    std::cout << "  X Y Z | L1φ | L1ψ | L2φ(chaos) | L2ψ(stable)\n";
    std::cout << "  " << std::string(55, '-') << "\n";
    
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            for (int z = 0; z <= 1; z++) {
                DualGate l1 = layer1_iO(sc, x, y, z);
                DualGate l2 = fractal_iO(sc, x, y, z);
                
                int l1_phi = (int)purify(reveal(l1, sc, PHI), threshold);
                int l1_psi = (int)purify(reveal(l1, sc, PSI), threshold);
                double l2_phi_raw = reveal(l2, sc, PHI);
                double l2_psi_raw = reveal(l2, sc, PSI);
                
                std::cout << "  " << x << " " << y << " " << z << " | "
                          << std::setw(3) << l1_phi << " | "
                          << std::setw(3) << l1_psi << " | "
                          << std::fixed << std::setprecision(4) << std::setw(10) << l2_phi_raw << " | "
                          << std::setw(10) << l2_psi_raw << "\n";
            }
        }
    }
    
    std::cout << "\n  Indistinguishability — Layer 2 Chaos vs Stability (" << trials << " trials):\n";
    std::vector<double> chaos_vals, stable_vals;
    
    for (int t = 0; t < trials; t++) {
        SecureContext sc2 = create_context(ring_dim, depth, 256);
        double x = bit_dist(gen), y = bit_dist(gen), z = bit_dist(gen);
        
        DualGate l2 = fractal_iO(sc2, x, y, z);
        chaos_vals.push_back(reveal(l2, sc2, PHI));
        stable_vals.push_back(reveal(l2, sc2, PSI));
    }
    
    double chaos_mean = 0, stable_mean = 0;
    for (int i = 0; i < trials; i++) {
        chaos_mean += chaos_vals[i];
        stable_mean += stable_vals[i];
    }
    chaos_mean /= trials;
    stable_mean /= trials;
    
    double chaos_var = 0, stable_var = 0;
    for (int i = 0; i < trials; i++) {
        chaos_var += (chaos_vals[i] - chaos_mean) * (chaos_vals[i] - chaos_mean);
        stable_var += (stable_vals[i] - stable_mean) * (stable_vals[i] - stable_mean);
    }
    chaos_var /= trials;
    stable_var /= trials;
    
    std::cout << "  Chaos mean: " << std::fixed << std::setprecision(6) << chaos_mean 
              << " var: " << chaos_var << "\n";
    std::cout << "  Stable mean: " << stable_mean << " var: " << stable_var << "\n";
    std::cout << "  Distributions: " << (std::abs(chaos_mean - stable_mean) < 0.01 ? "SIMILAR" : "DIFFERENT") << "\n\n";
    
    time_t et = time(0);
    std::cout << "  Ended: " << ctime(&et) << "\n";
    return 0;
}
