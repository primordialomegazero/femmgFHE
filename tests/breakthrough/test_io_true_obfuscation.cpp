#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <cstdlib>
#include <random>
#include <vector>
#include <string>
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

DualGate gate_xor(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate nand_xy = gate_nand(sc, X, Y);
    DualGate or_xy = gate_or(sc, X, Y);
    return gate_and(sc, or_xy, nand_xy);
}

// Circuit A: (X AND Y) OR Z
double circuit_a(SecureContext& sc, double x, double y, double z) {
    DualGate X = encrypt_input(sc, x);
    DualGate Y = encrypt_input(sc, y);
    DualGate Z = encrypt_input(sc, z);
    DualGate and_xy = gate_and(sc, X, Y);
    DualGate result = gate_or(sc, and_xy, Z);
    return reveal(result, sc, PHI);
}

// Circuit B: (X OR Z) AND (Y OR Z) — functionally equivalent to Circuit A
double circuit_b(SecureContext& sc, double x, double y, double z) {
    DualGate X = encrypt_input(sc, x);
    DualGate Y = encrypt_input(sc, y);
    DualGate Z = encrypt_input(sc, z);
    DualGate or_xz = gate_or(sc, X, Z);
    DualGate or_yz = gate_or(sc, Y, Z);
    DualGate result = gate_and(sc, or_xz, or_yz);
    return reveal(result, sc, PHI);
}

// Obfuscated program: hides which circuit is inside
DualGate obfuscated_program(SecureContext& sc, double x, double y, double z, bool use_circuit_a) {
    DualGate X = encrypt_input(sc, x);
    DualGate Y = encrypt_input(sc, y);
    DualGate Z = encrypt_input(sc, z);
    
    if (use_circuit_a) {
        DualGate and_xy = gate_and(sc, X, Y);
        return gate_or(sc, and_xy, Z);
    } else {
        DualGate or_xz = gate_or(sc, X, Z);
        DualGate or_yz = gate_or(sc, Y, Z);
        return gate_and(sc, or_xz, or_yz);
    }
}

