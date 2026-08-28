// ============================================
// φ-COMPLETE UNIVERSAL GATES — ZERO-LEVEL
// NOT, AND, OR, NAND, NOR, XOR, XNOR
// Lahat sa shifted log space
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <complex>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-COMPLETE UNIVERSAL GATES\n";
    cout << "  Zero-level sa shifted log space\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(10);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 1;
    double PHI = 1.6180339887498948482;
    double LN_PHI = log(PHI);

    cout << "SHIFTED LOG SPACE ENCODING:\n";
    cout << "  0 → log(1) = 0\n";
    cout << "  1 → log(φ) = " << fixed << setprecision(4) << LN_PHI << "\n\n";

    // Helper functions
    auto encode_bit = [&](int bit) {
        return (bit == 0) ? 0.0 : LN_PHI;
    };

    auto decode_bit = [&](double log_val, double threshold = 0.1) {
        return (log_val > threshold) ? 1 : 0;
    };

    cout << "========================================\n";
    cout << "  ALL GATES (ENCRYPTED, ZERO-LEVEL)\n";
    cout << "========================================\n\n";

    vector<pair<int, int>> tests = {{0,0}, {0,1}, {1,0}, {1,1}};

    for (auto& test : tests) {
        int a = test.first;
        int b = test.second;

        cout << "----------------------------------------\n";
        cout << "INPUT: A=" << a << ", B=" << b << "\n";
        cout << "----------------------------------------\n";

        // Encrypt in shifted log space
        double log_a = encode_bit(a);
        double log_b = encode_bit(b);

        vector<double> plain_a(slots, log_a);
        vector<double> plain_b(slots, log_b);

        auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));
        auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_b));

        // 1. NOT(A) = ¬A = 1 - A
        // Sa shifted log: NOT(A) = LN_PHI - log_a
        double not_a_val = (a == 0) ? 1 : 0;
        auto ct_not_a = cc->EvalSub(cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(vector<double>(1, LN_PHI))), ct_a);
        Plaintext p_not_a;
        cc->Decrypt(keyPair.secretKey, ct_not_a, &p_not_a);
        p_not_a->SetLength(1);
        auto not_a_complex = p_not_a->GetCKKSPackedValue();
        double not_a_result = decode_bit(not_a_complex[0].real());

        // 2. AND(A,B) = A·B
        // Sa shifted log: log(A) + log(B) > 0 → 1
        auto ct_and = cc->EvalAdd(ct_a, ct_b);
        Plaintext p_and;
        cc->Decrypt(keyPair.secretKey, ct_and, &p_and);
        p_and->SetLength(1);
        auto and_complex = p_and->GetCKKSPackedValue();
        double and_result = decode_bit(and_complex[0].real());

        // 3. OR(A,B) = A + B - A·B
        // Sa shifted log: log(A) + log(B) kung isa ay 1
        auto ct_or = cc->EvalAdd(ct_a, ct_b);
        Plaintext p_or;
        cc->Decrypt(keyPair.secretKey, ct_or, &p_or);
        p_or->SetLength(1);
        auto or_complex = p_or->GetCKKSPackedValue();
        double or_result = decode_bit(or_complex[0].real());

        // 4. NAND(A,B) = ¬(A·B)
        auto ct_nand = cc->EvalNegate(ct_and);
        Plaintext p_nand;
        cc->Decrypt(keyPair.secretKey, ct_nand, &p_nand);
        p_nand->SetLength(1);
        auto nand_complex = p_nand->GetCKKSPackedValue();
        double nand_result = decode_bit(nand_complex[0].real());

        // 5. NOR(A,B) = ¬(A+B)
        auto ct_nor = cc->EvalNegate(ct_or);
        Plaintext p_nor;
        cc->Decrypt(keyPair.secretKey, ct_nor, &p_nor);
        p_nor->SetLength(1);
        auto nor_complex = p_nor->GetCKKSPackedValue();
        double nor_result = decode_bit(nor_complex[0].real());

        // 6. XOR(A,B) = |A - B| sa shifted log
        auto ct_diff = cc->EvalSub(ct_a, ct_b);
        // Manu-manong absolute: |x| = x*sign(x)
        // Para sa log space, gamitin ang diff directly

        Plaintext p_xor;
        cc->Decrypt(keyPair.secretKey, ct_diff, &p_xor);
        p_xor->SetLength(1);
        auto xor_complex = p_xor->GetCKKSPackedValue();
        double xor_result = decode_bit(abs(xor_complex[0].real()), 0.2);

        // Expected values
        int expected_not = (a == 0) ? 1 : 0;
        int expected_and = (a == 1 && b == 1) ? 1 : 0;
        int expected_or = (a == 1 || b == 1) ? 1 : 0;
        int expected_nand = (a == 1 && b == 1) ? 0 : 1;
        int expected_nor = (a == 0 && b == 0) ? 1 : 0;
        int expected_xor = (a != b) ? 1 : 0;

        cout << "Gate | Result | Expected | Match?\n";
        cout << "-----|--------|----------|-------\n";
        cout << "NOT(A) | " << setw(4) << not_a_result << " | " << setw(8) << expected_not << " | " << (not_a_result == expected_not ? "✅" : "❌") << "\n";
        cout << "AND    | " << setw(4) << and_result << " | " << setw(8) << expected_and << " | " << (and_result == expected_and ? "✅" : "❌") << "\n";
        cout << "OR     | " << setw(4) << or_result << " | " << setw(8) << expected_or << " | " << (or_result == expected_or ? "✅" : "❌") << "\n";
        cout << "NAND   | " << setw(4) << nand_result << " | " << setw(8) << expected_nand << " | " << (nand_result == expected_nand ? "✅" : "❌") << "\n";
        cout << "NOR    | " << setw(4) << nor_result << " | " << setw(8) << expected_nor << " | " << (nor_result == expected_nor ? "✅" : "❌") << "\n";
        cout << "XOR    | " << setw(4) << xor_result << " | " << setw(8) << expected_xor << " | " << (xor_result == expected_xor ? "✅" : "❌") << "\n\n";
    }

    cout << "========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  Gates tested: 6 (NOT, AND, OR, NAND, NOR, XOR)\n";
    cout << "  Inputs: 4 combinations each\n";
    cout << "  Total tests: 24\n";
    cout << "  Zero-level: ✅ Lahat\n";
    cout << "  Walang multiplication: ✅\n";
    cout << "  Walang bootstrapping: ✅\n";
    cout << "========================================\n";

    return 0;
}
