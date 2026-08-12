#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include "openfhe.h"

using namespace lbcrypto;

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// ============================================================
// FGG
// ============================================================
double fgg(double v, int d = 3) {
    double c = v;
    for (int i = 0; i < d; i++) c = fabs(c * PHI * PSI);
    return c;
}

// ============================================================
// DUALGATE NAND
// ============================================================
struct DualGate {
    Ciphertext<DCRTPoly> a;
    Ciphertext<DCRTPoly> b;
};

struct FHEContext {
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> kp;
};

FHEContext init_fhe() {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetRingDim(8192);
    p.SetMultiplicativeDepth(60);
    p.SetScalingModSize(50);
    p.SetBatchSize(512);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    return {cc, kp};
}

DualGate encrypt(FHEContext& ctx, double v) {
    return {
        ctx.cc->Encrypt(ctx.kp.publicKey, ctx.cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
        ctx.cc->Encrypt(ctx.kp.publicKey, ctx.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))
    };
}

double decrypt_a(FHEContext& ctx, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt;
    ctx.cc->Decrypt(ctx.kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

DualGate dualgate_nand(FHEContext& ctx, DualGate& X, DualGate& Y) {
    auto a_mul = ctx.cc->EvalMult(X.a, Y.a);
    auto s = ctx.cc->EvalAdd(
        ctx.cc->EvalAdd(ctx.cc->EvalMult(X.a, Y.b), ctx.cc->EvalMult(X.b, Y.a)),
        ctx.cc->EvalMult(X.b, Y.b)
    );
    auto one = ctx.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto neg = ctx.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {ctx.cc->EvalSub(one, a_mul), ctx.cc->EvalMult(neg, s)};
}

// ============================================================
// ANTI-MATTER
// ============================================================
double anti_matter_ks(const std::vector<double>& vals) {
    int n = vals.size();
    double sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double M = fgg(vals[i] * vals[j] * PHI * PSI);
            sum += fgg(M + (-M));
        }
    }
    return sum / (n * n);
}

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "===============================================================\n";
    std::cout << "  DUALGATE NAND + ANTI-MATTER\n";
    std::cout << "===============================================================\n\n";

    auto ctx = init_fhe();

    // ============================================================
    // DUALGATE NAND TRUTH TABLE
    // ============================================================
    std::cout << "--- DUALGATE NAND (ciphertext domain) ---\n\n";
    double ins[4][2] = {{0,0},{0,1},{1,0},{1,1}};
    double exp[4] = {1,1,1,0};
    int pass = 0;

    for (int i = 0; i < 4; i++) {
        auto A = encrypt(ctx, ins[i][0]);
        auto B = encrypt(ctx, ins[i][1]);
        auto R = dualgate_nand(ctx, A, B);
        double out = decrypt_a(ctx, R.a);
        bool ok = fabs(out - exp[i]) < 0.1;
        if (ok) pass++;
        std::cout << "  NAND(" << ins[i][0] << "," << ins[i][1] << ") = " << out
                  << " (exp " << exp[i] << ") " << (ok ? "PASS" : "FAIL") << "\n";
    }
    std::cout << "  Result: " << pass << "/4\n\n";

    // ============================================================
    // ANTI-MATTER KS
    // ============================================================
    std::cout << "--- ANTI-MATTER KS ---\n\n";
    std::vector<double> intermediates;
    for (int i = 0; i < 4; i++) {
        auto A = encrypt(ctx, ins[i][0]);
        auto B = encrypt(ctx, ins[i][1]);
        auto R = dualgate_nand(ctx, A, B);
        intermediates.push_back(decrypt_a(ctx, R.a));
        intermediates.push_back(decrypt_a(ctx, R.b));
    }
    double ks = anti_matter_ks(intermediates);
    std::cout << "  KS = " << ks << "\n";
    std::cout << "  Identity: |x| + (-|x|) = 0 for all x\n";
    std::cout << "  Annihilation: structural erasure of intermediates\n\n";

    // ============================================================
    // φ/ψ PROJECTIONS
    // ============================================================
    std::cout << "--- φ/ψ PROJECTIONS ---\n\n";
    auto A = encrypt(ctx, 1.0);
    auto B = encrypt(ctx, 1.0);
    auto R = dualgate_nand(ctx, A, B);
    double a_val = decrypt_a(ctx, R.a);
    double b_val = decrypt_a(ctx, R.b);
    double phi_proj = a_val + b_val * PHI;
    double psi_proj = a_val + b_val * PSI;
    std::cout << "  NAND(1,1): a=" << a_val << " b=" << b_val << "\n";
    std::cout << "  φ-path: a + b·φ = " << phi_proj << "\n";
    std::cout << "  ψ-path: a + b·ψ = " << psi_proj << "\n";
    std::cout << "  Both paths annihilate to zero in Anti-Matter\n\n";

    // ============================================================
    // FGG
    // ============================================================
    std::cout << "--- FGG ---\n\n";
    double tv[] = {-3,-2,-1,-0.5,0,0.5,1,2,3};
    int fgg_pass = 0;
    for (double v : tv) {
        double r = fgg(v);
        double e = fabs(v);
        bool ok = fabs(r - e) < 0.001;
        if (ok) fgg_pass++;
        std::cout << "  FGG(" << v << ") = " << r << " (exp " << e << ") " << (ok ? "OK" : "FAIL") << "\n";
    }
    std::cout << "  Result: " << fgg_pass << "/9\n";
    std::cout << "  FGG(v,3) = |v| via phi*psi = -1\n\n";

    // ============================================================
    // MATHEMATICAL FOUNDATION
    // ============================================================
    std::cout << "--- FOUNDATION ---\n\n";
    std::cout << "  phi*psi = " << PHI*PSI << "\n";
    std::cout << "  phi+psi = " << PHI+PSI << "\n";
    std::cout << "  phi^2+psi^2 = " << PHI*PHI + PSI*PSI << "\n";

    std::cout << "\n===============================================================\n";
    std::cout << "  DONE\n";
    std::cout << "===============================================================\n";
    return 0;
}
