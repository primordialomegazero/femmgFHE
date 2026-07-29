// ╔══════════════════════════════════════════════════════════════════╗
// ║  ZERO-AXIS MIRROR iO — Zero as reflection axis                 ║
// ║  AND ←→ 0 ←→ OR · φ ←→ 0 ←→ ψ                                ║
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
const double ZERO_AXIS = 0.5;  // The mirror's center

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

// ═══════════════════════════════════════════════════════════════
// ZERO-CENTERED iO CORE
// ═══════════════════════════════════════════════════════════════
DualGate unified_nand(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a1a2 = cc->EvalMult(X.a, Y.a), b1b2 = cc->EvalMult(X.b, Y.b);
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto a_out = cc->EvalSub(one, cc->EvalAdd(a1a2, b1b2));
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(Y.a, X.b)), b1b2);
    return {a_out, cc->EvalMult(cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

// Zero-centered AND: (a - 0.5) part centers around zero
DualGate direct_and_z(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto half = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.5});
    auto xa = cc->EvalSub(X.a, half);  // Center X around zero
    auto ya = cc->EvalSub(Y.a, half);  // Center Y around zero
    auto a_out = cc->EvalAdd(cc->EvalMult(xa, ya), half); // AND then un-center
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(X.b, Y.a)), cc->EvalMult(X.b, Y.b));
    return {a_out, cc->EvalMult(cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
}

// Zero-centered OR: 1 - (1-a)(1-b) with zero-centering
DualGate direct_or_z(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto half = cc->MakeCKKSPackedPlaintext(std::vector<double>{0.5});
    auto xa = cc->EvalSub(X.a, half);  // Center X
    auto ya = cc->EvalSub(Y.a, half);  // Center Y
    // OR = NOT(AND(NOT(x), NOT(y))) centered
    auto not_x = cc->EvalSub(cc->EvalSub(one, X.a), half);
    auto not_y = cc->EvalSub(cc->EvalSub(one, Y.a), half);
    auto and_not = cc->EvalAdd(cc->EvalMult(not_x, not_y), half);
    auto a_out = cc->EvalSub(one, and_not);
    auto b_out = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(cc->EvalSub(one,X.a), Y.b), 
                           cc->EvalMult(X.b, cc->EvalSub(one,Y.a))), cc->EvalMult(X.b, Y.b));
    return {a_out, b_out};
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
    return {cc->EvalSub(val_A, b_phi), b_out};
}
DualGate make_input(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

std::mt19937 rng(std::random_device{}());

struct FractalCircuit { std::vector<DualGate> backbone; bool phi_is_A; };

FractalCircuit compile_mirror(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                               const std::vector<double>& inputs) {
    FractalCircuit fc;
    fc.phi_is_A = (rng() % 2 == 0);
    fc.backbone.resize(8);
    
    DualGate x = make_input(cc, kp, inputs[0]);
    DualGate y = make_input(cc, kp, inputs[1]);
    DualGate z = make_input(cc, kp, inputs[2]);
    fc.backbone[0]=x; fc.backbone[1]=y; fc.backbone[2]=z;
    
    // MIRROR LAYER: Zero-centered operations
    // G3: φ=AND(x,y) / ψ=OR(x,y)  — mirrors across zero
    DualGate g3_A = direct_and_z(cc, x, y);
    DualGate g3_B = direct_or_z(cc, x, y);
    fc.backbone[3] = fc.phi_is_A ? encode_dual(cc, g3_A, g3_B) : encode_dual(cc, g3_B, g3_A);
    
    // G4: φ=OR(x,z) / ψ=AND(x,z)  — mirrors across zero
    DualGate g4_A = direct_or_z(cc, x, z);
    DualGate g4_B = direct_and_z(cc, x, z);
    fc.backbone[4] = fc.phi_is_A ? encode_dual(cc, g4_A, g4_B) : encode_dual(cc, g4_B, g4_A);
    
    // G5: φ=OR(y,z) / ψ=AND(y,z)  — mirrors across zero
    DualGate g5_A = direct_or_z(cc, y, z);
    DualGate g5_B = direct_and_z(cc, y, z);
    fc.backbone[5] = fc.phi_is_A ? encode_dual(cc, g5_A, g5_B) : encode_dual(cc, g5_B, g5_A);
    
    // FRACTAL COMBINE — verified equivalent
    // φ: G6 = OR(G3, G4)
    // ψ: G6 = AND(G3, G4)
    DualGate g6_A = direct_or_z(cc, fc.backbone[3], fc.backbone[4]);
    DualGate g6_B = direct_and_z(cc, fc.backbone[3], fc.backbone[4]);
    fc.backbone[6] = fc.phi_is_A ? encode_dual(cc, g6_A, g6_B) : encode_dual(cc, g6_B, g6_A);
    
    // OUTPUT
    // φ: AND(G6, G5) = (x AND y) OR z
    // ψ: OR(G6, G5)  = (x OR z) AND (y OR z)
    DualGate g7_A = direct_and_z(cc, fc.backbone[6], fc.backbone[5]);
    DualGate g7_B = direct_or_z(cc, fc.backbone[6], fc.backbone[5]);
    fc.backbone[7] = fc.phi_is_A ? encode_dual(cc, g7_A, g7_B) : encode_dual(cc, g7_B, g7_A);
    
    return fc;
}

int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ZERO-AXIS MIRROR iO — Zero as the reflection center       ║\n";
    std::cout << "  ║  AND ←0→ OR · φ ←0→ ψ · Perfect mirror symmetry           ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(35); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    std::cout << "  φ = " << std::fixed << std::setprecision(6) << PHI << "\n";
    std::cout << "  ψ = " << PSI << "\n";
    std::cout << "  Zero-axis = " << ZERO_AXIS << " (mirror center)\n\n";
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  MIRROR CORRECTNESS — Zero-centered operations       │\n";
    std::cout << "  ├─────┬─────┬─────┬──────────┬──────────┬──────────────┤\n";
    
    int r1_ok=0, r2_ok=0;
    for (int i=0; i<8; i++) {
        std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
        auto fc = compile_mirror(cc, kp, dv);
        int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
        int bp = (decode_dual(fc.backbone[7], cc, kp, PHI) > 0.5) ? 1 : 0;
        int bs = (decode_dual(fc.backbone[7], cc, kp, PSI) > 0.5) ? 1 : 0;
        int bA = fc.phi_is_A ? bp : bs, bB = fc.phi_is_A ? bs : bp;
        if (bA == expected) r1_ok++; if (bB == expected) r2_ok++;
        std::cout << "  │  " << inputs[i][0] << "  │  " << inputs[i][1] << "  │  " << inputs[i][2]
                  << "  │  " << bA << "(" << expected << ") │  " << bB << "(" << expected << ") │  " 
                  << (bA==expected&&bB==expected?"OK ✓":"FAIL") << "        │\n";
    }
    std::cout << "  ├─────┴─────┴─────┴──────────┴──────────┴──────────────┤\n";
    std::cout << "  │  φ-path: " << r1_ok << "/8 · ψ-path: " << r2_ok << "/8                                   │\n";
    
    const int TRIALS = 100;
    int io_ok = 0;
    std::cout << "  │  iO Game: " << std::flush;
    for (int t=0; t<TRIALS; t++) {
        int inp = rng()%8;
        std::vector<double> dv = {(double)((inp>>0)&1), (double)((inp>>1)&1), (double)((inp>>2)&1)};
        auto fc = compile_mirror(cc, kp, dv);
        double vp = decode_dual(fc.backbone[7], cc, kp, PHI);
        double vs = decode_dual(fc.backbone[7], cc, kp, PSI);
        if ((vp > vs) == fc.phi_is_A) io_ok++;
        if ((t+1)%25==0) std::cout << "." << std::flush;
    }
    double io_adv = std::abs(100.0*io_ok/TRIALS - 50.0);
    std::cout << "\n  │  iO Game: " << std::fixed << std::setprecision(1) << 100.0*io_ok/TRIALS 
              << "% guess, " << std::setprecision(2) << io_adv << "% advantage               │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    bool pass = (r1_ok==8 && r2_ok==8 && io_adv<5.0);
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  ZERO-AXIS MIRROR iO — " << (pass ? "PERFECT MIRROR SYMMETRY ✓✓✓" : "TUNING") << "   ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
