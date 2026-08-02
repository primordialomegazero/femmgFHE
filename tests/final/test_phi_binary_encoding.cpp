// BINARY ENCODING: {0, 1} not {ψ, φ}
// This is the ORIGINAL working approach
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
    return (ratio > 0.5) ? 1 : 0;
}
PE enc(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v = b ? 1.0 : 0.0;  // {0, 1} encoding!
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE rec(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return enc(cc, kp, bit(cc, kp, s));
}

int main() {
    std::cout << "\n  BINARY ENCODING: {0, 1} + Fibonacci Normalize\n\n";

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

    auto nand = [&](PE A, PE B) -> PE {
        auto aa = cc->EvalMult(A.a, B.a);
        auto bb = cc->EvalMult(A.b, B.b);
        PE raw = {cc->EvalSub(bb, aa), bb};
        for (int i = 0; i < 8; i++) raw = mulY(cc, raw);
        return raw;
    };

    PE b0 = enc(cc, kp, 0), b1 = enc(cc, kp, 1);
    PE in[2] = {b0, b1};

    // Truth table
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

    // Chain
    std::cout << "  Chain (25 gates):\n";
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
        if (g < 5 || g >= 20)
            std::cout << "  " << g << ": " << std::fixed << std::setprecision(4) << r(cc,kp,state)
                      << " bit=" << b << " exp=" << exp << (b==exp?" OK":" FAIL") << "\n";
    }
    gettimeofday(&t1, NULL);
    double tms = (t1.tv_sec-t0.tv_sec)*1000.0 + (t1.tv_usec-t0.tv_usec)/1000.0;
    std::cout << "  ...\n  Chain: " << chain << "/25 (" << std::fixed << std::setprecision(0) << tms << "ms)\n\n";

    return 0;
}
