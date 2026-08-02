// Find minimum depth for BFV φ-chain
#include <openfhe.h>
#include <iostream>
#include <vector>
using namespace lbcrypto;
using namespace std;

int64_t MOD;
int64_t mp(int64_t v) { return ((v % MOD) + MOD) % MOD; }

int test_at_depth(int depth, int cycles) {
    MOD = 1073643521;
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

    struct PE { Ciphertext<DCRTPoly> a, b; };
    auto mul_X = [&](const PE& x) -> PE { return {x.b, cc->EvalAdd(x.a, x.b)}; };
    auto div_X = [&](const PE& x) -> PE { return {cc->EvalSub(x.b, x.a), x.a}; };
    auto clean = [&](PE x) -> PE { for(int i=0;i<3;i++) x=mul_X(x); for(int i=0;i<1;i++) x=div_X(x); return x; };
    auto mul = [&](const PE& x, const PE& y) -> PE {
        auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    };

    auto state = PE{enc(1LL), enc(0LL)};
    auto multiplier = PE{enc(2LL), enc(0LL)};
    int64_t a_exp = 1, b_exp = 0;

    for (int c = 0; c < cycles; c++) {
        try {
            state = clean(state);
            int64_t ta = b_exp, tb = mp(a_exp + b_exp);
            int64_t ta2 = tb; tb = mp(ta + tb); ta = ta2;
            ta2 = tb; tb = mp(ta + tb); ta = ta2;
            a_exp = mp(tb - ta); b_exp = ta;

            for (int m = 0; m < 3; m++) {
                state = mul(state, multiplier);
                a_exp = mp(a_exp * 2); b_exp = mp(b_exp * 2);
            }
        } catch (...) { return c * 3; }
    }
    return cycles * 3;
}

int main() {
    cout << "\n  BFV Minimum Depth Test (45 mults = 15 cycles)\n\n";
    cout << "  " << setw(6) << "Depth" << setw(8) << "Mults" << setw(10) << "Status\n";
    cout << "  " << string(30, '-') << "\n";
    
    int depths[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int d : depths) {
        int mults = test_at_depth(d, 15);
        cout << "  " << setw(6) << d << setw(8) << mults;
        if (mults >= 45) cout << setw(10) << "✓ ALL 45\n";
        else cout << setw(10) << ("only " + to_string(mults) + "\n");
    }
    return 0;
}
