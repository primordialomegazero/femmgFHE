// FEmmg-iO: Cubic Polynomial via CRT5 + FHE
// f(x) = (x+1)^3 using two sequential CTxCT multiplications
// Tests encrypted output reuse: (x+1)^2 becomes input to second multiply
// Phase 1 of P/Poly iO pipeline

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

class CubicCRT5 {
    static constexpr int64_t M1 = 1073643521;
    static constexpr int64_t M2 = 1073692673;
    static constexpr int64_t M3 = 1073750017;
    static constexpr int64_t M4 = 1073815553;
    static constexpr int64_t M5 = 1073872897;

    static constexpr int64_t inv12    = 357919402;
    static constexpr int64_t inv123   = 589973977;
    static constexpr int64_t inv1234  = 197295683;
    static constexpr int64_t inv12345 = 1004546623;

    CryptoContext<DCRTPoly> cc1, cc2, cc3, cc4, cc5;
    KeyPair<DCRTPoly> keys1, keys2, keys3, keys4, keys5;
    Ciphertext<DCRTPoly> anchor1, anchor2, anchor3, anchor4, anchor5;

    int64_t mod(int64_t v, int64_t m) {
        return ((v % m) + m) % m;
    }

    int64_t crt5_combine(int64_t r1, int64_t r2, int64_t r3, int64_t r4, int64_t r5) {
        __int128 accum = r1;
        __int128 prod  = M1;

        int64_t diff = mod((int64_t)(r2 - accum % M2), M2);
        int64_t coeff = (int64_t)(((__int128)diff * inv12) % M2);
        accum = accum + prod * coeff;
        prod = prod * M2;

        diff = mod((int64_t)(r3 - accum % M3), M3);
        coeff = (int64_t)(((__int128)diff * inv123) % M3);
        accum = accum + prod * coeff;
        prod = prod * M3;

        diff = mod((int64_t)(r4 - accum % M4), M4);
        coeff = (int64_t)(((__int128)diff * inv1234) % M4);
        accum = accum + prod * coeff;
        prod = prod * M4;

        diff = mod((int64_t)(r5 - accum % M5), M5);
        coeff = (int64_t)(((__int128)diff * inv12345) % M5);
        accum = accum + prod * coeff;

        return (int64_t)accum;
    }

    Ciphertext<DCRTPoly> true_divine_ctct(
        CryptoContext<DCRTPoly>& cc,
        const KeyPair<DCRTPoly>& keys,
        const Ciphertext<DCRTPoly>& a,
        const Ciphertext<DCRTPoly>& b,
        const Ciphertext<DCRTPoly>& anchor,
        int64_t half_mod
    ) {
        auto M_enc = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));

        auto sum = cc->EvalAdd(a, M_enc);
        sum = cc->EvalAdd(sum, anchor);
        auto back = cc->EvalSub(sum, M_enc);
        back = cc->EvalAdd(back, anchor);
        auto overflow = cc->EvalSub(a, back);
        overflow = cc->EvalAdd(overflow, anchor);

        auto result = cc->EvalMult(a, b);

        result = cc->EvalAdd(result, anchor);
        result = cc->EvalAdd(result, anchor);
        result = cc->EvalAdd(result, anchor);

        auto divine = cc->EvalMult(overflow, anchor);
        divine = cc->EvalAdd(divine, anchor);
        result = cc->EvalAdd(result, divine);
        result = cc->EvalAdd(result, divine);

        return result;
    }

    CryptoContext<DCRTPoly> init_context(int64_t modulus) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(20);
        params.SetPlaintextModulus(modulus);
        params.SetRingDim(4096);
        params.SetSecurityLevel(HEStd_NotSet);

        auto cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        cc->Enable(ADVANCEDSHE);
        return cc;
    }

    struct ModContext {
        CryptoContext<DCRTPoly> cc;
        KeyPair<DCRTPoly> keys;
        Ciphertext<DCRTPoly> anchor;
        int64_t modulus;
        int64_t half_mod;
    };

    ModContext ctx1, ctx2, ctx3, ctx4, ctx5;

    void init_mod(ModContext& ctx, int64_t mod) {
        ctx.cc = init_context(mod);
        ctx.keys = ctx.cc->KeyGen();
        ctx.cc->EvalMultKeyGen(ctx.keys.secretKey);
        ctx.anchor = ctx.cc->Encrypt(ctx.keys.publicKey, ctx.cc->MakePackedPlaintext(vector<int64_t>{0}));
        ctx.modulus = mod;
        ctx.half_mod = mod / 2;
    }

    int64_t evaluate_cubic_mod(ModContext& ctx, int64_t x) {
        int64_t v = mod(x + 1, ctx.modulus);
        auto ct = ctx.cc->Encrypt(ctx.keys.publicKey, ctx.cc->MakePackedPlaintext(vector<int64_t>{v}));

        // Step 1: (x+1)^2
        auto square = true_divine_ctct(ctx.cc, ctx.keys, ct, ct, ctx.anchor, ctx.half_mod);

        // Step 2: (x+1)^3 = (x+1)^2 × (x+1)
        auto cube = true_divine_ctct(ctx.cc, ctx.keys, square, ct, ctx.anchor, ctx.half_mod);

        Plaintext pt;
        ctx.cc->Decrypt(ctx.keys.secretKey, cube, &pt);
        return pt->GetPackedValue()[0];
    }

public:
    CubicCRT5() {
        init_mod(ctx1, M1); init_mod(ctx2, M2); init_mod(ctx3, M3);
        init_mod(ctx4, M4); init_mod(ctx5, M5);
    }

    int64_t evaluate(int64_t x) {
        int64_t r1 = evaluate_cubic_mod(ctx1, x);
        int64_t r2 = evaluate_cubic_mod(ctx2, x);
        int64_t r3 = evaluate_cubic_mod(ctx3, x);
        int64_t r4 = evaluate_cubic_mod(ctx4, x);
        int64_t r5 = evaluate_cubic_mod(ctx5, x);

        return crt5_combine(r1, r2, r3, r4, r5);
    }

    void run_test() {
        cout << "\n";
        cout << "  FEmmg-iO Phase 1: Cubic via CRT5 + FHE\n";
        cout << "  f(x) = (x+1)^3 — two sequential CTxCT multiplies\n";
        cout << "  Encrypted output of multiply 1 feeds multiply 2\n";
        cout << "  Date: " << ts() << "\n";
        cout << "  Combined modulus: ~151 bits\n\n";

        cout << "  " << string(65, '-') << "\n";
        cout << setw(12) << "x" << setw(22) << "CRT5+FHE" << setw(22) << "Expected" << "\n";
        cout << "  " << string(65, '-') << "\n";

        vector<int64_t> tests = {
            0, 1, 2, 3, 4, 5, 7, 10, 15, 42, 100,
            1000, 10000, 100000, 1000000, 10000000,
            100000000, 1000000000, 2147483647
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

        cout << "  " << string(65, '-') << "\n";
        cout << "  Result: " << (all_ok ? "ALL CORRECT" : "ERRORS DETECTED") << "\n";
        cout << "\n";
    }
};

int main() {
    CubicCRT5 cubic;
    cubic.run_test();
    return 0;
}
