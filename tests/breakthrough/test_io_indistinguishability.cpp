#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <cstdlib>
#include <random>
#include <vector>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;
const double PSI = (1.0 - std::sqrt(5.0)) / 2.0;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
struct SecureContext { CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> kp; };

SecureContext create_context() {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(20);
    p.SetScalingModSize(50);
    p.SetBatchSize(256);
    p.SetRingDim(2048);
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

// NOT gate
DualGate gate_not(SecureContext& sc, DualGate& X) {
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto neg_one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {sc.cc->EvalSub(one, X.a), sc.cc->EvalMult(neg_one, X.b)};
}

// NAND gate
DualGate gate_nand(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a_out = sc.cc->EvalMult(X.a, Y.a);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    DualGate and_result = {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
    return gate_not(sc, and_result);
}

// 4 functionally equivalent AND gates
DualGate gate_standard(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a_out = sc.cc->EvalMult(X.a, Y.a);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    return {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

DualGate gate_double_not(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate nand_result = gate_nand(sc, X, Y);
    return gate_not(sc, nand_result);
}

DualGate gate_demorgan(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate not_x = gate_not(sc, X);
    DualGate not_y = gate_not(sc, Y);
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto or_a = sc.cc->EvalSub(one, sc.cc->EvalMult(
        sc.cc->EvalSub(one, not_x.a), sc.cc->EvalSub(one, not_y.a)));
    auto or_b = sc.cc->EvalAdd(
        sc.cc->EvalAdd(
            sc.cc->EvalMult(sc.cc->EvalSub(one, not_x.a), not_y.b),
            sc.cc->EvalMult(not_x.b, sc.cc->EvalSub(one, not_y.a))
        ), sc.cc->EvalMult(not_x.b, not_y.b));
    DualGate or_result = {or_a, or_b};
    return gate_not(sc, or_result);
}

DualGate gate_arithmetic(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto inv_phi = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0/PHI});
    auto x_phi = sc.cc->EvalAdd(X.a, sc.cc->EvalMult(X.b, phi_pt));
    auto y_phi = sc.cc->EvalAdd(Y.a, sc.cc->EvalMult(Y.b, phi_pt));
    auto prod = sc.cc->EvalMult(x_phi, y_phi);
    auto a_out = sc.cc->EvalMult(prod, inv_phi);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    return {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> gate_dist(0, 3);
    
    typedef DualGate (*GateFunc)(SecureContext&, DualGate&, DualGate&);
    GateFunc gates[4] = {gate_standard, gate_double_not, gate_demorgan, gate_arithmetic};
    std::string gate_names[4] = {"Standard", "NOT(NAND)", "De Morgan", "Arithmetic"};
    
    std::cout << "\n";
    std::cout << "  iO Indistinguishability Stress Test\n";
    std::cout << "  4 functionally equivalent, structurally different AND gates\n\n";

    // Test 1: Functional Equivalence — all inputs
    std::cout << "  TEST 1: Functional Equivalence (All Gates, All Inputs)\n";
    std::cout << "  " << std::string(60, '-') << "\n";
    
    SecureContext sc1 = create_context();
    int eq_ok = 0, eq_total = 0;
    
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            DualGate X = encrypt_input(sc1, (double)x);
            DualGate Y = encrypt_input(sc1, (double)y);
            int expected = x & y;
            bool all_same = true;
            
            for (int g = 0; g < 4; g++) {
                DualGate result = gates[g](sc1, X, Y);
                double val = reveal(result, sc1, PHI);
                int bit = (val > 0.5) ? 1 : 0;
                if (bit != expected) all_same = false;
            }
            
            eq_total++;
            if (all_same) eq_ok++;
            
            std::cout << "  Input (" << x << "," << y << "): "
                      << (all_same ? "ALL 4 GATES MATCH" : "MISMATCH")
                      << " (expected " << expected << ")\n";
        }
    }
    std::cout << "  Result: " << eq_ok << "/" << eq_total << " inputs pass\n\n";

    // Test 2: Indistinguishability Game — attacker tries to guess which gate
    std::cout << "  TEST 2: Indistinguishability Game (Attacker's View)\n";
    std::cout << "  " << std::string(60, '-') << "\n";
    
    int trials = 10000;
    std::vector<int> gate_usage(4, 0);
    std::vector<int> attacker_correct(4, 0);
    std::vector<int> output_0(4, 0);
    std::vector<int> output_1(4, 0);
    
    for (int t = 0; t < trials; t++) {
        SecureContext sc = create_context();
        int gate_idx = gate_dist(gen);
        gate_usage[gate_idx]++;
        
        // Random input
        int x = gate_dist(gen) % 2;
        int y = gate_dist(gen) % 2;
        
        DualGate X = encrypt_input(sc, (double)x);
        DualGate Y = encrypt_input(sc, (double)y);
        
        DualGate result = gates[gate_idx](sc, X, Y);
        double val = reveal(result, sc, PHI);
        int bit = (val > 0.5) ? 1 : 0;
        
        if (bit == 0) output_0[gate_idx]++;
        else output_1[gate_idx]++;
    }
    
    std::cout << "\n  Gate Usage Distribution:\n";
    for (int g = 0; g < 4; g++) {
        double pct = (double)gate_usage[g] / trials * 100.0;
        double pct_0 = gate_usage[g] > 0 ? (double)output_0[g] / gate_usage[g] * 100.0 : 0;
        double pct_1 = gate_usage[g] > 0 ? (double)output_1[g] / gate_usage[g] * 100.0 : 0;
        std::cout << "  " << std::setw(12) << gate_names[g] << ": "
                  << std::setw(5) << gate_usage[g] << " uses ("
                  << std::fixed << std::setprecision(1) << pct << "%)"
                  << " | Output 0: " << std::setprecision(1) << pct_0 << "%"
                  << " | Output 1: " << pct_1 << "%\n";
    }
    
    // Attacker's best strategy: always guess the most common gate
    int max_usage = 0;
    for (int g = 0; g < 4; g++) {
        if (gate_usage[g] > max_usage) max_usage = gate_usage[g];
    }
    double attacker_advantage = (double)max_usage / trials * 100.0;
    
    std::cout << "\n  Attacker's Best Guess: " << std::fixed << std::setprecision(1) 
              << attacker_advantage << "% (random guess = 25%)\n";
    std::cout << "  Attacker Advantage: " << std::setprecision(1) 
              << (attacker_advantage - 25.0) << "% over random\n";
    
    if (attacker_advantage < 30.0) {
        std::cout << "  Status: INDISTINGUISHABLE (attacker near random)\n";
    } else if (attacker_advantage < 35.0) {
        std::cout << "  Status: WEAKLY DISTINGUISHABLE\n";
    } else {
        std::cout << "  Status: DISTINGUISHABLE\n";
    }
    
    // Test 3: Output distribution per gate
    std::cout << "\n  TEST 3: Output Distribution Per Gate\n";
    std::cout << "  " << std::string(60, '-') << "\n";
    
    for (int g = 0; g < 4; g++) {
        SecureContext sc = create_context();
        std::cout << "  " << gate_names[g] << ":\n";
        for (int x = 0; x <= 1; x++) {
            for (int y = 0; y <= 1; y++) {
                DualGate X = encrypt_input(sc, (double)x);
                DualGate Y = encrypt_input(sc, (double)y);
                DualGate result = gates[g](sc, X, Y);
                double phi_val = reveal(result, sc, PHI);
                double psi_val = reveal(result, sc, PSI);
                std::cout << "    (" << x << "," << y << "): φ=" << std::fixed << std::setprecision(6) << phi_val
                          << " ψ=" << std::setprecision(6) << psi_val << "\n";
            }
        }
        std::cout << "\n";
    }
    
    return 0;
}
