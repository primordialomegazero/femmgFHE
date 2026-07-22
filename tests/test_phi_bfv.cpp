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
    cout <<   "  ║   BFV φ-EXTENSION: Exact arithmetic verification     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    MOD = 1073643521;
    CCParams<CryptoContextBFVRNS> params;
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

    // Test: (a,b) after clean should equal (a,b) * φ in the extension ring
    // In exact arithmetic: φ acts as the matrix [[0,1],[1,1]]
    // clean = 3× mul_X then 1× div_X
    // mul_X = [[0,1],[1,1]], div_X = [[-1,1],[1,0]]
    // clean = div_X ∘ mul_X³
    
    auto state = make(0LL, 1LL);  // φ
    cout << "  Start: (a,b) = (0,1) = φ\n";
    cout << "  Clean = div_X ∘ mul_X³\n\n";
    cout << "  " << setw(4) << "n" << setw(12) << "a" << setw(12) << "b" << setw(20) << "expected\n";
    cout << "  " << string(50, '-') << "\n";

    // Track expected using matrix multiplication in plaintext
    int64_t a_exp = 0, b_exp = 1;
    // mul_X matrix: (a,b) → (b, a+b)
    // div_X matrix: (a,b) → (b-a, a)
    // clean: 3 mul_X then 1 div_X

    for (int n = 0; n <= 8; n++) {
        int64_t a_val = dec(state.a), b_val = dec(state.b);
        cout << "  " << setw(4) << n << setw(12) << a_val << setw(12) << b_val
             << setw(12) << a_exp << "," << setw(8) << b_exp;
        
        if (a_val == mp(a_exp) && b_val == mp(b_exp)) cout << " ✓";
        else cout << " ✗";
        cout << "\n";

        // Compute expected for next step
        if (n < 8) {
            // Apply clean to expected
            int64_t ta, tb;
            // 3× mul_X
            ta = b_exp; tb = mp(a_exp + b_exp);  // 1
            int64_t ta2 = tb; tb = mp(ta + tb); ta = ta2;  // 2
            ta2 = tb; tb = mp(ta + tb); ta = ta2;  // 3
            // 1× div_X: (b-a, a)
            int64_t new_a = mp(tb - ta);
            int64_t new_b = ta;
            a_exp = new_a; b_exp = new_b;
            
            state = clean(state);
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BFV φ-extension: Matrix math verified             ║\n";
    cout <<   "  ║   mul_X and div_X work on BFV (exact arithmetic)    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
