// FEmmg-iO Phase 2: Sequential CRT5 Cubic
// f(x) = (x+1)^3 via two CTxCT multiplies
// Sequential modulus evaluation — one context at a time
// Foundation for Phase 3: matrix-based general polynomials

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

class SequentialCubicCRT5 {
    static constexpr int64_t moduli[5] = {
        1073643521, 1073692673, 1073750017, 1073815553, 1073872897
    };
    static constexpr int64_t inv12    = 357919402;
    static constexpr int64_t inv123   = 589973977;
    static constexpr int64_t inv1234  = 197295683;
    static constexpr int64_t inv12345 = 1004546623;

    int64_t mod(int64_t v, int64_t m) {
        return ((v % m) + m) % m;
    }

    int64_t crt5_combine(int64_t r[5]) {
        __int128 accum = r[0];
        __int128 prod  = moduli[0];

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
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            return pt->GetPackedValue()[0];
        };

        auto anchor = enc(0);
        int64_t half = modulus / 2;

        auto true_divine = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
            auto M_enc = enc(half);
            auto sum = cc->EvalAdd(a, M_enc); sum = cc->EvalAdd(sum, anchor);
            auto back = cc->EvalSub(sum, M_enc); back = cc->EvalAdd(back, anchor);
            auto overflow = cc->EvalSub(a, back); overflow = cc->EvalAdd(overflow, anchor);
            auto result = cc->EvalMult(a, b);
            result = cc->EvalAdd(result, anchor); result = cc->EvalAdd(result, anchor); result = cc->EvalAdd(result, anchor);
            auto divine = cc->EvalMult(overflow, anchor); divine = cc->EvalAdd(divine, anchor);
            result = cc->EvalAdd(result, divine); result = cc->EvalAdd(result, divine);
            return result;
        };

        int64_t v = mod(x + 1, modulus);
        auto ct_val = enc(v);

        // (x+1)^2
        auto ct_sq = true_divine(ct_val, ct_val);

        // (x+1)^3 = (x+1)^2 * (x+1)
        auto ct_cu = true_divine(ct_sq, ct_val);

        return dec(ct_cu);
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
        cout << "  FEmmg-iO Phase 2: Sequential CRT5 Cubic\n";
        cout << "  f(x) = (x+1)^3 via two CTxCT multiplies\n";
        cout << "  Sequential modulus evaluation\n";
        cout << "  Date: " << ts() << "\n\n";

        cout << "  " << string(60, '-') << "\n";
        cout << setw(12) << "x" << setw(22) << "CRT5+FHE" << setw(22) << "Expected" << "\n";
        cout << "  " << string(60, '-') << "\n";

        vector<int64_t> tests = {
            0, 1, 2, 3, 4, 5, 10, 42, 100, 1000,
            10000, 100000, 1000000, 10000000
        };

        bool all_ok = true;
        for (int64_t x : tests) {
            int64_t expected = (x + 1) * (x + 1) * (x + 1);
            int64_t result = evaluate(x);

            bool ok = (result == expected);
            if (!ok) all_ok = false;

            cout << setw(12) << x
                 << setw(22) << result
                 << setw(22) << expected
                 << (ok ? "  OK" : "  FAIL") << "\n";
        }

        cout << "  " << string(60, '-') << "\n";
        cout << "  Result: " << (all_ok ? "ALL CORRECT" : "ERRORS DETECTED") << "\n";
        cout << "\n";
    }
};

int main() {
    SequentialCubicCRT5 cubic;
    cubic.run_test();
    return 0;
}
