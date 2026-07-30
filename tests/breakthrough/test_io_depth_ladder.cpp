#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <cstdlib>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
struct SecureContext { CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> kp; };

SecureContext create_context(uint32_t ring_dim, uint32_t depth) {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(depth); p.SetScalingModSize(50); p.SetBatchSize(256);
    p.SetRingDim(ring_dim); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    return {cc, kp};
}

DualGate input(SecureContext& sc, double v) {
    return {sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

DualGate nand(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a = sc.cc->EvalMult(X.a, Y.a);
    auto s = sc.cc->EvalAdd(sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), sc.cc->EvalMult(X.b, Y.b));
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto neg = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {sc.cc->EvalSub(one, a), sc.cc->EvalMult(neg, s)};
}

DualGate and_op(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto n = nand(sc, X, Y);
    return nand(sc, n, n);
}

DualGate iO_core(SecureContext& sc, DualGate& X, DualGate& Y, DualGate& Z) {
    DualGate xy = and_op(sc, X, Y);
    DualGate xz = and_op(sc, X, Z);
    DualGate yz = and_op(sc, Y, Z);
    DualGate d = nand(sc, xy, xy);
    DualGate e = nand(sc, xz, yz);
    return nand(sc, d, e);
}

int test_depth(uint32_t ring_dim, uint32_t depth_budget, int max_test) {
    SecureContext sc = create_context(ring_dim, depth_budget);
    
    DualGate X = input(sc, 1.0);
    DualGate Y = input(sc, 0.0);
    DualGate Z = input(sc, 1.0);
    DualGate state = iO_core(sc, X, Y, Z);
    int gates = 1;
    
    for (int i = 1; i < max_test; i++) {
        DualGate Xi = input(sc, 0.0);
        DualGate Yi = input(sc, 1.0);
        DualGate Zi = input(sc, 0.0);
        DualGate next = iO_core(sc, Xi, Yi, Zi);
        try {
            state = and_op(sc, state, next);
            gates++;
        } catch (...) { break; }
    }
    
    Plaintext pta, ptb;
    sc.cc->Decrypt(sc.kp.secretKey, state.a, &pta);
    sc.cc->Decrypt(sc.kp.secretKey, state.b, &ptb);
    double phi_val = pta->GetCKKSPackedValue()[0].real() + ptb->GetCKKSPackedValue()[0].real() * PHI;
    double accuracy = (phi_val > 0.5) ? 1.0 : (phi_val < -0.5 ? -1.0 : phi_val);
    
    std::cout << "  R" << ring_dim << " D" << std::setw(3) << depth_budget
              << " | " << std::setw(4) << gates << " gates"
              << " | φ=" << std::fixed << std::setprecision(4) << accuracy
              << " | " << (gates >= max_test ? "MAX" : "STOP") << "\n" << std::flush;
    
    return gates;
}

int main() {
    std::cout << "\n  Pure FHE iO Depth Ladder\n";
    std::cout << "  iO Core: NAND-based, φ/ψ dual, no decrypt, no bootstrap\n\n";
    
    int max_test = 500;
    int best_gates = 0;
    uint32_t best_rd = 0, best_dp = 0;
    
    for (uint32_t rd : {2048, 4096, 8192}) {
        for (uint32_t dp : {30, 60, 90, 120, 150, 180, 210, 240, 270, 300}) {
            if (rd == 2048 && dp > 120) continue;
            if (rd == 4096 && dp > 240) continue;
            
            try {
                int gates = test_depth(rd, dp, max_test);
                if (gates > best_gates) {
                    best_gates = gates;
                    best_rd = rd;
                    best_dp = dp;
                }
            } catch (...) {
                std::cout << "  R" << rd << " D" << std::setw(3) << dp << " | FAIL\n" << std::flush;
            }
        }
    }
    
    std::cout << "\n  Best: R" << best_rd << " D" << best_dp << " = " << best_gates << " gates\n\n";
    
    return 0;
}
