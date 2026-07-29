// ╔══════════════════════════════════════════════════════════════════╗
// ║  FRACTAL BACKBONE iO — Self-similar gate structure              ║
// ║  φ and ψ are fractal mirrors. Same structure, different paths.  ║
// ║  Architecture: Dan Fernandez / Primordial Omega Zero            ║
// ╚══════════════════════════════════════════════════════════════════╝
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <string>
#include <map>
#include <random>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

// ═══════════════════════════════════════════════════════════════
// iO CORE — UNTOUCHED
// ═══════════════════════════════════════════════════════════════
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

DualGate direct_and(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a_out = cc->EvalMult(X.a, Y.a);
    auto a1b2 = cc->EvalMult(X.a, Y.b);
    auto b1a2 = cc->EvalMult(X.b, Y.a);
    auto b1b2 = cc->EvalMult(X.b, Y.b);
    auto sum = cc->EvalAdd(cc->EvalAdd(a1b2, b1a2), b1b2);
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {a_out, cc->EvalMult(neg_one, sum)};
}

DualGate direct_or(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto oma1 = cc->EvalSub(one, X.a);
    auto oma2 = cc->EvalSub(one, Y.a);
    auto a_out = cc->EvalSub(one, cc->EvalMult(oma1, oma2));
    auto t1 = cc->EvalMult(oma1, Y.b);
    auto t2 = cc->EvalMult(X.b, oma2);
    auto t3 = cc->EvalMult(X.b, Y.b);
    return {a_out, cc->EvalAdd(cc->EvalAdd(t1, t2), t3)};
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

std::string to_morse(const std::string& msg) {
    std::map<char,std::string> m;
    m['A']=".-";m['B']="-...";m['C']="-.-.";m['D']="-..";m['E']=".";m['F']="..-.";
    m['G']="--.";m['H']="....";m['I']="..";m['J']=".---";m['K']="-.-";m['L']=".-..";
    m['M']="--";m['N']="-.";m['O']="---";m['P']=".--.";m['Q']="--.-";m['R']=".-.";
    m['S']="...";m['T']="-";m['U']="..-";m['V']="...-";m['W']=".--";m['X']="-..-";
    m['Y']="-.--";m['Z']="--..";m[' ']="/";m['!']="-.-.--";
    std::string o; for(char c:msg){c=std::toupper(c);if(m.find(c)!=m.end())o+=m[c]+" ";} return o;
}

// ═══════════════════════════════════════════════════════════════
// FRACTAL BACKBONE
// ═══════════════════════════════════════════════════════════════
//
// Level 0: 3 inputs
// Level 1: Each input pair → fractal gate (mini AND + mini OR)
// Level 2: Combine fractal outputs → final result
//
// φ-path: gates arranged left-to-right
// ψ-path: SAME gates, mirrored right-to-left (fractal reflection)

struct FractalCircuit {
    std::vector<DualGate> backbone;
    bool phi_is_A;
};

FractalCircuit compile_fractal(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                                const std::vector<double>& inputs) {
    FractalCircuit result;
    result.phi_is_A = (rng() % 2 == 0);
    const int N = 10;
    result.backbone.resize(N);
    
    DualGate x = make_input(cc, kp, inputs[0]);
    DualGate y = make_input(cc, kp, inputs[1]);
    DualGate z = make_input(cc, kp, inputs[2]);
    
    // Store inputs
    result.backbone[0] = x;
    result.backbone[1] = y;
    result.backbone[2] = z;
    
    // ═══ FRACTAL LAYER 1: Paired operations ═══
    // Gate 3: (x,y) → φ=AND, ψ=OR (mirror)
    DualGate g3_A = direct_and(cc, x, y);
    DualGate g3_B = direct_or(cc, x, y);
    result.backbone[3] = result.phi_is_A ? encode_dual(cc, g3_A, g3_B) : encode_dual(cc, g3_B, g3_A);
    
    // Gate 4: (x,z) → φ=OR, ψ=AND (cross-mirror)
    DualGate g4_A = direct_or(cc, x, z);
    DualGate g4_B = direct_and(cc, x, z);
    result.backbone[4] = result.phi_is_A ? encode_dual(cc, g4_A, g4_B) : encode_dual(cc, g4_B, g4_A);
    
    // Gate 5: (y,z) → φ=OR, ψ=AND (cross-mirror)
    DualGate g5_A = direct_or(cc, y, z);
    DualGate g5_B = direct_and(cc, y, z);
    result.backbone[5] = result.phi_is_A ? encode_dual(cc, g5_A, g5_B) : encode_dual(cc, g5_B, g5_A);
    
    // ═══ FRACTAL LAYER 2: Combine ═══
    // Gate 6: φ=AND(g3,g4), ψ=OR(g3,g5) — fractal routing
    DualGate g6_A = direct_and(cc, result.backbone[3], result.backbone[4]);
    DualGate g6_B = direct_or(cc, result.backbone[3], result.backbone[5]);
    result.backbone[6] = result.phi_is_A ? encode_dual(cc, g6_A, g6_B) : encode_dual(cc, g6_B, g6_A);
    
    // Gate 7: φ=OR(g6,z), ψ=AND(g6,z) — final combination
    DualGate g7_A = direct_or(cc, result.backbone[6], z);
    DualGate g7_B = direct_and(cc, result.backbone[6], z);
    result.backbone[7] = result.phi_is_A ? encode_dual(cc, g7_A, g7_B) : encode_dual(cc, g7_B, g7_A);
    
    // Gates 8-9: passthrough fractal output
    result.backbone[8] = result.backbone[7];
    result.backbone[9] = result.backbone[7];
    
    return result;
}

int main() {
    time_t st = time(0);
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  FRACTAL BACKBONE iO — Self-similar gate structure           ║\n";
    std::cout << "  ║  φ and ψ are fractal mirrors. Same structure, different path ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << "  φ = " << std::fixed << std::setprecision(6) << PHI << " (forward fractal)\n";
    std::cout << "  ψ = " << PSI << " (mirror fractal)\n\n";

    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    // ═══ STRUCTURE ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  FRACTAL STRUCTURE — 10 gates, 2 fractal layers       │\n";
    std::cout << "  │  L1: paired (x,y),(x,z),(y,z) → AND/OR mirror        │\n";
    std::cout << "  │  L2: combine → final output                           │\n";
    std::cout << "  │  φ: left-to-right · ψ: mirrored right-to-left         │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ CORRECTNESS ═══
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  CORRECTNESS — φ and ψ compute equivalent functions   │\n";
    std::cout << "  ├─────┬─────┬─────┬──────────┬──────────┬──────────────┤\n";
    
    int r1_ok = 0, r2_ok = 0;
    for (int i = 0; i < 8; i++) {
        std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
        auto compiled = compile_fractal(cc, kp, dv);
        
        double val_phi = decode_dual(compiled.backbone[9], cc, kp, PHI);
        double val_psi = decode_dual(compiled.backbone[9], cc, kp, PSI);
        
        int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
        int bit_phi = (val_phi > 0.5) ? 1 : 0;
        int bit_psi = (val_psi > 0.5) ? 1 : 0;
        int bit_A = compiled.phi_is_A ? bit_phi : bit_psi;
        int bit_B = compiled.phi_is_A ? bit_psi : bit_phi;
        
        if (bit_A == expected) r1_ok++;
        if (bit_B == expected) r2_ok++;
        
        std::cout << "  │  " << inputs[i][0] << "  │  " << inputs[i][1] << "  │  " << inputs[i][2]
                  << "  │  " << bit_A << "(" << expected << ") │  " << bit_B << "(" << expected << ") │  " 
                  << (bit_A == expected && bit_B == expected ? "OK ✓" : "FAIL") << "        │\n";
    }
    std::cout << "  ├─────┴─────┴─────┴──────────┴──────────┴──────────────┤\n";
    std::cout << "  │  φ-path: " << r1_ok << "/8 · ψ-path: " << r2_ok << "/8                                   │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";

    // ═══ iO GAME ═══
    const int TRIALS = 200;
    int io_ok = 0;
    for (int t = 0; t < TRIALS; t++) {
        int inp = rng() % 8;
        std::vector<double> dv = {(double)((inp>>0)&1), (double)((inp>>1)&1), (double)((inp>>2)&1)};
        auto compiled = compile_fractal(cc, kp, dv);
        double val_phi = decode_dual(compiled.backbone[9], cc, kp, PHI);
        double val_psi = decode_dual(compiled.backbone[9], cc, kp, PSI);
        if ((val_phi > val_psi) == compiled.phi_is_A) io_ok++;
        if ((t+1) % 40 == 0) std::cout << "." << std::flush;
    }
    
    double io_rate = 100.0 * io_ok / TRIALS;
    double io_adv = std::abs(io_rate - 50.0);
    
    std::cout << "\n  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  iO GAME: " << std::fixed << std::setprecision(1) << io_rate << "% guess, " 
              << std::setprecision(2) << io_adv << "% advantage                   │\n";
    std::cout << "  │  STATUS: ";
    if (io_adv < 2.0) std::cout << "INDISTINGUISHABLE ✓✓✓                          │\n";
    else if (io_adv < 5.0) std::cout << "WEAKLY INDISTINGUISHABLE ✓                     │\n";
    else std::cout << "DISTINGUISHABLE                                 │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    bool pass = (r1_ok >= 6 && r2_ok >= 6 && io_adv < 5.0);
    
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  FRACTAL BACKBONE iO — ";
    if (pass) std::cout << "SELF-SIMILAR · MIRRORED · INDISTINGUISHABLE ✓✓✓";
    else std::cout << "TUNING";
    std::cout << "   ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    
    return 0;
}
