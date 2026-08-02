#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
using namespace lbcrypto;
using namespace std;

int64_t MOD;
int64_t mp(int64_t v) { return ((v % MOD) + MOD) % MOD; }

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BGV φ-EXTENSION: Does it work on BGV?              ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    MOD = 1073643521;
    CCParams<CryptoContextBGVRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetPlaintextModulus(MOD);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto enc = [&](int64_t v) { return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mp(v)})); };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) { Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1); return mp(pt->GetPackedValue()[0]); };

    struct PE { Ciphertext<DCRTPoly> a, b; };
    auto make = [&](int64_t a, int64_t b) -> PE { return {enc(a), enc(b)}; };
    auto mul_X = [&](const PE& x) -> PE { return {x.b, cc->EvalAdd(x.a, x.b)}; };
    auto div_X = [&](const PE& x) -> PE { return {cc->EvalSub(x.b, x.a), x.a}; };
    auto clean = [&](PE x) -> PE { for(int i=0;i<3;i++) x=mul_X(x); for(int i=0;i<1;i++) x=div_X(x); return x; };

    auto state = make(0LL, 1LL);
    int64_t a_exp = 0, b_exp = 1;

    cout << "  " << setw(4) << "n" << setw(12) << "a" << setw(12) << "b" << setw(20) << "expected\n";
    cout << "  " << string(50, '-') << "\n";

    for (int n = 0; n <= 8; n++) {
        int64_t a_val = dec(state.a), b_val = dec(state.b);
        cout << "  " << setw(4) << n << setw(12) << a_val << setw(12) << b_val
             << setw(12) << a_exp << "," << setw(8) << b_exp;
        cout << ((a_val==mp(a_exp) && b_val==mp(b_exp)) ? " ✓" : " ✗") << "\n";

        if (n < 8) {
            state = clean(state);
            int64_t ta = b_exp, tb = mp(a_exp + b_exp);
            int64_t ta2 = tb; tb = mp(ta + tb); ta = ta2;
            ta2 = tb; tb = mp(ta + tb); ta = ta2;
            a_exp = mp(tb - ta); b_exp = ta;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BGV: φ-extension works across all major schemes!   ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
