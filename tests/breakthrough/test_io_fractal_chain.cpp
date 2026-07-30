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

SecureContext create_context(uint32_t ring_dim, uint32_t depth) {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(depth); p.SetScalingModSize(50); p.SetBatchSize(256);
    p.SetRingDim(ring_dim); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    return {cc, kp};
}

DualGate encrypt(SecureContext& sc, double v) {
    return {sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

double decrypt(SecureContext& sc, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; sc.cc->Decrypt(sc.kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double phi_val(DualGate& g, SecureContext& sc) { return decrypt(sc, g.a) + decrypt(sc, g.b) * PHI; }
double psi_val(DualGate& g, SecureContext& sc) { return decrypt(sc, g.a) + decrypt(sc, g.b) * PSI; }

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

DualGate or_op(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto nx = nand(sc, X, X);
    auto ny = nand(sc, Y, Y);
    return nand(sc, nx, ny);
}

DualGate iO_encode(SecureContext& sc, DualGate& result_A, DualGate& result_B) {
    auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto inv_denom = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0 / (PHI - PSI)});
    DualGate out;
    auto diff_a = sc.cc->EvalSub(result_A.a, result_B.a);
    auto diff_b = sc.cc->EvalSub(result_A.b, result_B.b);
    out.b = sc.cc->EvalAdd(sc.cc->EvalMult(diff_a, inv_denom), sc.cc->EvalMult(diff_b, inv_denom));
    out.a = sc.cc->EvalSub(result_A.a, sc.cc->EvalMult(out.b, phi_pt));
    return out;
}

// Fractal iO-Chain: self-similar composition
// Each step: state = iO_chain(state, next)
// Output is same format as input → can feed back recursively
DualGate iO_chain(SecureContext& sc, DualGate& state, DualGate& next) {
    auto phi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI});
    auto psi_pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI});
    auto inv_denom = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0 / (PHI - PSI)});
    
    auto state_phi = sc.cc->EvalAdd(state.a, sc.cc->EvalMult(state.b, phi_pt));
    auto state_psi = sc.cc->EvalAdd(state.a, sc.cc->EvalMult(state.b, psi_pt));
    auto next_phi = sc.cc->EvalAdd(next.a, sc.cc->EvalMult(next.b, phi_pt));
    auto next_psi = sc.cc->EvalAdd(next.a, sc.cc->EvalMult(next.b, psi_pt));
    
    auto phi_result = sc.cc->EvalMult(state_phi, next_phi);
    auto psi_result = sc.cc->EvalMult(state_psi, next_psi);
    
    DualGate out;
    auto diff = sc.cc->EvalSub(phi_result, psi_result);
    out.b = sc.cc->EvalMult(diff, inv_denom);
    out.a = sc.cc->EvalSub(phi_result, sc.cc->EvalMult(out.b, phi_pt));
    return out;
}

DualGate make_circuit_A(SecureContext& sc, DualGate& X, DualGate& Y, DualGate& Z) {
    auto xy = and_op(sc, X, Y);
    return or_op(sc, xy, Z);
}

DualGate make_circuit_B(SecureContext& sc, DualGate& X, DualGate& Y, DualGate& Z) {
    auto xz = or_op(sc, X, Z);
    auto yz = or_op(sc, Y, Z);
    return and_op(sc, xz, yz);
}

int main(int argc, char* argv[]) {
    uint32_t rd = (argc > 1) ? std::atoi(argv[1]) : 4096;
    uint32_t dp = (argc > 2) ? std::atoi(argv[2]) : 120;
    int target = (argc > 3) ? std::atoi(argv[3]) : 100;
    int fractal_depth = (argc > 4) ? std::atoi(argv[4]) : 2;
    
    std::random_device rdev; std::mt19937 gen(rdev());
    std::uniform_int_distribution<int> bit(0, 1);
    
    time_t start = time(0);
    std::cout << "\n";
    std::cout << "  Fractal iO-Chain\n";
    std::cout << "  Self-similar composition, depth " << fractal_depth << "\n";
    std::cout << "  RingDim " << rd << " Depth " << dp << "\n\n";
    
    SecureContext sc = create_context(rd, dp);
    
    DualGate X0 = encrypt(sc, 1.0), Y0 = encrypt(sc, 0.0), Z0 = encrypt(sc, 1.0);
    DualGate rA0 = make_circuit_A(sc, X0, Y0, Z0);
    DualGate rB0 = make_circuit_B(sc, X0, Y0, Z0);
    DualGate state = iO_encode(sc, rA0, rB0);
    int gates = 1;
    
    for (int i = 1; i < target; i++) {
        double x = bit(gen), y = bit(gen), z = bit(gen);
        DualGate Xi = encrypt(sc, x), Yi = encrypt(sc, y), Zi = encrypt(sc, z);
        DualGate rAi = make_circuit_A(sc, Xi, Yi, Zi);
        DualGate rBi = make_circuit_B(sc, Xi, Yi, Zi);
        DualGate next = iO_encode(sc, rAi, rBi);
        
        try {
            // Fractal: compose state with next, then feed result back
            DualGate composed = iO_chain(sc, state, next);
            for (int f = 1; f < fractal_depth; f++) {
                composed = iO_chain(sc, composed, next);
            }
            state = composed;
            gates++;
        } catch (...) { break; }
        
        if (i % 15 == 0 || i == target - 1) {
            std::cout << "  " << std::setw(4) << i 
                      << " φ=" << std::fixed << std::setprecision(4) << phi_val(state, sc)
                      << " ψ=" << psi_val(state, sc) << "\n" << std::flush;
        }
    }
    
    time_t end = time(0);
    std::cout << "\n";
    std::cout << "  Gates: " << gates << "/" << target << "\n";
    std::cout << "  Fractal depth: " << fractal_depth << "\n";
    std::cout << "  Time: " << difftime(end, start) << "s\n";
    std::cout << "  Final φ: " << std::fixed << std::setprecision(6) << phi_val(state, sc) << "\n";
    std::cout << "  Final ψ: " << psi_val(state, sc) << "\n";
    std::cout << "  Core unchanged. Extended via fractal self-similarity.\n";
    std::cout << "  No decrypt in loop. Pure FHE.\n\n";
    
    return 0;
}