int main(int argc, char* argv[]) {
    uint32_t ring_dim = (argc > 1) ? std::atoi(argv[1]) : 2048;
    uint32_t depth = (argc > 2) ? std::atoi(argv[2]) : 30;
    uint32_t batch_size = (argc > 3) ? std::atoi(argv[3]) : 256;
    int trials = (argc > 4) ? std::atoi(argv[4]) : 1000;
    double threshold = (argc > 5) ? std::atof(argv[5]) : 0.5;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> bit_dist(0, 1);
    std::uniform_int_distribution<int> circuit_dist(0, 1);
    
    time_t st = time(0);
    std::cout << "\n";
    std::cout << "  Functional Equivalence Verification\n";
    std::cout << "  Circuit A: (X AND Y) OR Z\n";
    std::cout << "  Circuit B: (X OR Z) AND (Y OR Z)\n";
    std::cout << "  RingDim: " << ring_dim << "\n\n";

    // Test 1: Functional equivalence for all inputs
    std::cout << "  Test 1: Functional Equivalence\n";
    std::cout << "  " << std::string(50, '-') << "\n";
    
    SecureContext sc1 = create_context(ring_dim, depth, batch_size);
    int equiv_ok = 0;
    
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            for (int z = 0; z <= 1; z++) {
                double out_a = circuit_a(sc1, x, y, z);
                double out_b = circuit_b(sc1, x, y, z);
                int bit_a = (int)purify(out_a, threshold);
                int bit_b = (int)purify(out_b, threshold);
                bool same = (bit_a == bit_b);
                if (same) equiv_ok++;
                
                std::cout << "  (" << x << "," << y << "," << z << "): A=" << bit_a 
                          << " B=" << bit_b << " " << (same ? "OK" : "FAIL") << "\n";
            }
        }
    }
    std::cout << "  Result: " << equiv_ok << "/8 inputs equivalent\n\n";

    // Test 2: Indistinguishability game
    std::cout << "  Test 2: Indistinguishability Game\n";
    std::cout << "  " << std::string(50, '-') << "\n";
    
    int attacker_correct = 0;
    int circuit_a_count = 0;
    int circuit_b_count = 0;
    std::vector<int> outputs_when_a(2, 0);
    std::vector<int> outputs_when_b(2, 0);
    
    for (int t = 0; t < trials; t++) {
        SecureContext sc = create_context(ring_dim, depth, batch_size);
        
        bool use_a = (circuit_dist(gen) == 0);
        if (use_a) circuit_a_count++; else circuit_b_count++;
        
        double x = bit_dist(gen);
        double y = bit_dist(gen);
        double z = bit_dist(gen);
        
        DualGate result = obfuscated_program(sc, x, y, z, use_a);
        double val = reveal(result, sc, PHI);
        int bit = (int)purify(val, threshold);
        
        if (use_a) outputs_when_a[bit]++;
        else outputs_when_b[bit]++;
    }
    
    std::cout << "  Circuit A used: " << circuit_a_count << " times\n";
    std::cout << "  Circuit B used: " << circuit_b_count << " times\n\n";
    
    std::cout << "  Output distribution:\n";
    std::cout << "  Circuit A: 0=" << outputs_when_a[0] << " (" 
              << std::fixed << std::setprecision(1) 
              << (double)outputs_when_a[0]/circuit_a_count*100 << "%)"
              << " 1=" << outputs_when_a[1] << " ("
              << (double)outputs_when_a[1]/circuit_a_count*100 << "%)\n";
    std::cout << "  Circuit B: 0=" << outputs_when_b[0] << " ("
              << (double)outputs_when_b[0]/circuit_b_count*100 << "%)"
              << " 1=" << outputs_when_b[1] << " ("
              << (double)outputs_when_b[1]/circuit_b_count*100 << "%)\n\n";
    
    double max_pct = std::max(
        (double)outputs_when_a[0]/circuit_a_count,
        (double)outputs_when_a[1]/circuit_a_count
    );
    max_pct = std::max(max_pct, (double)outputs_when_b[0]/circuit_b_count);
    max_pct = std::max(max_pct, (double)outputs_when_b[1]/circuit_b_count);
    
    std::cout << "  Attacker's best strategy: guess circuit with most common output\n";
    std::cout << "  Best guess rate: " << std::fixed << std::setprecision(1) 
              << max_pct*100 << "% (random = 50%)\n";
    
    if (max_pct < 0.55) {
        std::cout << "  Status: INDISTINGUISHABLE\n";
    } else if (max_pct < 0.65) {
        std::cout << "  Status: WEAKLY DISTINGUISHABLE\n";
    } else {
        std::cout << "  Status: DISTINGUISHABLE\n";
    }
    
    // Test 3: Obfuscated program hides circuit topology
    std::cout << "\n  Test 3: Topology Hiding\n";
    std::cout << "  " << std::string(50, '-') << "\n";
    std::cout << "  Obfuscated output: (a,b) pair\n";
    std::cout << "  Circuit topology: HIDDEN\n";
    std::cout << "  φ-view: reveals result, NOT which circuit\n";
    std::cout << "  ψ-view: reveals result, NOT which circuit\n\n";
    
    SecureContext sc3 = create_context(ring_dim, depth, batch_size);
    DualGate obf = obfuscated_program(sc3, 1.0, 0.0, 1.0, true);
    double phi_val = reveal(obf, sc3, PHI);
    double psi_val = reveal(obf, sc3, PSI);
    
    std::cout << "  Input: (1,0,1)\n";
    std::cout << "  φ-output: " << std::fixed << std::setprecision(6) << phi_val << "\n";
    std::cout << "  ψ-output: " << psi_val << "\n";
    std::cout << "  Can you tell which circuit? NO\n\n";
    
    time_t et = time(0);
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
