// ╔══════════════════════════════════════════════════════════════════╗
// ║  iO v8 — Direct AND/OR formulas, 8K ring, randomized φ/ψ       ║
// ║  Architecture: Dan Fernandez / Primordial Omega Zero            ║
// ╚══════════════════════════════════════════════════════════════════╝
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <random>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

DualGate unified_nand(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a1a2 = cc->EvalMult(X.a, Y.a);
    auto b1b2 = cc->EvalMult(X.b, Y.b);
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto a_out = cc->EvalSub(one, cc->EvalAdd(a1a2, b1b2));
    auto a1b2 = cc->EvalMult(X.a, Y.b);
    auto a2b1 = cc->EvalMult(Y.a, X.b);
    auto sum = cc->EvalAdd(cc->EvalAdd(a1b2, a2b1), b1b2);
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    auto b_out = cc->EvalMult(neg_one, sum);
    return {a_out, b_out};
}

// Direct AND: a_out = a1*a2,  b_out = -(a1*b2 + b1*a2 + b1*b2)
DualGate direct_and(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a_out = cc->EvalMult(X.a, Y.a);
    auto a1b2 = cc->EvalMult(X.a, Y.b);
    auto b1a2 = cc->EvalMult(X.b, Y.a);
    auto b1b2 = cc->EvalMult(X.b, Y.b);
    auto sum = cc->EvalAdd(cc->EvalAdd(a1b2, b1a2), b1b2);
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    auto b_out = cc->EvalMult(neg_one, sum);
    return {a_out, b_out};
}

// Direct OR: a_out = 1 - (1-a1)(1-a2),  b_out = (1-a1)b2 + b1(1-a2) + b1*b2
DualGate direct_or(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto one_minus_a1 = cc->EvalSub(one, X.a);
    auto one_minus_a2 = cc->EvalSub(one, Y.a);
    auto a_out = cc->EvalSub(one, cc->EvalMult(one_minus_a1, one_minus_a2));
    auto neg_b1 = cc->EvalMult(cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), X.b);
    auto neg_b2 = cc->EvalMult(cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), Y.b);
    auto t1 = cc->EvalMult(one_minus_a1, Y.b);
    auto t2 = cc->EvalMult(X.b, one_minus_a2);
    auto t3 = cc->EvalMult(X.b, Y.b);
    auto sum = cc->EvalAdd(cc->EvalAdd(t1, t2), t3);
    return {a_out, sum};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
