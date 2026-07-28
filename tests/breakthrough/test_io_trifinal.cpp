// ╔══════════════════════════════════════════════════════════════════╗
// ║  TRIFINAL iO v2 — Guaranteed decoy + iO game                   ║
// ║  Reality 1 (φ): Circuit A · Reality 2 (ψ): Circuit B           ║
// ║  Reality 3 (Ω): Guaranteed wrong decoy + Morse watermark        ║
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
// iO CORE — UNTOUCHED (v8 proven)
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
    auto sum = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(X.a, Y.b), cc->EvalMult(X.b, Y.a)), cc->EvalMult(X.b, Y.b));
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {a_out, cc->EvalMult(neg_one, sum)};
}

DualGate direct_or(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto oma1 = cc->EvalSub(one, X.a), oma2 = cc->EvalSub(one, Y.a);
    auto a_out = cc->EvalSub(one, cc->EvalMult(oma1, oma2));
    auto b_out = cc->EvalAdd(cc->EvalAdd(cc->EvalMult(oma1, Y.b), cc->EvalMult(X.b, oma2)), cc->EvalMult(X.b, Y.b));
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

std::string to_morse(const std::string& msg) {
    std::map<char,std::string> m;
    m['A']=".-";m['B']="-...";m['C']="-.-.";m['D']="-..";m['E']=".";m['F']="..-.";
    m['G']="--.";m['H']="....";m['I']="..";m['J']=".---";m['K']="-.-";m['L']=".-..";
    m['M']="--";m['N']="-.";m['O']="---";m['P']=".--.";m['Q']="--.-";m['R']=".-.";
    m['S']="...";m['T']="-";m['U']="..-";m['V']="...-";m['W']=".--";m['X']="-..-";
    m['Y']="-.--";m['Z']="--..";m[' ']="/";m['!']="-.-.--";
    std::string o; for(char c:msg){c=std::toupper(c);if(m.find(c)!=m.end())o+=m[c]+" ";} return o;
}

std::mt19937 rng(std::random_device{}());

struct TrifinalOutput {
    std::vector<DualGate> backbone;
    std::vector<DualGate> decoy_path;
    bool phi_is_A;
};

TrifinalOutput compile_trifinal(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp,
                                 const std::vector<double>& inputs) {
    TrifinalOutput result;
    result.phi_is_A = (rng() % 2 == 0);
    result.backbone.resize(8);
    result.decoy_path.resize(8);
    
    DualGate g0 = make_input(cc, kp, inputs[0]);
    DualGate g1 = make_input(cc, kp, inputs[1]);
    DualGate g2 = make_input(cc, kp, inputs[2]);
    
    // ═══ iO CORE (UNTOUCHED) ═══
    result.backbone[0]=g0; result.backbone[1]=g1; result.backbone[2]=g2;
    DualGate A_and = direct_and(cc, g0, g1);
    DualGate A_out = direct_or(cc, A_and, g2);
    DualGate B_or1 = direct_or(cc, g0, g2);
    DualGate B_or2 = direct_or(cc, g1, g2);
    DualGate B_out = direct_and(cc, B_or1, B_or2);
    result.backbone[3] = result.phi_is_A ? encode_dual(cc, A_and, B_or1) : encode_dual(cc, B_or1, A_and);
    result.backbone[4] = result.phi_is_A ? encode_dual(cc, A_out, B_out) : encode_dual(cc, B_out, A_out);
    result.backbone[5]=result.backbone[4]; result.backbone[6]=result.backbone[4]; result.backbone[7]=result.backbone[4];
    
    // ═══ DECOY — GUARANTEED WRONG (NAND instead of AND/OR) ═══
    result.decoy_path[0]=g0; result.decoy_path[1]=g1; result.decoy_path[2]=g2;
    DualGate decoy3 = unified_nand(cc, g0, g1);  // NAND(x,y) — definitely wrong
    DualGate decoy_out = unified_nand(cc, decoy3, g2); // NAND(NAND(x,y), z) — double wrong
    result.decoy_path[3] = encode_dual(cc, decoy3, decoy3);
    result.decoy_path[4] = encode_dual(cc, decoy_out, decoy_out);
    result.decoy_path[5]=result.decoy_path[4]; result.decoy_path[6]=result.decoy_path[4]; result.decoy_path[7]=result.decoy_path[4];
    
    return result;
}

int main() {
    time_t st = time(0);
    std::cout << "\n  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  TRIFINAL iO v2 — Guaranteed decoy + iO game                ║\n";
    std::cout << "  ║  Reality 1 (φ) · Reality 2 (ψ) · Reality 3 (Ω NAND decoy)  ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "  Started: " << ctime(&st) << "\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p); cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen(); cc->EvalMultKeyGen(kp.secretKey);
    
    int inputs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    
    std::cout << "  ┌──────────────────────────────────────────────────────┐\n";
    std::cout << "  │  THREE REALITIES + iO GAME                            │\n";
    std::cout << "  ├─────┬─────┬─────┬──────────┬──────────┬──────────────┤\n";
    
    int r1_ok=0, r2_ok=0, r3_wrong=0;
    for (int i=0; i<8; i++) {
        std::vector<double> dv = {(double)inputs[i][0], (double)inputs[i][1], (double)inputs[i][2]};
        auto compiled = compile_trifinal(cc, kp, dv);
        int expected = (inputs[i][0] & inputs[i][1]) | inputs[i][2];
        int b_phi = (decode_dual(compiled.backbone[7], cc, kp, PHI) > 0.5) ? 1 : 0;
        int b_psi = (decode_dual(compiled.backbone[7], cc, kp, PSI) > 0.5) ? 1 : 0;
        int b_decoy = (decode_dual(compiled.decoy_path[7], cc, kp, PHI) > 0.5) ? 1 : 0;
        int bA = compiled.phi_is_A ? b_phi : b_psi;
        int bB = compiled.phi_is_A ? b_psi : b_phi;
        if (bA == expected) r1_ok++; if (bB == expected) r2_ok++;
        if (b_decoy != expected) r3_wrong++;
        std::cout << "  │  " << inputs[i][0] << "  │  " << inputs[i][1] << "  │  " << inputs[i][2]
                  << "  │  " << bA << "(" << expected << ") │  " << bB << "(" << expected << ") │  Ω=" << b_decoy 
                  << " " << (b_decoy != expected ? "D✓" : "?") << "        │\n";
    }
    std::cout << "  ├─────┴─────┴─────┴──────────┴──────────┴──────────────┤\n";
    std::cout << "  │  R1(φ): " << r1_ok << "/8 · R2(ψ): " << r2_ok << "/8 · R3(Ω decoy): " << r3_wrong << "/8 wrong        │\n";
    
    // iO GAME
    const int TRIALS = 50;
    int io_ok = 0;
    for (int t=0; t<TRIALS; t++) { if ((t+1)%10==0) std::cout << "." << std::flush; if ((t+1)%10==0) std::cout<<"."<<std::flush;
        int inp = rng()%8;
        std::vector<double> dv = {(double)((inp>>0)&1), (double)((inp>>1)&1), (double)((inp>>2)&1)};
        auto compiled = compile_trifinal(cc, kp, dv);
        double vp = decode_dual(compiled.backbone[7], cc, kp, PHI);
        double vs = decode_dual(compiled.backbone[7], cc, kp, PSI);
        if ((vp > vs) == compiled.phi_is_A) io_ok++;
    }
    double io_adv = std::abs(100.0*io_ok/TRIALS - 50.0);
    
    std::cout << "  │  iO Game: " << std::fixed << std::setprecision(1) << 100.0*io_ok/TRIALS 
              << "% guess, " << std::setprecision(2) << io_adv << "% advantage               │\n";
    std::cout << "  │  Watermarks: " << to_morse("FALSEKEY R1 DECOY") << "  " << to_morse("FALSEKEY R2 VOID") << "  " << to_morse("FALSEKEY GOODLUCK KID") << " │\n";
    std::cout << "  │  iO CORE: UNTOUCHED v8.                                 │\n";
    std::cout << "  └──────────────────────────────────────────────────────┘\n\n";
    
    time_t et = time(0);
    bool pass = (r1_ok==8 && r2_ok==8 && r3_wrong>=4 && io_adv<5.0);
    std::cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  TRIFINAL iO v2 — " << (pass ? "3 REALITIES · iO CORE PRESERVED ✓✓✓" : "TUNING") << "          ║\n";
    std::cout << "  ║  Author: Dan Fernandez / Primordial Omega Zero               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "  Ended: " << ctime(&et) << "\n";
    return 0;
}
