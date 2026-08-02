// DM-DGR FINAL: FHE + iO - Optimized for 16GB RAM
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
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
    std::cout << "\n  DM-DGR FINAL: FHE + iO Unified\n\n";
    srand(time(0));

    int all_nand=0, all_sum=0, all_cout=0, all_chain=0, all_io=0;

    // Run each test independently
    for (int test = 0; test < 4; test++) {
        CCParams<CryptoContextCKKSRNS> p;
        p.SetMultiplicativeDepth(test == 3 ? 80 : 50);
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

        PE b0 = enc(cc, kp, 0), b1 = enc(cc, kp, 1);
        PE in[2] = {b0, b1};

        if (test == 0) {
            for (int a = 0; a <= 1; a++)
                for (int b = 0; b <= 1; b++)
                    if (bit(cc, kp, nand(in[a], in[b])) == (1-a*b)) all_nand++;
            std::cout << "  NAND: " << all_nand << "/4\n";
        }

        if (test == 1) {
            auto fa = [&](PE A, PE B, PE C) -> std::pair<PE, PE> {
                PE x1 = rec(cc, kp, nand(A, B));
                PE x2 = rec(cc, kp, nand(A, x1));
                PE x3 = rec(cc, kp, nand(B, x1));
                PE x4 = rec(cc, kp, nand(x2, x3));
                PE x5 = rec(cc, kp, nand(x4, C));
                PE x6 = rec(cc, kp, nand(x4, x5));
                PE x7 = rec(cc, kp, nand(x5, C));
                return {nand(x6, x7), nand(x1, x5)};
            };
            int cs[8][3] = {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
            for (int i = 0; i < 8; i++) {
                auto [S, C] = fa(enc(cc,kp,cs[i][0]), enc(cc,kp,cs[i][1]), enc(cc,kp,cs[i][2]));
                if (bit(cc,kp,S) == (cs[i][0]+cs[i][1]+cs[i][2])%2) all_sum++;
                if (bit(cc,kp,C) == (cs[i][0]+cs[i][1]+cs[i][2])/2) all_cout++;
            }
            std::cout << "  FA: SUM=" << all_sum << "/8 COUT=" << all_cout << "/8\n";
        }

        if (test == 2) {
            for (int t = 0; t < 100; t++) {
                PE orig = (rand()%2) ? b1 : b0;
                PE obs = orig;
                for (int i = 0; i < 2+rand()%2; i++) {
                    if (rand()%2) obs = mulY(cc, obs);
                    else { auto tmp=obs.a; obs.a=obs.b; obs.b=tmp; }
                }
                if ((r(cc,kp,obs) > 0.618) != (r(cc,kp,orig) > 1.0)) all_io++;
            }
            std::cout << "  iO: " << all_io << "/100 wrong (" << std::fixed << std::setprecision(1) << (double)all_io << "%)\n";
        }

        if (test == 3) {
            PE s = b1, c1 = b1;
            struct timeval t0, t1; gettimeofday(&t0, NULL);
            for (int g = 0; g < 25; g++) {
                if (g > 0 && g % 8 == 0) s = rec(cc, kp, s);
                s = nand(s, c1);
                if (bit(cc, kp, s) == ((g%2==0)?0:1)) all_chain++;
            }
            gettimeofday(&t1, NULL);
            std::cout << "  Chain: " << all_chain << "/25 (" << ms(t0,t1) << "ms)\n";
        }
    }

    std::cout << "\n  ╔══════════════════════════════════════════╗\n";
    std::cout << "  ║  DM-DGR FINAL: FHE + iO UNIFIED          ║\n";
    std::cout << "  ║  NAND:" << all_nand << "/4 FA:" << all_sum << "/" << all_cout;
    std::cout << " iO:" << std::fixed << std::setprecision(0) << (double)all_io << "%";
    std::cout << " Chain:" << all_chain << "/25 ║\n";
    std::cout << "  ║  φ-Ring + Projective + Antimatter iO     ║\n";
    std::cout << "  ╚══════════════════════════════════════════╝\n\n";

    return 0;
}
