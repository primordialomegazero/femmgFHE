// HYBRID ENCODING: Fresh={0,1}, Chain=F4B4 Bidirectional
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE mulY(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {x.b, cc->EvalAdd(x.a, x.b)};
}
PE mulY_inv(CryptoContext<DCRTPoly>& cc, const PE& x) {
    return {cc->EvalSub(x.b, x.a), x.a};
}
double d(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}
double r(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = d(cc, kp, s.a), b = d(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}
int bit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double ratio = r(cc, kp, s);
    // For chained values: large magnitude, alternating sign
    // threshold at 0.5 works for both {0,1} and large values
    return (ratio > 0.5) ? 1 : 0;
}
PE enc(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v = b ? 1.0 : 0.0;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE rec(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return enc(cc, kp, bit(cc, kp, s));
}

int main() {
    std::cout << "\n  HYBRID: Fresh={0,1} + F4B4 Chain\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(80);
    p.SetScalingModSize(50);
    p.SetBatchSize(1024);
    p.SetRingDim(16384);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    // F4B4 Bidirectional NAND
    auto nand = [&](PE A, PE B) -> PE {
        auto aa = cc->EvalMult(A.a, B.a);
        auto bb = cc->EvalMult(A.b, B.b);
        PE raw = {cc->EvalSub(bb, aa), bb};
        for (int i = 0; i < 4; i++) raw = mulY(cc, raw);
        for (int i = 0; i < 4; i++) raw = mulY_inv(cc, raw);
        return raw;
    };

    PE b0 = enc(cc, kp, 0), b1 = enc(cc, kp, 1);
    PE in[2] = {b0, b1};

    // Truth Table
    std::cout << "  NAND Truth Table:\n  A B | Ratio      | Bit | Exp\n  ---------------------------------\n";
    int tt = 0;
    for (int a = 0; a <= 1; a++)
        for (int b = 0; b <= 1; b++) {
            PE res = nand(in[a], in[b]);
            int dec = bit(cc, kp, res);
            int exp = 1 - a*b;
            if (dec == exp) tt++;
            std::cout << "  " << a << " " << b << " | " << std::fixed << std::setprecision(6) << std::setw(10) << r(cc,kp,res)
                      << " | " << dec << "   | " << exp << (dec==exp?" OK":" FAIL") << "\n";
        }
    std::cout << "  Truth table: " << tt << "/4\n\n";

    // Chain Test
    std::cout << "  Chain (25 gates, recycle every 8):\n";
    PE state = b1, c1 = b1;
    int chain = 0;
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);
    
    for (int g = 0; g < 25; g++) {
        if (g > 0 && g % 8 == 0) state = rec(cc, kp, state);
        state = nand(state, c1);
        int b = bit(cc, kp, state);
        int exp = (g % 2 == 0) ? 0 : 1;
        if (b == exp) chain++;
        std::cout << "  " << g << ": " << std::fixed << std::setprecision(2) << r(cc,kp,state)
                  << " bit=" << b << " exp=" << exp << (b==exp?" OK":" FAIL") << "\n";
    }
    gettimeofday(&t1, NULL);
    double tms = (t1.tv_sec-t0.tv_sec)*1000.0 + (t1.tv_usec-t0.tv_usec)/1000.0;
    std::cout << "  Chain: " << chain << "/25 (" << std::fixed << std::setprecision(0) << tms << "ms)\n\n";

    // Full Adder Test
    std::cout << "  Full Adder (with recycle):\n";
    auto fa = [&](PE A, PE B, PE Cin) -> std::pair<PE, PE> {
        PE X1 = rec(cc, kp, nand(A, B));
        PE X2 = rec(cc, kp, nand(A, X1));
        PE X3 = rec(cc, kp, nand(B, X1));
        PE X4 = rec(cc, kp, nand(X2, X3));
        PE X5 = rec(cc, kp, nand(X4, Cin));
        PE X6 = rec(cc, kp, nand(X4, X5));
        PE X7 = rec(cc, kp, nand(X5, Cin));
        return {nand(X6, X7), nand(X1, X5)};
    };
    
    int sum_ok=0, cout_ok=0;
    int cs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
    for (int i=0;i<8;i++) {
        PE A=enc(cc,kp,cs[i][0]), B=enc(cc,kp,cs[i][1]), Cin=enc(cc,kp,cs[i][2]);
        auto [S,C] = fa(A,B,Cin);
        if (bit(cc,kp,S)==(cs[i][0]+cs[i][1]+cs[i][2])%2) sum_ok++;
        if (bit(cc,kp,C)==(cs[i][0]+cs[i][1]+cs[i][2])/2) cout_ok++;
    }
    std::cout << "  SUM: " << sum_ok << "/8 COUT: " << cout_ok << "/8\n\n";

    // FINAL
    std::cout << "  ╔═══════════════════════════════════╗\n";
    std::cout << "  ║  HYBRID RESULTS                    ║\n";
    std::cout << "  ║  NAND: " << tt << "/4  Chain: " << chain << "/25           ║\n";
    std::cout << "  ║  FA: SUM=" << sum_ok << "/8 COUT=" << cout_ok << "/8              ║\n";
    std::cout << "  ╚═══════════════════════════════════╝\n\n";

    return 0;
}