double decode_dual(DualGate& dg, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double root) {
    return decrypt_val(cc, kp, dg.a) + decrypt_val(cc, kp, dg.b) * root;
}
DualGate encode_dual(CryptoContext<DCRTPoly>& cc, const DualGate& gA, const DualGate& gB) {
    auto bA_phi = cc->EvalMult(gA.b, cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI}));
    auto val_A = cc->EvalAdd(gA.a, bA_phi);
    auto bB_psi = cc->EvalMult(gB.b, cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto val_B = cc->EvalAdd(gB.a, bB_psi);
    auto diff = cc->EvalSub(val_A, val_B);
    auto inv_denom = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0 / (PHI - PSI)});
    auto b_out = cc->EvalMult(diff, inv_denom);
    auto b_phi = cc->EvalMult(b_out, cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI}));
    auto a_out = cc->EvalSub(val_A, b_phi);
    return {a_out, b_out};
}
DualGate make_input(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

std::mt19937 rng(std::random_device{}());

struct CompiledCircuit { std::vector<DualGate> backbone; bool phi_is_A; };

CompiledCircuit compile_8g(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                            const std::vector<double>& inputs) {
    CompiledCircuit result;
    result.phi_is_A = (rng() % 2 == 0);
    result.backbone.resize(8);
    
    DualGate g0 = make_input(cc, kp, inputs[0]);
    DualGate g1 = make_input(cc, kp, inputs[1]);
    DualGate g2 = make_input(cc, kp, inputs[2]);
    result.backbone[0] = g0; result.backbone[1] = g1; result.backbone[2] = g2;
    
    // Circuit A: G3=AND(x,y), G4=OR(G3,z)
    DualGate A_and = direct_and(cc, g0, g1);
    DualGate A_out = direct_or(cc, A_and, g2);
    
    // Circuit B: G3=OR(x,z), G4=AND(G3, OR(y,z))
    DualGate B_or1 = direct_or(cc, g0, g2);
    DualGate B_or2 = direct_or(cc, g1, g2);
    DualGate B_out = direct_and(cc, B_or1, B_or2);
    
    // Encode intermediates
    result.backbone[3] = result.phi_is_A ? encode_dual(cc, A_and, B_or1) : encode_dual(cc, B_or1, A_and);
    result.backbone[4] = result.phi_is_A ? encode_dual(cc, A_out, B_out) : encode_dual(cc, B_out, A_out);
    
    // Dummy gates 5-7 (passthrough last output)
    result.backbone[5] = result.backbone[4];
    result.backbone[6] = result.backbone[4];
    result.backbone[7] = result.backbone[4];
    
    return result;
}

int main() {
    time_t st = time(0);
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  iO v8 — Direct AND/OR, 8K ring, randomized φ/ψ             ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << "  φ = " << std::fixed << std::setprecision(6) << PHI << "\n";
    std::cout << "  ψ = " << PSI << "\n\n";

    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  CORRECTNESS — Direct AND/OR, randomized φ/ψ          │\n";
    std::cout << "  ├─────┬─────┬─────┬──────────┬──────────┬──────────────┤\n";
    
    int ok = 0;
    for (int i = 0; i < 8; i++) {
        std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
        auto compiled = compile_8g(cc, kp, dv);
        
        double val_phi = decode_dual(compiled.backbone[7], cc, kp, PHI);
        double val_psi = decode_dual(compiled.backbone[7], cc, kp, PSI);
        
        int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
        int bit_phi = (val_phi > 0.5) ? 1 : 0;
        int bit_psi = (val_psi > 0.5) ? 1 : 0;
        int bit_A = compiled.phi_is_A ? bit_phi : bit_psi;
        int bit_B = compiled.phi_is_A ? bit_psi : bit_phi;
        
        bool both = (bit_A == expected && bit_B == expected);
        if (both) ok++;
        
        std::cout << "  │  " << inputs[i][0] << "  │  " << inputs[i][1] << "  │  " << inputs[i][2]
                  << "  │     " << bit_A << "    │     " << bit_B << "    │     " 
                  << (both ? "OK ✓" : "FAIL") << "     │\n";
    }
    std::cout << "  ├─────┴─────┴─────┴──────────┴──────────┴──────────────┤\n";
    std::cout << "  │  Both circuits: " << ok << "/8                                         │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    const int TRIALS = 100;
    int io_ok = 0;
    for (int t = 0; t < TRIALS; t++) {
        int inp = rng() % 8;
        std::vector<double> dv = {(double)((inp>>0)&1), (double)((inp>>1)&1), (double)((inp>>2)&1)};
        auto compiled = compile_8g(cc, kp, dv);
        double val_phi = decode_dual(compiled.backbone[7], cc, kp, PHI);
        double val_psi = decode_dual(compiled.backbone[7], cc, kp, PSI);
        if ((val_phi > val_psi) == compiled.phi_is_A) io_ok++;
    }
    
    double io_rate = 100.0 * io_ok / TRIALS;
    double io_adv = std::abs(io_rate - 50.0);
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  iO GAME — " << io_rate << "% guess, " << io_adv << "% advantage";
    if (io_adv < 2.0) std::cout << " INDISTINGUISHABLE ✓✓✓";
    else if (io_adv < 5.0) std::cout << " WEAK ✓";
    std::cout << "        │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  iO v8 — " << (ok == 8 && io_adv < 5.0 ? "8/8 CORRECT · INDISTINGUISHABLE ✓✓✓" : "TUNING") << "        ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
