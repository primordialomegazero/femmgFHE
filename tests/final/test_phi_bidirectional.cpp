// BIDIRECTIONAL NORMALIZE: Forward + Reverse Fibonacci
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
    return (ratio > 1.0) ? 1 : 0;  // φ=1.618>1, ψ=0.618<1
}
double ms(struct timeval s, struct timeval e) {
    return (e.tv_sec-s.tv_sec)*1000.0 + (e.tv_usec-s.tv_usec)/1000.0;
}
PE enc(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, int b) {
    double v = b ? 1.618033988749895 : 0.6180339887498949;
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{v})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
PE rec(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return enc(cc, kp, bit(cc, kp, s));
}

int main() {
    std::cout << "\n  BIDIRECTIONAL: Forward + Reverse Fibonacci\n\n";

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

    // Test different bidirectional patterns
    struct Pattern { int forward; int backward; };
    Pattern patterns[] = {{4,4}, {3,3}, {5,5}, {4,2}, {2,4}, {3,5}, {5,3}};
    
    for (auto pat : patterns) {
        auto nand = [&](PE A, PE B) -> PE {
            auto aa = cc->EvalMult(A.a, B.a);
            auto bb = cc->EvalMult(A.b, B.b);
            PE raw = {cc->EvalSub(bb, aa), bb};
            for (int i = 0; i < pat.forward; i++) raw = mulY(cc, raw);
            for (int i = 0; i < pat.backward; i++) raw = mulY_inv(cc, raw);
            return raw;
        };

        PE b0 = enc(cc, kp, 0), b1 = enc(cc, kp, 1);
        PE in[2] = {b0, b1};
        int tt = 0;
        for (int a = 0; a <= 1; a++)
            for (int b = 0; b <= 1; b++) {
                PE res = nand(in[a], in[b]);
                int dec = bit(cc, kp, res);
                if (dec == (1-a*b)) tt++;
            }

        // Check stability: do inputs preserve through the gate?
        double r0 = r(cc, kp, nand(b0, b1)); // should be φ (>1)
        double r1 = r(cc, kp, nand(b1, b1)); // should be ψ (<1)
        
        std::cout << "  F" << pat.forward << "B" << pat.backward 
                  << ": NAND=" << tt << "/4"
                  << " out(0,1)=" << std::fixed << std::setprecision(4) << r0
                  << " out(1,1)=" << r1 << "\n";
    }

    // Best pattern detailed test
    std::cout << "\n  DETAILED: F4B4 (balanced bidirectional)\n";
    
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
    
    std::cout << "  NAND Truth Table:\n";
    for (int a = 0; a <= 1; a++)
        for (int b = 0; b <= 1; b++) {
            PE res = nand(in[a], in[b]);
            std::cout << "  " << a << " " << b << " | " 
                      << std::fixed << std::setprecision(6) << r(cc, kp, res)
                      << " | " << bit(cc, kp, res) << " | " << (1-a*b) << "\n";
        }

    // Chain
    std::cout << "\n  Chain (25 gates, recycle every 8):\n";
    PE state = b1, c1 = b1;
    int chain = 0;
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    for (int g = 0; g < 25; g++) {
        if (g > 0 && g % 8 == 0) {
            state = rec(cc, kp, state);
            std::cout << "  ---- RECYCLE ----\n";
        }
        state = nand(state, c1);
        int b = bit(cc, kp, state);
        int exp = (g % 2 == 0) ? 0 : 1;
        if (b == exp) chain++;
        std::cout << "  " << g << ": ratio=" << std::fixed << std::setprecision(4) << r(cc,kp,state)
                  << " bit=" << b << " exp=" << exp << (b==exp?" OK":" FAIL") << "\n";
    }
    gettimeofday(&t1, NULL);
    std::cout << "\n  Chain: " << chain << "/25 (" << std::fixed << std::setprecision(0) << ms(t0,t1) << "ms)\n\n";

    return 0;
}
