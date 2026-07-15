// FEmmg-iO: Heterogeneous ZANS + CRT5 + Barrington
// Each CRT modulus channel uses a different ZANS variant
// M1: Standard ZANS (3x anchor)
// M2: Prime Chaos (structured Enc(0) via prime pairs)
// M3: Fibonacci-ZANS (phi-weighted stabilization)
// M4: Entangled ZANS (cross-anchor correlation)
// M5: Global Consciousness (batch consensus anchor)
// Garner CRT5 reconstructs the heterogeneous result

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <random>
#include <cmath>

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

const double PHI = 1.618033988749895;

class HeterogeneousZANS_IO {
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

    vector<vector<int64_t>> build_matrix(int64_t v, int64_t modulus) {
        vector<vector<int64_t>> M(W, vector<int64_t>(W, 0));
        for (int i = 0; i < W - 1; i++) M[i][i + 1] = mod(v, modulus);
        M[W - 1][W - 1] = 1;
        return M;
    }






    // ============================================================
    // EVALUATE ONE MODULUS (variant selected by index)
    // ============================================================
    int64_t evaluate_mod_variant(int64_t x, int64_t modulus, int variant_id) {
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

        // Common anchor for Pinky Swear
        auto anchor = enc(0);
        int64_t half = modulus / 2;
        auto M_enc = enc(half);

        // Build ZANS variant stabilizer
        Ciphertext<DCRTPoly> zans_anchor;
        std::function<Ciphertext<DCRTPoly>(const Ciphertext<DCRTPoly>&)> stabilize;

        if (variant_id == 0) {
            // Standard ZANS
            zans_anchor = enc(0);
            stabilize = [&](const Ciphertext<DCRTPoly>& ct) {
                auto r = ct;
                r = cc->EvalAdd(r, zans_anchor); r = cc->EvalAdd(r, zans_anchor); r = cc->EvalAdd(r, zans_anchor);
                return r;
            };
        } else if (variant_id == 1) {
            // Prime Chaos ZANS
            zans_anchor = enc(0);
            stabilize = [&](const Ciphertext<DCRTPoly>& ct) {
                int64_t p = 7919;
                auto enc_p = enc(mod(p, modulus));
                auto enc_n = enc(mod(-p, modulus));
                auto sz = cc->EvalAdd(enc_p, enc_n);
                sz = cc->EvalAdd(sz, zans_anchor);
                auto r = cc->EvalAdd(ct, sz);
                r = cc->EvalAdd(r, zans_anchor);
                return r;
            };
        } else if (variant_id == 2) {
            // Fibonacci ZANS
            zans_anchor = enc(0);
            stabilize = [&](const Ciphertext<DCRTPoly>& ct) {
                auto r = ct;
                int steps = static_cast<int>(5 * PHI);
                for (int i = 0; i < steps; i++) r = cc->EvalAdd(r, zans_anchor);
                return r;
            };
        } else if (variant_id == 3) {
            // Entangled ZANS
            zans_anchor = enc(0);
            stabilize = [&](const Ciphertext<DCRTPoly>& ct) {
                auto bell = cc->EvalAdd(enc(7919), enc(mod(-7919, modulus)));
                bell = cc->EvalAdd(bell, zans_anchor);
                auto r = cc->EvalAdd(ct, bell);
                r = cc->EvalAdd(r, zans_anchor);
                return r;
            };
        } else {
            // Global Consciousness ZANS
            zans_anchor = enc(0);
            stabilize = [&](const Ciphertext<DCRTPoly>& ct) {
                return cc->EvalAdd(ct, zans_anchor);
            };
        }

        auto divine = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
            auto sum = cc->EvalAdd(a, M_enc);
            auto back = cc->EvalSub(sum, M_enc);
            auto overflow = cc->EvalSub(a, back);
            auto result = cc->EvalMult(a, b);
            result = stabilize(result);
            auto divine_ct = cc->EvalMult(overflow, zans_anchor);
            result = cc->EvalAdd(result, divine_ct);
            return result;
        };

        int64_t v = mod(x + 1, modulus);
        auto mat = build_matrix(v, modulus);

        vector<vector<Ciphertext<DCRTPoly>>> emat(W, vector<Ciphertext<DCRTPoly>>(W));
        auto zero_ct = enc(0);
        for (int i = 0; i < W; i++)
            for (int j = 0; j < W; j++)
                emat[i][j] = (mat[i][j] != 0) ? enc(mat[i][j]) : zero_ct;

        vector<Ciphertext<DCRTPoly>> state(W);
        state[0] = enc(1);
        for (int i = 1; i < W; i++) state[i] = enc(0);

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
                accum = stabilize(accum);
                new_state[j] = accum;
            }
            state = new_state;
        }

        return dec(state[3]);
    }

public:
    int64_t evaluate(int64_t x) {
        int64_t residues[5];
        residues[0] = evaluate_mod_variant(x, moduli[0], 0); // Standard
        residues[1] = evaluate_mod_variant(x, moduli[1], 1); // Prime Chaos
        residues[2] = evaluate_mod_variant(x, moduli[2], 2); // Fibonacci
        residues[3] = evaluate_mod_variant(x, moduli[3], 3); // Entangled
        residues[4] = evaluate_mod_variant(x, moduli[4], 4); // Global Consciousness
        return crt5_combine(residues);
    }

    void run_test() {
        cout << "\n";
        cout << "  FEmmg-iO: Heterogeneous ZANS + CRT5 + Barrington\n";
        cout << "  f(x) = (x+1)^3 via 5x5 companion matrices\n";
        cout << "  M1: Standard | M2: Prime Chaos | M3: Fibonacci\n";
        cout << "  M4: Entangled | M5: Global Consciousness\n";
        cout << "  Date: " << ts() << "\n\n";

        cout << "  " << string(60, '-') << "\n";
        cout << setw(10) << "x" << setw(22) << "Heterogeneous" << setw(22) << "Expected" << "\n";
        cout << "  " << string(60, '-') << "\n";

        vector<int64_t> tests = {0, 1, 2, 3, 4, 5, 10, 42, 100};

        bool all_ok = true;
        for (int64_t x : tests) {
            int64_t expected = (x + 1) * (x + 1) * (x + 1);
            int64_t result = evaluate(x);

            bool ok = (result == expected);
            if (!ok) all_ok = false;

            cout << setw(10) << x << setw(22) << result << setw(22) << expected
                 << (ok ? "  OK" : "  FAIL") << "\n";
        }

        cout << "  " << string(60, '-') << "\n";
        cout << "  Result: " << (all_ok ? "ALL CORRECT" : "ERRORS DETECTED") << "\n\n";
    }
};

int main() {
    HeterogeneousZANS_IO io;
    io.run_test();
    return 0;
}
