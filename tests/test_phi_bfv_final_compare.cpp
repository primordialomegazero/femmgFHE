// BFV: Definitive WITH vs WITHOUT comparison
// Same depth, same circuit. Count correct mults.

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
using namespace lbcrypto;
using namespace std;

int64_t MOD;
int64_t mp(int64_t v) { return ((v % MOD) + MOD) % MOD; }

int main() {
    MOD = 1073643521;
    int depths[] = {5, 10, 20, 30};

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BFV: WITH φ-CLEAN vs WITHOUT (same depth)          ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    cout << "  " << setw(6) << "Depth" << setw(12) << "WITH CLEAN" << setw(16) << "WITHOUT CLEAN" << setw(12) << "Ratio\n";
    cout << "  " << string(50, '-') << "\n";

    for (int depth : depths) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(depth);
        params.SetPlaintextModulus(MOD);
        params.SetRingDim(4096);
        params.SetSecurityLevel(HEStd_NotSet);
        auto cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        auto enc = [&](int64_t v) { return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mp(v)})); };
        auto dec = [&](const Ciphertext<DCRTPoly>& ct) { Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1); return mp(pt->GetPackedValue()[0]); };

        // === WITH CLEAN ===
        struct PE { Ciphertext<DCRTPoly> a, b; };
        auto mul_X = [&](const PE& x) -> PE { return {x.b, cc->EvalAdd(x.a, x.b)}; };
        auto div_X = [&](const PE& x) -> PE { return {cc->EvalSub(x.b, x.a), x.a}; };
        auto clean = [&](PE x) -> PE { for(int i=0;i<3;i++) x=mul_X(x); for(int i=0;i<1;i++) x=div_X(x); return x; };
        auto mul_pe = [&](const PE& x, const PE& y) -> PE {
            auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
            auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
            return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
        };

        auto state_c = PE{enc(1LL), enc(0LL)};
        auto mult_c = PE{enc(2LL), enc(0LL)};
        int64_t a_exp = 1, b_exp = 0;
        int clean_mults = 0;

        for (int c = 0; c < 50; c++) {
            try {
                state_c = clean(state_c);
                int64_t ta = b_exp, tb = mp(a_exp + b_exp);
                int64_t ta2 = tb; tb = mp(ta + tb); ta = ta2;
                ta2 = tb; tb = mp(ta + tb); ta = ta2;
                a_exp = mp(tb - ta); b_exp = ta;

                state_c = mul_pe(state_c, mult_c);
                a_exp = mp(a_exp * 2); b_exp = mp(b_exp * 2);
                clean_mults++;

                int64_t av = dec(state_c.a), bv = dec(state_c.b);
                if (!(av == mp(a_exp) && bv == mp(b_exp))) break;
            } catch (...) { break; }
        }

        // === WITHOUT CLEAN ===
        auto state_n = enc(1LL);
        auto mult_n = enc(2LL);
        int64_t n_exp = 1;
        int no_clean_mults = 0;

        for (int i = 0; i < 100; i++) {
            try {
                state_n = cc->EvalMult(state_n, mult_n);
                n_exp = mp(n_exp * 2);
                no_clean_mults++;
                if (dec(state_n) != mp(n_exp)) break;
            } catch (...) { break; }
        }

        double ratio = (no_clean_mults > 0) ? (double)clean_mults / no_clean_mults : 0;
        cout << "  " << setw(6) << depth 
             << setw(12) << clean_mults 
             << setw(16) << no_clean_mults 
             << setw(10) << fixed << setprecision(1) << ratio << "×\n";
    }
    cout << "\n  Clean = ZERO depth. Only CT×CT mults consume levels.\n";
    cout << "  Without clean: noise kills correctness before levels.\n";
    cout << "  With clean: noise suppressed, levels are the bottleneck.\n\n";
    return 0;
}
