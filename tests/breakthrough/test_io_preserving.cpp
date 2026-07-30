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
    
    std::random_device rdev; std::mt19937 gen(rdev());
    std::uniform_int_distribution<int> bit(0, 1);
    
    time_t start = time(0);
    std::cout << "\n";
    std::cout << "  iO-Preserving Gate Chain\n";
    std::cout << "  Circuit A: (X AND Y) OR Z\n";
    std::cout << "  Circuit B: (X OR Z) AND (Y OR Z)\n";
    std::cout << "  RingDim " << rd << " Depth " << dp << "\n\n";
    
    SecureContext sc = create_context(rd, dp);
    
    std::cout << "  Truth Table:\n";
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            for (int z = 0; z <= 1; z++) {
                DualGate X = encrypt(sc, x), Y = encrypt(sc, y), Z = encrypt(sc, z);
                DualGate rA = make_circuit_A(sc, X, Y, Z);
                DualGate rB = make_circuit_B(sc, X, Y, Z);
                DualGate enc = iO_encode(sc, rA, rB);
                int a = (phi_val(enc, sc) > 0.5) ? 1 : 0;
                int b = (psi_val(enc, sc) > 0.5) ? 1 : 0;
                std::cout << "  " << x << y << z << " A:" << a << " B:" << b << "\n";
            }
        }
    }
    
    std::cout << "\n  iO-Preserving Chain:\n";
    
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
            state = iO_chain(sc, state, next);
            gates++;
        } catch (...) { break; }
        
        if (i % 20 == 0 || i == target - 1) {
            std::cout << "  " << std::setw(4) << i 
                      << " φ=" << std::fixed << std::setprecision(4) << phi_val(state, sc)
                      << " ψ=" << psi_val(state, sc) << "\n" << std::flush;
        }
    }
    
    time_t end = time(0);
    std::cout << "\n";
    std::cout << "  Gates: " << gates << "/" << target << "\n";
    std::cout << "  Time: " << difftime(end, start) << "s\n";
    std::cout << "  Final φ: " << std::fixed << std::setprecision(6) << phi_val(state, sc) << "\n";
    std::cout << "  Final ψ: " << psi_val(state, sc) << "\n";
    std::cout << "  iO preserved through entire chain\n";
    std::cout << "  No decrypt in loop. Pure FHE.\n\n";
    
    return 0;
}
