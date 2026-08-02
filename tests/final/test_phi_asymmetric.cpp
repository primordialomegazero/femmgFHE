// ASYMMETRIC BIDIRECTIONAL: More forward, less backward
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
    return (ratio > 1.0) ? 1 : 0;
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
    std::cout << "\n  ASYMMETRIC BIDIRECTIONAL TEST\n\n";

    // Test many asymmetric combinations
    struct { int f; int b; } patterns[] = {
        {4,2}, {4,3}, {5,2}, {5,3}, {5,4}, {6,2}, {6,3}, {6,4},
        {3,1}, {4,1}, {5,1}, {3,2}, {2,1}
    };
    
    for (auto pat : patterns) {
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
            for (int i = 0; i < pat.f; i++) raw = mulY(cc, raw);
            for (int i = 0; i < pat.b; i++) raw = mulY_inv(cc, raw);
            return raw;
        };

        PE b0 = enc(cc, kp, 0), b1 = enc(cc, kp, 1);
        PE in[2] = {b0, b1};
        
        // Truth table
        int tt = 0;
        double vals[4];
        int idx = 0;
        for (int a = 0; a <= 1; a++)
            for (int b = 0; b <= 1; b++) {
                PE res = nand(in[a], in[b]);
                vals[idx] = r(cc, kp, res);
                if (bit(cc, kp, res) == (1-a*b)) tt++;
                idx++;
            }

        // Quick chain test (10 gates)
        PE state = b1, c1 = b1;
        int chain = 0;
        for (int g = 0; g < 10; g++) {
            try {
                state = nand(state, c1);
                if (bit(cc, kp, state) == ((g%2==0)?0:1)) chain++;
            } catch (...) { break; }
        }

        std::cout << "  F" << pat.f << "B" << pat.b 
                  << ": TT=" << tt << "/4"
                  << " vals=[" << std::fixed << std::setprecision(2) 
                  << vals[0] << "," << vals[1] << "," << vals[2] << "," << vals[3] << "]"
                  << " chain=" << chain << "/10\n";
    }

    // Best candidate detailed test
    std::cout << "\n  DETAILED: Best asymmetric pattern\n";
    // Pick the one with TT=4/4 and chain=10/10 from above
    // Let's use F5B3 as a candidate
    
    CCParams<CryptoContextCKKSRNS> pf;
    pf.SetMultiplicativeDepth(80);
    pf.SetScalingModSize(50);
    pf.SetBatchSize(1024);
    pf.SetRingDim(16384);
    pf.SetSecretKeyDist(UNIFORM_TERNARY);
    pf.SetSecurityLevel(HEStd_NotSet);
    
    auto ccf = GenCryptoContext(pf);
    ccf->Enable(PKE); ccf->Enable(KEYSWITCH); ccf->Enable(LEVELEDSHE); ccf->Enable(ADVANCEDSHE);
    auto kpf = ccf->KeyGen();
    ccf->EvalMultKeyGen(kpf.secretKey);

    // Test F5B3
    auto nand = [&](PE A, PE B) -> PE {
        auto aa = ccf->EvalMult(A.a, B.a);
        auto bb = ccf->EvalMult(A.b, B.b);
        PE raw = {ccf->EvalSub(bb, aa), bb};
        for (int i = 0; i < 5; i++) raw = mulY(ccf, raw);
        for (int i = 0; i < 3; i++) raw = mulY_inv(ccf, raw);
        return raw;
    };

    PE b0 = enc(ccf, kpf, 0), b1 = enc(ccf, kpf, 1);
    PE in[2] = {b0, b1};
    
    std::cout << "  Truth Table:\n";
    for (int a = 0; a <= 1; a++)
        for (int b = 0; b <= 1; b++) {
            PE res = nand(in[a], in[b]);
            std::cout << "  " << a << " " << b << " | " << std::fixed << std::setprecision(4) << r(ccf,kpf,res)
                      << " | " << bit(ccf,kpf,res) << " | " << (1-a*b) << "\n";
        }

    // Full chain
    std::cout << "\n  Full Chain (25 gates):\n";
    PE state = b1, c1 = b1;
    int chain = 0;
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);
    
    for (int g = 0; g < 25; g++) {
        if (g > 0 && g % 8 == 0) {
            state = rec(ccf, kpf, state);
            std::cout << "  RECYCLE\n";
        }
        state = nand(state, c1);
        int b = bit(ccf, kpf, state);
        int exp = (g % 2 == 0) ? 0 : 1;
        if (b == exp) chain++;
        std::cout << "  " << g << ": " << std::fixed << std::setprecision(2) << r(ccf,kpf,state)
                  << " bit=" << b << " exp=" << exp << (b==exp?" OK":" FAIL") << "\n";
    }
    gettimeofday(&t1, NULL);
    
    // fixed below
    // Fix: use t0 and t1
    double tms = (t1.tv_sec-t0.tv_sec)*1000.0 + (t1.tv_usec-t0.tv_usec)/1000.0;
    
    std::cout << "\n  Chain: " << chain << "/25 (" << std::fixed << std::setprecision(0) << tms << "ms)\n\n";

    return 0;
}
