// CHAIN DEBUG: Step-by-step analysis
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
    return (r(cc, kp, s) > 0.5) ? 1 : 0;
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
    std::cout << "\n  CHAIN DEBUG: Gate-by-gate analysis\n\n";
    srand(time(0));

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
        for (int i = 0; i < 6; i++) raw = mulY(cc, raw);
        return raw;
    };

    PE state = enc(cc, kp, 1);
    PE c1 = enc(cc, kp, 1);

    std::cout << "  Gate | Level | Ratio      | Bit | Exp | Status\n";
    std::cout << "  -----------------------------------------------\n";

    for (int g = 0; g < 25; g++) {
        if (g > 0 && g % 8 == 0) {
            state = rec(cc, kp, state);
            std::cout << "  ---- RECYCLE ----\n";
        }
        
        state = nand(state, c1);
        
        int lvl = state.a->GetLevel();
        double ratio = r(cc, kp, state);
        int b = bit(cc, kp, state);
        int exp = (g % 2 == 0) ? 0 : 1;
        bool ok = (b == exp);
        
        std::cout << "  " << std::setw(4) << g 
                  << " | " << std::setw(5) << lvl
                  << " | " << std::fixed << std::setprecision(4) << std::setw(10) << ratio
                  << " | " << b << "   | " << exp << "   | "
                  << (ok ? "OK" : "FAIL") << "\n";
        
        if (!ok && g > 10) {
            std::cout << "  Breaking at gate " << g << " - ratio drifted\n";
            break;
        }
    }

    // Test: fewer mulY steps
    std::cout << "\n  Testing with FEWER mulY steps:\n";
    
    for (int mulY_steps = 2; mulY_steps <= 8; mulY_steps += 2) {
        CCParams<CryptoContextCKKSRNS> p2;
        p2.SetMultiplicativeDepth(80);
        p2.SetScalingModSize(45);
        p2.SetBatchSize(1024);
        p2.SetRingDim(16384);
        p2.SetSecretKeyDist(UNIFORM_TERNARY);
        p2.SetSecurityLevel(HEStd_NotSet);
        
        auto cc2 = GenCryptoContext(p2);
        cc2->Enable(PKE); cc2->Enable(KEYSWITCH); cc2->Enable(LEVELEDSHE); cc2->Enable(ADVANCEDSHE);
        auto kp2 = cc2->KeyGen();
        cc2->EvalMultKeyGen(kp2.secretKey);

        auto nand2 = [&](PE A, PE B) -> PE {
            auto aa = cc2->EvalMult(A.a, B.a);
            auto bb = cc2->EvalMult(A.b, B.b);
            PE raw = {cc2->EvalSub(bb, aa), bb};
            for (int i = 0; i < mulY_steps; i++) raw = mulY(cc2, raw);
            return raw;
        };

        PE s2 = enc(cc2, kp2, 1);
        PE c2 = enc(cc2, kp2, 1);
        
        int ok_count = 0;
        for (int g = 0; g < 25; g++) {
            if (g > 0 && g % 8 == 0) s2 = rec(cc2, kp2, s2);
            try {
                s2 = nand2(s2, c2);
                if (bit(cc2, kp2, s2) == ((g%2==0)?0:1)) ok_count++;
            } catch (...) { break; }
        }
        
        // Also check NAND truth table
        PE b0 = enc(cc2, kp2, 0), b1 = enc(cc2, kp2, 1);
        PE in[2] = {b0, b1};
        int tt_ok = 0;
        for (int a = 0; a <= 1; a++)
            for (int b = 0; b <= 1; b++)
                if (bit(cc2, kp2, nand2(in[a], in[b])) == (1-a*b)) tt_ok++;
        
        std::cout << "  mulY=" << mulY_steps << ": NAND=" << tt_ok << "/4 Chain=" << ok_count << "/25\n";
    }

    std::cout << "\n  Done.\n\n";
    return 0;
}
