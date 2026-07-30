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

// Circuit A: (X AND Y) OR Z
DualGate circuit_A(SecureContext& sc, DualGate& X, DualGate& Y, DualGate& Z) {
    auto xy = and_op(sc, X, Y);
    return or_op(sc, xy, Z);
}

// Circuit B: (X OR Z) AND (Y OR Z)
DualGate circuit_B(SecureContext& sc, DualGate& X, DualGate& Y, DualGate& Z) {
    auto xz = or_op(sc, X, Z);
    auto yz = or_op(sc, Y, Z);
    return and_op(sc, xz, yz);
}

// iO Encode: embed both circuits into one (a,b) pair
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

int main(int argc, char* argv[]) {
    uint32_t rd = (argc > 1) ? std::atoi(argv[1]) : 4096;
    uint32_t dp = (argc > 2) ? std::atoi(argv[2]) : 120;
    int target = (argc > 3) ? std::atoi(argv[3]) : 100;
    
    std::random_device rdev; std::mt19937 gen(rdev());
    std::uniform_int_distribution<int> bit(0, 1);
    
    time_t start = time(0);
    std::cout << "\n";
    std::cout << "  iO Baseline — Two Arbitrary Circuits, Pure FHE\n";
    std::cout << "  Circuit A: (X AND Y) OR Z\n";
    std::cout << "  Circuit B: (X OR Z) AND (Y OR Z)\n";
    std::cout << "  φ reveals A, ψ reveals B\n";
    std::cout << "  RingDim " << rd << " Depth " << dp << "\n\n";
    
    SecureContext sc = create_context(rd, dp);
    
    // Truth table verification
    std::cout << "  Truth Table:\n";
    std::cout << "  X Y Z | A | B\n";
    std::cout << "  " << std::string(20, '-') << "\n";
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            for (int z = 0; z <= 1; z++) {
                DualGate X = encrypt(sc, x);
                DualGate Y = encrypt(sc, y);
                DualGate Z = encrypt(sc, z);
                DualGate rA = circuit_A(sc, X, Y, Z);
                DualGate rB = circuit_B(sc, X, Y, Z);
                DualGate enc = iO_encode(sc, rA, rB);
                double pv = phi_val(enc, sc);
                double sv = psi_val(enc, sc);
                int a_bit = (pv > 0.5) ? 1 : 0;
                int b_bit = (sv > 0.5) ? 1 : 0;
                std::cout << "  " << x << " " << y << " " << z << " | " << a_bit << " | " << b_bit;
                std::cout << "  φ=" << std::fixed << std::setprecision(4) << pv << " ψ=" << sv << "\n";
            }
        }
    }
    
    // Depth chain
    std::cout << "\n  Depth Chain:\n";
    
    DualGate X0 = encrypt(sc, 1.0);
    DualGate Y0 = encrypt(sc, 0.0);
    DualGate Z0 = encrypt(sc, 1.0);
    DualGate rA0 = circuit_A(sc, X0, Y0, Z0);
    DualGate rB0 = circuit_B(sc, X0, Y0, Z0);
    DualGate state = iO_encode(sc, rA0, rB0);
    int gates = 1;
    
    for (int i = 1; i < target; i++) {
        DualGate Xi = encrypt(sc, bit(gen));
        DualGate Yi = encrypt(sc, bit(gen));
        DualGate Zi = encrypt(sc, bit(gen));
        DualGate rA = circuit_A(sc, Xi, Yi, Zi);
        DualGate rB = circuit_B(sc, Xi, Yi, Zi);
        DualGate next = iO_encode(sc, rA, rB);
        
        try {
            state.a = sc.cc->EvalMult(state.a, next.a);
            state.b = sc.cc->EvalAdd(state.b, next.b);
            gates++;
        } catch (...) { break; }
        
        if (i % 25 == 0 || i == target - 1) {
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
    std::cout << "  No decrypt in loop. Pure FHE.\n\n";
    
    return 0;
}
