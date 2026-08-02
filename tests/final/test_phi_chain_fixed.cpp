// CHAIN FIXED: Correct threshold + optimal mulY steps
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
    return (r(cc, kp, s) > 1.0) ? 1 : 0;
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
    std::cout << "\n  CHAIN FIXED: Threshold=1.0\n\n";
    srand(time(0));

    for (int mulY_steps : {4, 5, 6}) {
        CCParams<CryptoContextCKKSRNS> p;
        p.SetMultiplicativeDepth(80);
        p.SetScalingModSize(45);
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
            for (int i = 0; i < mulY_steps; i++) raw = mulY(cc, raw);
            return raw;
        };

        PE b0 = enc(cc, kp, 0), b1 = enc(cc, kp, 1);
        PE in[2] = {b0, b1};
        int tt = 0;
        for (int a = 0; a <= 1; a++)
            for (int b = 0; b <= 1; b++)
                if (bit(cc, kp, nand(in[a], in[b])) == (1-a*b)) tt++;

        PE state = b1, c1 = b1;
        int chain = 0;
        for (int g = 0; g < 25; g++) {
            if (g > 0 && g % 8 == 0) state = rec(cc, kp, state);
            try {
                state = nand(state, c1);
                if (bit(cc, kp, state) == ((g%2==0)?0:1)) chain++;
            } catch (...) { break; }
        }
        std::cout << "  mulY=" << mulY_steps << ": NAND=" << tt << "/4 Chain=" << chain << "/25\n";
    }

    // Detailed run with best mulY=5
    std::cout << "\n  DETAILED RUN (mulY=5):\n  Gate | Level | Ratio      | Bit | Exp\n  ---------------------------------------\n";
    
    CCParams<CryptoContextCKKSRNS> pf;
    pf.SetMultiplicativeDepth(80);
    pf.SetScalingModSize(45);
    pf.SetBatchSize(1024);
    pf.SetRingDim(16384);
    pf.SetSecretKeyDist(UNIFORM_TERNARY);
    pf.SetSecurityLevel(HEStd_NotSet);
    
    auto ccf = GenCryptoContext(pf);
    ccf->Enable(PKE); ccf->Enable(KEYSWITCH); ccf->Enable(LEVELEDSHE); ccf->Enable(ADVANCEDSHE);
    auto kpf = ccf->KeyGen();
    ccf->EvalMultKeyGen(kpf.secretKey);

    auto nandf = [&](PE A, PE B) -> PE {
        auto aa = ccf->EvalMult(A.a, B.a);
        auto bb = ccf->EvalMult(A.b, B.b);
        PE raw = {ccf->EvalSub(bb, aa), bb};
        for (int i = 0; i < 5; i++) raw = mulY(ccf, raw);
        return raw;
    };

    PE sf = enc(ccf, kpf, 1), c1f = enc(ccf, kpf, 1);
    int ok = 0;
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    for (int g = 0; g < 25; g++) {
        if (g > 0 && g % 8 == 0) sf = rec(ccf, kpf, sf);
        sf = nandf(sf, c1f);
        
        int lvl = sf.a->GetLevel();
        double ratio = r(ccf, kpf, sf);
        int b = bit(ccf, kpf, sf);
        int exp = (g % 2 == 0) ? 0 : 1;
        if (b == exp) ok++;
        
        std::cout << "  " << std::setw(4) << g 
                  << " | " << std::setw(5) << lvl
                  << " | " << std::fixed << std::setprecision(4) << std::setw(10) << ratio
                  << " | " << b << "   | " << exp
                  << (b == exp ? "" : "  FAIL") << "\n";
    }
    gettimeofday(&t1, NULL);

    std::cout << "\n  Result: " << ok << "/25 (" << std::fixed << std::setprecision(0) << ms(t0,t1) << "ms)\n\n";
    return 0;
}
