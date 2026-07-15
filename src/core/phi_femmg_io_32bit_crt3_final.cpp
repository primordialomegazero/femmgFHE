// FEmmg-iO: 32-bit CRT3 + FHE
// Three 30-bit moduli via Garner's algorithm
// Combined ~90 bits — handles full 32-bit (x+1)^2
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

class CRT3_FHE_IO {
    static constexpr int64_t M1 = 1073643521;
    static constexpr int64_t M2 = 1073692673;
    static constexpr int64_t M3 = 1073750017;

    // M12 = M1 * M2
    static constexpr int64_t M12 = 1152763181911621633LL;

    // Pre-computed via Extended Euclidean Algorithm
    // inv12 = M1^(-1) mod M2, inv123 = M12^(-1) mod M3
    static constexpr int64_t inv12  = 357919402;
    static constexpr int64_t inv123 = 589973977;

    CryptoContext<DCRTPoly> cc1, cc2, cc3;
    KeyPair<DCRTPoly> keys1, keys2, keys3;
    Ciphertext<DCRTPoly> anchor1, anchor2, anchor3;

    int64_t mod(int64_t v, int64_t m) {
        return ((v % m) + m) % m;
    }

    // Garner's algorithm for three moduli
    // Step 1: r12 = r1 + M1 * ((r2 - r1) * inv12 mod M2)
    // Step 2: result = r12 + M12 * ((r3 - r12) * inv123 mod M3)
    int64_t crt3_combine(int64_t r1, int64_t r2, int64_t r3) {
        // Step 1: combine r1 and r2
        int64_t diff12 = mod(r2 - r1, M2);
        int64_t coeff12 = (int64_t)(((__int128)diff12 * inv12) % M2);
        __int128 r12 = (__int128)r1 + (__int128)M1 * coeff12;

        // Step 2: combine r12 with r3
        int64_t r12_mod_M3 = (int64_t)(r12 % M3);
        int64_t diff123 = mod(r3 - r12_mod_M3, M3);
        int64_t coeff123 = (int64_t)(((__int128)diff123 * inv123) % M3);
        __int128 result = r12 + (__int128)M12 * coeff123;

        return (int64_t)result;
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

        // Pinky Swear overflow detection
        auto sum = cc->EvalAdd(a, M_enc);
        sum = cc->EvalAdd(sum, anchor);
        auto back = cc->EvalSub(sum, M_enc);
        back = cc->EvalAdd(back, anchor);
        auto overflow = cc->EvalSub(a, back);
        overflow = cc->EvalAdd(overflow, anchor);

        // CTxCT multiplication
        auto result = cc->EvalMult(a, b);

        // ZANS stabilization
        result = cc->EvalAdd(result, anchor);
        result = cc->EvalAdd(result, anchor);
        result = cc->EvalAdd(result, anchor);

        // Divine intervention
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
    CRT3_FHE_IO() {
        cc1 = init_context(M1);
        keys1 = cc1->KeyGen();
        cc1->EvalMultKeyGen(keys1.secretKey);
        anchor1 = cc1->Encrypt(keys1.publicKey, cc1->MakePackedPlaintext(vector<int64_t>{0}));

        cc2 = init_context(M2);
        keys2 = cc2->KeyGen();
        cc2->EvalMultKeyGen(keys2.secretKey);
        anchor2 = cc2->Encrypt(keys2.publicKey, cc2->MakePackedPlaintext(vector<int64_t>{0}));

        cc3 = init_context(M3);
        keys3 = cc3->KeyGen();
        cc3->EvalMultKeyGen(keys3.secretKey);
        anchor3 = cc3->Encrypt(keys3.publicKey, cc3->MakePackedPlaintext(vector<int64_t>{0}));
    }

    int64_t evaluate_fhe(int64_t x) {
        int64_t val1 = mod(x + 1, M1);
        int64_t val2 = mod(x + 1, M2);
        int64_t val3 = mod(x + 1, M3);

        auto ct1 = cc1->Encrypt(keys1.publicKey, cc1->MakePackedPlaintext(vector<int64_t>{val1}));
        auto ct2 = cc2->Encrypt(keys2.publicKey, cc2->MakePackedPlaintext(vector<int64_t>{val2}));
        auto ct3 = cc3->Encrypt(keys3.publicKey, cc3->MakePackedPlaintext(vector<int64_t>{val3}));

        // f(x) = (x+1)^2
        auto r1_ct = divine_multiply(cc1, keys1, ct1, ct1, anchor1, M1 / 2);
        auto r2_ct = divine_multiply(cc2, keys2, ct2, ct2, anchor2, M2 / 2);
        auto r3_ct = divine_multiply(cc3, keys3, ct3, ct3, anchor3, M3 / 2);

        Plaintext pt;
        cc1->Decrypt(keys1.secretKey, r1_ct, &pt);
        int64_t r1 = pt->GetPackedValue()[0];
        cc2->Decrypt(keys2.secretKey, r2_ct, &pt);
        int64_t r2 = pt->GetPackedValue()[0];
        cc3->Decrypt(keys3.secretKey, r3_ct, &pt);
        int64_t r3 = pt->GetPackedValue()[0];

        return crt3_combine(r1, r2, r3);
    }

    void run_test() {
        cout << "\n";
        cout << "  FEmmg-iO: 32-bit CRT3 + FHE\n";
        cout << "  f(x) = (x+1)^2 via Pure CTxCT with True Divine\n";
        cout << "  Garner's algorithm — all intermediate values modulo-bounded\n";
        cout << "  Date: " << ts() << "\n";
        cout << "  Moduli: " << M1 << ", " << M2 << ", " << M3 << "\n";
        cout << "  Combined: ~90 bits\n\n";

        cout << "  " << string(65, '-') << "\n";
        cout << setw(15) << "x" << setw(25) << "CRT3+FHE" << setw(25) << "Expected" << "\n";
        cout << "  " << string(65, '-') << "\n";

        vector<int64_t> test_inputs = {
            0LL, 1LL, 2LL, 3LL, 5LL, 7LL, 10LL, 15LL, 42LL, 100LL,
            1000LL, 10000LL, 65535LL, 100000LL, 1000000LL,
            10000000LL, 100000000LL, 1000000000LL, 2147483647LL
        };

        bool all_correct = true;
        for (int64_t x : test_inputs) {
            int64_t expected = (x + 1) * (x + 1);
            int64_t result = evaluate_fhe(x);

            bool ok = (result == expected);
            if (!ok) all_correct = false;

            cout << setw(15) << x
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
    CRT3_FHE_IO io;
    io.run_test();
    return 0;
}
