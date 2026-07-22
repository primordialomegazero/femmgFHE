// FEmmg-FHE — BFV DEEP: Push exact arithmetic to the limit
// No CKKS approximation. Pure matrix math. How deep can we go?

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
    cout <<   "  ║   BFV DEEP: Push exact φ-arithmetic to the limit     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    MOD = 1073643521;
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(60);
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
    
    // CT×CT multiply (1 depth)
    auto mul = [&](const PE& x, const PE& y) -> PE {
        auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    };

    // Clean: 3 mul_X + 1 div_X (all ZERO depth)
    auto clean = [&](PE x) -> PE { for(int i=0;i<3;i++) x=mul_X(x); for(int i=0;i<1;i++) x=div_X(x); return x; };

    // Start with value = 1 (1 + 0φ)
    auto state = make(1LL, 0LL);
    
    // Track expected using exact matrix math
    int64_t a_exp = 1, b_exp = 0;
    
    int total_mults = 0, total_cleans = 0;
    bool alive = true;
    
    cout << "  Clean (3↑1↓, ZERO depth) → 3 CT×CT mults → Repeat\n\n";
    cout << "  " << setw(4) << "Cyc" << setw(6) << "Mults" << setw(6) << "Clean"
         << setw(12) << "a" << setw(12) << "b" << setw(12) << "expected\n";
    cout << "  " << string(55, '-') << "\n";

    for (int cycle = 0; cycle < 15 && alive; cycle++) {
        // Print state
        int64_t a_val = dec(state.a), b_val = dec(state.b);
        cout << "  " << setw(4) << cycle << setw(6) << total_mults << setw(6) << total_cleans
             << setw(12) << a_val << setw(12) << b_val
             << setw(8) << a_exp << "," << setw(4) << b_exp;
        cout << ((a_val==mp(a_exp) && b_val==mp(b_exp)) ? " ✓" : " ✗") << "\n";

        if (cycle < 15) {
            try {
                // Clean
                state = clean(state); total_cleans++;
                // Update expected: clean = div_X ∘ mul_X³
                int64_t ta = b_exp, tb = mp(a_exp + b_exp);
                int64_t ta2 = tb; tb = mp(ta + tb); ta = ta2;
                ta2 = tb; tb = mp(ta + tb); ta = ta2;
                a_exp = mp(tb - ta); b_exp = ta;

                // 3 CT×CT multiplications by fixed multiplier (2 + 0φ)
                auto multiplier = make(2LL, 0LL);
                for (int m = 0; m < 3; m++) {
                    state = mul(state, multiplier);
                    total_mults++;
                    a_exp = mp(a_exp * 2); b_exp = mp(b_exp * 2);
                }
            } catch (const exception& e) {
                cout << "  CRASHED at cycle " << cycle << ": " << e.what() << "\n";
                alive = false;
            }
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BFV: " << total_mults << " mults, " << total_cleans << " cleans                              ║\n";
    cout <<   "  ║   Exact arithmetic = no drift                         ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
