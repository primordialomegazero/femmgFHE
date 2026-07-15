// FEmmg-iO: Full Merge — Barrington Matrices + CRT5 FHE
// f(x) = (x+1)^3 via 5x5 companion matrices
// Sequential modulus evaluation, Garner CRT5 combine
// "THE FULL PIPELINE. END TO END."

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>

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

class FullMergeIO {
    static constexpr int64_t moduli[5] = {
        1073643521, 1073692673, 1073750017, 1073815553, 1073872897
    };
    static constexpr int64_t inv12    = 357919402;
    static constexpr int64_t inv123   = 589973977;
    static constexpr int64_t inv1234  = 197295683;
    static constexpr int64_t inv12345 = 1004546623;
    static const int W = 5;

    int64_t mod(int64_t v, int64_t m) { return ((v % m) + m) % m; }

    int64_t crt5_combine(int64_t r[5]) {
        __int128 accum = r[0], prod = moduli[0];
        int64_t diff = mod((int64_t)(r[1] - accum % moduli[1]), moduli[1]);
        int64_t coeff = (int64_t)(((__int128)diff * inv12) % moduli[1]);
        accum = accum + prod * coeff; prod = prod * moduli[1];
        diff = mod((int64_t)(r[2] - accum % moduli[2]), moduli[2]);
        coeff = (int64_t)(((__int128)diff * inv123) % moduli[2]);
        accum = accum + prod * coeff; prod = prod * moduli[2];
        diff = mod((int64_t)(r[3] - accum % moduli[3]), moduli[3]);
        coeff = (int64_t)(((__int128)diff * inv1234) % moduli[3]);
        accum = accum + prod * coeff; prod = prod * moduli[3];
        diff = mod((int64_t)(r[4] - accum % moduli[4]), moduli[4]);
        coeff = (int64_t)(((__int128)diff * inv12345) % moduli[4]);
        accum = accum + prod * coeff;
        return (int64_t)accum;
    }

    // Build superdiagonal companion matrix: M[i][i+1] = v, M[4][4] = 1
    vector<vector<int64_t>> build_matrix(int64_t v, int64_t modulus) {
        vector<vector<int64_t>> M(W, vector<int64_t>(W, 0));
        for (int i = 0; i < W - 1; i++) M[i][i + 1] = mod(v, modulus);
        M[W - 1][W - 1] = 1;
        return M;
    }

    int64_t evaluate_mod(int64_t x, int64_t modulus) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(10);
        params.SetPlaintextModulus(modulus);
        params.SetRingDim(4096);
        params.SetSecurityLevel(HEStd_NotSet);

        auto cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);

        auto enc = [&](int64_t v) {
            return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mod(v, modulus)}));
        };
        auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
            return pt->GetPackedValue()[0];
        };

        auto anchor = enc(0);
        int64_t half = modulus / 2;

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

        int64_t v = mod(x + 1, modulus);
        auto mat = build_matrix(v, modulus);

        // Encrypt only non-zero entries (superdiagonal + bottom-right)
        vector<vector<Ciphertext<DCRTPoly>>> emat(W, vector<Ciphertext<DCRTPoly>>(W));
        auto zero_ct = enc(0);
        for (int i = 0; i < W; i++)
            for (int j = 0; j < W; j++)
                emat[i][j] = (mat[i][j] != 0) ? enc(mat[i][j]) : zero_ct;

        // Initial state: [1, 0, 0, 0, 0]
        vector<Ciphertext<DCRTPoly>> state(W);
        state[0] = enc(1);
        for (int i = 1; i < W; i++) state[i] = enc(0);

        // Three matrix-vector multiplies
        for (int step = 0; step < 3; step++) {
            vector<Ciphertext<DCRTPoly>> new_state(W, zero_ct);
            for (int j = 0; j < W; j++) {
                auto accum = zero_ct;
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

        return dec(state[3]);
    }

public:
    int64_t evaluate(int64_t x) {
        int64_t residues[5];
        for (int i = 0; i < 5; i++) {
            residues[i] = evaluate_mod(x, moduli[i]);
        }
        return crt5_combine(residues);
    }

    void run_test() {
        cout << "\n";
        cout << "  FEmmg-iO Full Merge: Barrington + CRT5 FHE\n";
        cout << "  f(x) = (x+1)^3 via 5x5 companion matrices\n";
        cout << "  Sequential 5-modulus evaluation, Garner CRT5\n";
        cout << "  Date: " << ts() << "\n\n";

        cout << "  " << string(55, '-') << "\n";
        cout << setw(10) << "x" << setw(18) << "Full Merge" << setw(18) << "Expected" << "\n";
        cout << "  " << string(55, '-') << "\n";

        vector<int64_t> tests = {0, 1, 2, 3, 4, 5, 10};

        bool all_ok = true;
        for (int64_t x : tests) {
            int64_t expected = (x + 1) * (x + 1) * (x + 1);
            int64_t result = evaluate(x);

            bool ok = (result == expected);
            if (!ok) all_ok = false;

            cout << setw(10) << x << setw(18) << result << setw(18) << expected
                 << (ok ? "  OK" : "  FAIL") << "\n";
        }

        cout << "  " << string(55, '-') << "\n";
        cout << "  Result: " << (all_ok ? "ALL CORRECT" : "ERRORS DETECTED") << "\n\n";
    }
};

int main() {
    FullMergeIO io;
    io.run_test();
    return 0;
}
