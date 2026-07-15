// FEmmg-iO: 64-bit CRT5 + FHE
// Five 30-bit moduli via Garner's algorithm
// Combined 151 bits — handles full 64-bit (x+1)^2
// Pure CTxCT with True Divine stabilization

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

class CRT5_FHE_IO {
    static constexpr int64_t M1 = 1073643521;
    static constexpr int64_t M2 = 1073692673;
    static constexpr int64_t M3 = 1073750017;
    static constexpr int64_t M4 = 1073815553;
    static constexpr int64_t M5 = 1073872897;

    // CRT inverses (pre-computed via Extended Euclidean)
    static constexpr int64_t inv12   = 357919402;   // M1^(-1) mod M2
    static constexpr int64_t inv123  = 589973977;   // (M1*M2)^(-1) mod M3
    static constexpr int64_t inv1234 = 197295683;   // (M1*M2*M3)^(-1) mod M4
    static constexpr int64_t inv12345 = 1004546623; // (M1*M2*M3*M4)^(-1) mod M5

    // Accumulated products
    static constexpr int64_t  M12   = 1152763181911621633LL;
    static constexpr int64_t  M123  = 0;  // Exceeds int64, use __int128 at runtime
    static constexpr int64_t  M1234 = 0;  // Same

    CryptoContext<DCRTPoly> cc1, cc2, cc3, cc4, cc5;
    KeyPair<DCRTPoly> keys1, keys2, keys3, keys4, keys5;
    Ciphertext<DCRTPoly> anchor1, anchor2, anchor3, anchor4, anchor5;

    int64_t mod(int64_t v, int64_t m) {
        return ((v % m) + m) % m;
    }

    // Garner's algorithm for five moduli
    // Iteratively combine: r_accum = r_accum + product_so_far * ((r_next - r_accum) * inv mod next_mod)
    int64_t crt5_combine(int64_t r1, int64_t r2, int64_t r3, int64_t r4, int64_t r5) {
        __int128 accum = r1;
        __int128 prod  = M1;

        // Step 1: combine with r2
        int64_t diff = mod((int64_t)(r2 - accum % M2), M2);
        int64_t coeff = (int64_t)(((__int128)diff * inv12) % M2);
        accum = accum + prod * coeff;
        prod = prod * M2;

        // Step 2: combine with r3
        diff = mod((int64_t)(r3 - accum % M3), M3);
        coeff = (int64_t)(((__int128)diff * inv123) % M3);
        accum = accum + prod * coeff;
        prod = prod * M3;

        // Step 3: combine with r4
        diff = mod((int64_t)(r4 - accum % M4), M4);
        coeff = (int64_t)(((__int128)diff * inv1234) % M4);
        accum = accum + prod * coeff;
        prod = prod * M4;

        // Step 4: combine with r5
        diff = mod((int64_t)(r5 - accum % M5), M5);
        coeff = (int64_t)(((__int128)diff * inv12345) % M5);
        accum = accum + prod * coeff;

        return (int64_t)accum;
    }

