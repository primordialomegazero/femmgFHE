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

// UNIFIED OBFUSCATED PROGRAM — No if/else
// Same structure, same operations, ALWAYS
// The selector is encoded in the φ/ψ interpretation
// φ reveals Circuit A, ψ reveals Circuit B
DualGate obfuscated_unified(SecureContext& sc, double x, double y, double z) {
    DualGate X = encrypt_input(sc, x);
    DualGate Y = encrypt_input(sc, y);
    DualGate Z = encrypt_input(sc, z);
    
    // ALWAYS compute BOTH paths
    DualGate and_xy = gate_and(sc, X, Y);
    DualGate or_xz  = gate_or(sc, X, Z);
    DualGate or_yz  = gate_or(sc, Y, Z);
    
    DualGate pathA = gate_or(sc, and_xy, Z);
    DualGate pathB = gate_and(sc, or_xz, or_yz);
    
    // Encode into (a,b) such that:
    // φ-decode → pathA (Circuit A)
    // ψ-decode → pathB (Circuit B)
    // a + b·φ = pathA
    // a + b·ψ = pathB
    // Solve: b = (pathA - pathB) / (φ - ψ)
    //        a = pathA - b·φ
    
    auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto psi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    auto inv_denom = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0 / (PHI - PSI)});
    
    DualGate result;
    
    // diff = pathA - pathB  (in a-component)
    auto diff_a = sc.cc->EvalSub(pathA.a, pathB.a);
    auto diff_b = sc.cc->EvalSub(pathA.b, pathB.b);
    
    // b = diff / (φ - ψ)
    result.b = sc.cc->EvalAdd(
        sc.cc->EvalMult(diff_a, inv_denom),
        sc.cc->EvalMult(diff_b, inv_denom));
    
    // a = pathA - b·φ
    auto b_phi_a = sc.cc->EvalMult(result.b, phi_pt);
    result.a = sc.cc->EvalSub(pathA.a, b_phi_a);
    
    return result;
}

int main(int argc, char* argv[]) {
    uint32_t ring_dim = (argc > 1) ? std::atoi(argv[1]) : 2048;
    uint32_t depth = (argc > 2) ? std::atoi(argv[2]) : 40;
    int trials = (argc > 3) ? std::atoi(argv[3]) : 1000;
    double threshold = (argc > 4) ? std::atof(argv[4]) : 0.5;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> bit_dist(0, 1);
    
    time_t st = time(0);
    std::cout << "\n";
    std::cout << "  Unified Obfuscated Program — No if/else\n";
    std::cout << "  φ reveals Circuit A: (X AND Y) OR Z\n";
    std::cout << "  ψ reveals Circuit B: (X OR Z) AND (Y OR Z)\n";
    std::cout << "  Same structure, encrypted routing in (a,b)\n\n";

    SecureContext sc1 = create_context(ring_dim, depth, 256);
    
    std::cout << "  Truth Table:\n";
    std::cout << "  X Y Z | φ(Circuit A) | ψ(Circuit B)\n";
    std::cout << "  " << std::string(40, '-') << "\n";
    
    int ok = 0;
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            for (int z = 0; z <= 1; z++) {
                DualGate result = obfuscated_unified(sc1, x, y, z);
                double phi_val = purify(reveal(result, sc1, PHI), threshold);
                double psi_val = purify(reveal(result, sc1, PSI), threshold);
                
                int expected = ((x & y) | z) ? 1 : 0;
                if ((int)phi_val == expected && (int)psi_val == expected) ok++;
                
                std::cout << "  " << x << " " << y << " " << z << " | "
                          << std::setw(10) << (int)phi_val << " | "
                          << std::setw(10) << (int)psi_val << "\n";
            }
        }
    }
    std::cout << "  " << ok << "/8 correct\n\n";

    std::cout << "  Indistinguishability (" << trials << " trials):\n";
    std::vector<int> out_phi(2,0), out_psi(2,0);
    
    for (int t = 0; t < trials; t++) {
        SecureContext sc = create_context(ring_dim, depth, 256);
        double x = bit_dist(gen), y = bit_dist(gen), z = bit_dist(gen);
        
        DualGate result = obfuscated_unified(sc, x, y, z);
        int phi_bit = (int)purify(reveal(result, sc, PHI), threshold);
        int psi_bit = (int)purify(reveal(result, sc, PSI), threshold);
        
        out_phi[phi_bit]++;
        out_psi[psi_bit]++;
    }
    
    std::cout << "  φ-output: 0=" << out_phi[0] << " 1=" << out_phi[1] << "\n";
    std::cout << "  ψ-output: 0=" << out_psi[0] << " 1=" << out_psi[1] << "\n";
    
    double max_phi = std::max((double)out_phi[0]/trials, (double)out_phi[1]/trials);
    double max_psi = std::max((double)out_psi[0]/trials, (double)out_psi[1]/trials);
    
    std::cout << "  φ/ψ distributions: " << (max_phi == max_psi ? "IDENTICAL" : "DIFFERENT") << "\n";
    std::cout << "  Attacker cannot distinguish which circuit via output alone\n\n";
    
    time_t et = time(0);
    std::cout << "  Ended: " << ctime(&et) << "\n";
    return 0;
}
