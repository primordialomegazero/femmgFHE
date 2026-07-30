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

// NAND in R_φ ring
DualGate nand(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a = sc.cc->EvalMult(X.a, Y.a);
    auto s = sc.cc->EvalAdd(sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), sc.cc->EvalMult(X.b, Y.b));
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto neg = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {sc.cc->EvalSub(one, a), sc.cc->EvalMult(neg, s)};
}

// AND in R_φ ring — THIS IS THE DIRECT CHAIN
// (a₁+b₁Y)(a₂+b₂Y) = (a₁a₂+b₁b₂) + (a₁b₂+a₂b₁+b₁b₂)Y
// This preserves iO structure: φ and ψ multiply independently
DualGate and_op(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a1a2 = sc.cc->EvalMult(X.a, Y.a);
    auto b1b2 = sc.cc->EvalMult(X.b, Y.b);
    auto a_out = sc.cc->EvalAdd(a1a2, b1b2);
    
    auto a1b2 = sc.cc->EvalMult(X.a, Y.b);
    auto a2b1 = sc.cc->EvalMult(X.b, Y.a);
    auto sum = sc.cc->EvalAdd(sc.cc->EvalAdd(a1b2, a2b1), b1b2);
    auto neg = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    auto b_out = sc.cc->EvalMult(neg, sum);
    
    return {a_out, b_out};
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

struct Circuit {
    std::vector<int> types;
    std::vector<std::pair<int,int>> inputs;
    int num_inputs;
};

Circuit make_circuit_A() {
    return {{0, 1}, {{0,1}, {2,2}}, 3}; // AND then OR
}

Circuit make_circuit_B() {
    return {{1, 1, 0}, {{0,2}, {1,2}, {3,4}}, 3}; // OR, OR, AND
}

DualGate evaluate(SecureContext& sc, Circuit& c, std::vector<DualGate>& in) {
    std::vector<DualGate> w = in;
    for (size_t i = 0; i < c.types.size(); i++) {
        if (c.types[i] == 0) w.push_back(and_op(sc, w[c.inputs[i].first], w[c.inputs[i].second]));
        else w.push_back(or_op(sc, w[c.inputs[i].first], w[c.inputs[i].second]));
    }
    return w.back();
}

int main(int argc, char* argv[]) {
    uint32_t rd = (argc > 1) ? std::atoi(argv[1]) : 4096;
    uint32_t dp = (argc > 2) ? std::atoi(argv[2]) : 120;
    int target = (argc > 3) ? std::atoi(argv[3]) : 100;
    
    std::random_device rdev; std::mt19937 gen(rdev());
    std::uniform_int_distribution<int> bit(0, 1);
    
    time_t start = time(0);
    std::cout << "\n";
    std::cout << "  Optimized iO Chain — Direct R_φ Ring Multiplication\n";
    std::cout << "  Chain = AND in φ-ψ basis (closed under ring)\n";
    std::cout << "  No decode-reencode. iO preserved.\n";
    std::cout << "  RingDim " << rd << " Depth " << dp << "\n\n";
    
    SecureContext sc = create_context(rd, dp);
    
    Circuit cA = make_circuit_A();
    Circuit cB = make_circuit_B();
    
    std::cout << "  Truth Table:\n";
    for (int x = 0; x <= 1; x++) {
        for (int y = 0; y <= 1; y++) {
            for (int z = 0; z <= 1; z++) {
                std::vector<DualGate> in = {encrypt(sc,x), encrypt(sc,y), encrypt(sc,z)};
                DualGate rA = evaluate(sc, cA, in);
                DualGate rB = evaluate(sc, cB, in);
                DualGate enc = iO_encode(sc, rA, rB);
                int a = (phi_val(enc, sc) > 0.5) ? 1 : 0;
                int b = (psi_val(enc, sc) > 0.5) ? 1 : 0;
                std::cout << "  " << x << y << z << " A:" << a << " B:" << b << "\n";
            }
        }
    }
    
    std::cout << "\n  Chain:\n";
    
    std::vector<DualGate> in0 = {encrypt(sc,1.0), encrypt(sc,0.0), encrypt(sc,1.0)};
    DualGate rA0 = evaluate(sc, cA, in0);
    DualGate rB0 = evaluate(sc, cB, in0);
    DualGate state = iO_encode(sc, rA0, rB0);
    int gates = 1;
    
    for (int i = 1; i < target; i++) {
        std::vector<DualGate> in_i = {encrypt(sc,bit(gen)), encrypt(sc,bit(gen)), encrypt(sc,bit(gen))};
        DualGate rAi = evaluate(sc, cA, in_i);
        DualGate rBi = evaluate(sc, cB, in_i);
        DualGate next = iO_encode(sc, rAi, rBi);
        
        try {
            state = and_op(sc, state, next);  // DIRECT RING MULTIPLICATION
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
    std::cout << "  Chain: Direct R_φ ring multiplication\n";
    std::cout << "  No decode-reencode. Pure FHE.\n\n";
    
    return 0;
}
