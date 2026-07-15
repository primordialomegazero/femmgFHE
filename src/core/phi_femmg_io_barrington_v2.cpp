// FEmmg-iO Phase 3: Barrington Matrix Fixed
// Correct companion matrix: state * M shifts values right
// M = superdiagonal with (x+1), bottom-right = 1
// state = row vector: [1, 0, 0, 0, 0]
// After k multiplies: state[k] = (x+1)^k

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <cctype>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int main() {
    const int64_t MOD = 1073643521;
    const int W = 5;

    cout << "\n";
    cout << "  FEmmg-iO Phase 3: Barrington Matrix v2 (Row Vector)\n";
    cout << "  f(x) = (x+1)^3 via state * M * M * M\n";
    cout << "  Date: " << ts() << "\n\n";

    // FHE setup
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(8);
    params.SetPlaintextModulus(MOD);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto mod = [&](int64_t v) { return ((v % MOD) + MOD) % MOD; };
    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mod(v)}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto anchor = enc(0);
    int64_t half = MOD / 2;

    // True Divine CTxCT multiply
    auto divine = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto M_enc = enc(half);
        auto s = cc->EvalAdd(a, M_enc); s = cc->EvalAdd(s, anchor);
        auto bk = cc->EvalSub(s, M_enc); bk = cc->EvalAdd(bk, anchor);
        auto ov = cc->EvalSub(a, bk); ov = cc->EvalAdd(ov, anchor);
        auto r = cc->EvalMult(a, b);
        r = cc->EvalAdd(r, anchor); r = cc->EvalAdd(r, anchor); r = cc->EvalAdd(r, anchor);
        auto dv = cc->EvalMult(ov, anchor); dv = cc->EvalAdd(dv, anchor);
        r = cc->EvalAdd(r, dv); r = cc->EvalAdd(r, dv);
        return r;
    };

    cout << "  " << string(55, '-') << "\n";
    cout << setw(8) << "x" << setw(18) << "Barrington" << setw(18) << "Expected" << "\n";
    cout << "  " << string(55, '-') << "\n";

    vector<int64_t> tests = {0, 1, 2, 3, 4, 5};
    bool all_ok = true;

    for (int64_t x : tests) {
        int64_t v = mod(x + 1);

        // Build row-vector companion matrix M:
        // M[i][i+1] = v for i=0..3, M[4][4] = 1, all else 0
        // state * M shifts values right and multiplies by v
        vector<vector<int64_t>> mat(W, vector<int64_t>(W, 0));
        for (int i = 0; i < W - 1; i++) {
            mat[i][i + 1] = v;
        }
        mat[W - 1][W - 1] = 1;

        // Encrypt matrix entries
        vector<vector<Ciphertext<DCRTPoly>>> emat(W, vector<Ciphertext<DCRTPoly>>(W));
        for (int i = 0; i < W; i++)
            for (int j = 0; j < W; j++)
                emat[i][j] = enc(mat[i][j]);

        // Initial state: row vector [1, 0, 0, 0, 0]
        vector<Ciphertext<DCRTPoly>> state(W);
        state[0] = enc(1);
        for (int i = 1; i < W; i++) state[i] = enc(0);

        auto zero = enc(0);

        // Three matrix-vector multiplies: new_state = state * M
        // new_state[j] = sum_i state[i] * M[i][j]
        for (int step = 0; step < 3; step++) {
            vector<Ciphertext<DCRTPoly>> new_state(W);
            for (int j = 0; j < W; j++) {
                auto accum = zero;
                for (int i = 0; i < W; i++) {
                    if (mat[i][j] != 0) {
                        auto prod = divine(state[i], emat[i][j]);
                        accum = cc->EvalAdd(accum, prod);
                    }
                }
                accum = cc->EvalAdd(accum, anchor);
                new_state[j] = accum;
            }
            state = new_state;
        }

        // After 3 steps, position 3 holds (x+1)^3
        int64_t result = dec(state[3]);
        int64_t expected = (x + 1) * (x + 1) * (x + 1);
        bool ok = (result == expected);
        if (!ok) all_ok = false;

        cout << setw(8) << x << setw(18) << result << setw(18) << expected
             << (ok ? "  OK" : "  FAIL") << "\n";
    }

    cout << "  " << string(55, '-') << "\n";
    cout << "  Result: " << (all_ok ? "ALL CORRECT" : "ERRORS DETECTED") << "\n\n";

    return 0;
}