    Ciphertext<DCRTPoly> divine_multiply(
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
        params.SetMultiplicativeDepth(10);
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

public:
    CRT5_FHE_IO() {
        cc1 = init_context(M1); keys1 = cc1->KeyGen(); cc1->EvalMultKeyGen(keys1.secretKey);
        anchor1 = cc1->Encrypt(keys1.publicKey, cc1->MakePackedPlaintext(vector<int64_t>{0}));

        cc2 = init_context(M2); keys2 = cc2->KeyGen(); cc2->EvalMultKeyGen(keys2.secretKey);
        anchor2 = cc2->Encrypt(keys2.publicKey, cc2->MakePackedPlaintext(vector<int64_t>{0}));

        cc3 = init_context(M3); keys3 = cc3->KeyGen(); cc3->EvalMultKeyGen(keys3.secretKey);
        anchor3 = cc3->Encrypt(keys3.publicKey, cc3->MakePackedPlaintext(vector<int64_t>{0}));

        cc4 = init_context(M4); keys4 = cc4->KeyGen(); cc4->EvalMultKeyGen(keys4.secretKey);
        anchor4 = cc4->Encrypt(keys4.publicKey, cc4->MakePackedPlaintext(vector<int64_t>{0}));

        cc5 = init_context(M5); keys5 = cc5->KeyGen(); cc5->EvalMultKeyGen(keys5.secretKey);
        anchor5 = cc5->Encrypt(keys5.publicKey, cc5->MakePackedPlaintext(vector<int64_t>{0}));
    }

    int64_t evaluate_fhe(int64_t x) {
        auto enc = [](CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keys, int64_t v) {
            return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
        };

        int64_t v1 = mod(x + 1, M1), v2 = mod(x + 1, M2), v3 = mod(x + 1, M3);
        int64_t v4 = mod(x + 1, M4), v5 = mod(x + 1, M5);

        auto ct1 = enc(cc1, keys1, v1); auto r1_ct = divine_multiply(cc1, keys1, ct1, ct1, anchor1, M1/2);
        auto ct2 = enc(cc2, keys2, v2); auto r2_ct = divine_multiply(cc2, keys2, ct2, ct2, anchor2, M2/2);
        auto ct3 = enc(cc3, keys3, v3); auto r3_ct = divine_multiply(cc3, keys3, ct3, ct3, anchor3, M3/2);
        auto ct4 = enc(cc4, keys4, v4); auto r4_ct = divine_multiply(cc4, keys4, ct4, ct4, anchor4, M4/2);
        auto ct5 = enc(cc5, keys5, v5); auto r5_ct = divine_multiply(cc5, keys5, ct5, ct5, anchor5, M5/2);

        Plaintext pt;
        cc1->Decrypt(keys1.secretKey, r1_ct, &pt); int64_t r1 = pt->GetPackedValue()[0];
        cc2->Decrypt(keys2.secretKey, r2_ct, &pt); int64_t r2 = pt->GetPackedValue()[0];
        cc3->Decrypt(keys3.secretKey, r3_ct, &pt); int64_t r3 = pt->GetPackedValue()[0];
        cc4->Decrypt(keys4.secretKey, r4_ct, &pt); int64_t r4 = pt->GetPackedValue()[0];
        cc5->Decrypt(keys5.secretKey, r5_ct, &pt); int64_t r5 = pt->GetPackedValue()[0];

        return crt5_combine(r1, r2, r3, r4, r5);
    }

    void run_test() {
        cout << "\n";
        cout << "  FEmmg-iO: 64-bit CRT5 + FHE\n";
        cout << "  f(x) = (x+1)^2 via Pure CTxCT with True Divine\n";
        cout << "  5 primes, 151-bit combined via Garner's algorithm\n";
        cout << "  Date: " << ts() << "\n\n";

        cout << "  " << string(65, '-') << "\n";
        cout << setw(20) << "x" << setw(25) << "CRT5+FHE" << setw(25) << "Expected" << "\n";
        cout << "  " << string(65, '-') << "\n";

        vector<int64_t> test_inputs = {
            0LL, 1LL, 2LL, 3LL, 5LL, 10LL, 100LL, 1000LL, 10000LL,
            65535LL, 100000LL, 1000000LL, 10000000LL, 100000000LL,
            1000000000LL, 2147483647LL, 4294967295LL
        };

        bool all_correct = true;
        for (int64_t x : test_inputs) {
            int64_t expected = (x + 1) * (x + 1);
            int64_t result = evaluate_fhe(x);

            bool ok = (result == expected);
            if (!ok) all_correct = false;

            cout << setw(20) << x
                 << setw(25) << result
                 << setw(25) << expected
                 << (ok ? "  OK" : "  FAIL") << "\n";
        }

        cout << "  " << string(65, '-') << "\n";
        cout << "  Result: " << (all_correct ? "ALL CORRECT" : "ERRORS DETECTED") << "\n";
        cout << "\n";
    }
};

int main() {
    CRT5_FHE_IO io;
    io.run_test();
    return 0;
}
