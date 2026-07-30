#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <cstdlib>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;
const double PSI = (1.0 - std::sqrt(5.0)) / 2.0;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

SecureContext create_context(uint32_t ring_dim, uint32_t depth, uint32_t batch_size) {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(depth); p.SetScalingModSize(50); p.SetBatchSize(batch_size);
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

DualGate and(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a = sc.cc->EvalMult(X.a, Y.a);
    auto s = sc.cc->EvalAdd(sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), sc.cc->EvalMult(X.b, Y.b));
    auto neg = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {a, sc.cc->EvalMult(neg, s)};
}

DualGate iO(SecureContext& sc, DualGate& X, DualGate& Y, DualGate& Z) {
    DualGate a = and(sc, X, Y);
    DualGate b = and(sc, X, Z);
    DualGate c = and(sc, Y, Z);
    DualGate d = nand(sc, a, a);
    DualGate e = nand(sc, b, c);
    return nand(sc, d, e);
}

int main(int argc, char* argv[]) {
    uint32_t rd = (argc > 1) ? std::atoi(argv[1]) : 4096;
    uint32_t dp = (argc > 2) ? std::atoi(argv[2]) : 120;
    int target = (argc > 3) ? std::atoi(argv[3]) : 200;
    
    time_t start = time(0);
    std::cout << "\n  Pure FHE Depth Test\n";
    std::cout << "  RingDim " << rd << " Depth " << dp << "\n";
    std::cout << "  No decrypt until end\n\n";
    
    SecureContext sc = create_context(rd, dp, 256);
    
    DualGate X = input(sc, 1.0);
    DualGate Y = input(sc, 0.0);
    DualGate Z = input(sc, 1.0);
    
    DualGate state = iO(sc, X, Y, Z);
    int gates = 1;
    
    for (int i = 1; i < target; i++) {
        DualGate Xi = input(sc, 0.0);
        DualGate Yi = input(sc, 1.0);
        DualGate Zi = input(sc, 0.0);
        DualGate next = iO(sc, Xi, Yi, Zi);
        try {
            state = and(sc, state, next);
            gates++;
        } catch (...) {
            break;
        }
    }
    
    time_t end = time(0);
    
    Plaintext pta, ptb;
    sc.cc->Decrypt(sc.kp.secretKey, state.a, &pta);
    sc.cc->Decrypt(sc.kp.secretKey, state.b, &ptb);
    double a = pta->GetCKKSPackedValue()[0].real();
    double b = ptb->GetCKKSPackedValue()[0].real();
    double phi_val = a + b * PHI;
    
    std::cout << "  Gates: " << gates << "/" << target << "\n";
    std::cout << "  Time: " << difftime(end, start) << "s\n";
    std::cout << "  Final φ: " << std::fixed << std::setprecision(6) << phi_val << "\n\n";
    
    return 0;
}
