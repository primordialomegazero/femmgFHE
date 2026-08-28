// ============================================
// DEBUG: LAHAT NG GATES — PER-TEST ANALYSIS
// Alamin kung alin ang nag-fail at bakit
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  DEBUG: LAHAT NG GATES\n";
    cout << "  Per-test analysis\n";
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

    // Lucas encoding
    double L_2 = 3.0;  // 0
    double L_4 = 7.0;  // 1

    cout << "ENCODING:\n";
    cout << "  0 → L_2 = " << L_2 << "\n";
    cout << "  1 → L_4 = " << L_4 << "\n\n";

    // Test all combinations
    vector<pair<int, int>> tests = {{0,0}, {0,1}, {1,0}, {1,1}};

    cout << "========================================\n";
    cout << "  NOT GATE — NOT(a) = 7 - a\n";
    cout << "========================================\n\n";

    cout << "A | Enc(A) | NOT Enc | Decrypt | Expected | Match?\n";
    cout << "--|--------|---------|---------|----------|-------\n";

    for (auto& test : tests) {
        int a = test.first;
        int expected_not = (a == 0) ? 1 : 0;

        double enc_a = (a == 0) ? L_2 : L_4;
        double expected_enc = (expected_not == 0) ? L_2 : L_4;

        vector<double> plain_a(slots, enc_a);
        auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));

        vector<double> plain_7(slots, L_4);
        auto ct_7 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_7));

        auto ct_not = cc->EvalSub(ct_7, ct_a);

        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_not, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double result = result_complex[0].real();

        cout << a << " | "
             << setw(6) << fixed << setprecision(1) << enc_a << " | "
             << setw(7) << fixed << setprecision(1) << L_4 - enc_a << " | "
             << setw(7) << fixed << setprecision(1) << result << " | "
             << setw(8) << fixed << setprecision(1) << expected_enc << " | "
             << (abs(result - expected_enc) < 0.1 ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  AND GATE — AND(a,b) = (a+b-3)/2\n";
    cout << "========================================\n\n";

    cout << "A B | Enc(A) | Enc(B) | AND Enc | Decrypt | Expected | Match?\n";
    cout << "----|--------|--------|---------|---------|----------|-------\n";

    for (auto& test : tests) {
        int a = test.first;
        int b = test.second;
        int expected_and = (a == 1 && b == 1) ? 1 : 0;

        double enc_a = (a == 0) ? L_2 : L_4;
        double enc_b = (b == 0) ? L_2 : L_4;
        double expected_enc = (expected_and == 0) ? L_2 : L_4;

        vector<double> plain_a(slots, enc_a);
        vector<double> plain_b(slots, enc_b);
        auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));
        auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_b));

        vector<double> plain_3(slots, L_2);
        auto ct_3 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_3));

        // AND = (a + b - 3) / 2
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        auto ct_diff = cc->EvalSub(ct_sum, ct_3);

        // Division by 2 = multiply by 0.5
        vector<double> plain_half(slots, 0.5);
        auto ct_half = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_half));
        auto ct_and = cc->EvalMult(ct_diff, ct_half);

        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_and, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double result = result_complex[0].real();

        cout << a << " " << b << " | "
             << setw(6) << fixed << setprecision(1) << enc_a << " | "
             << setw(6) << fixed << setprecision(1) << enc_b << " | "
             << setw(7) << fixed << setprecision(1) << (enc_a + enc_b - 3) / 2 << " | "
             << setw(7) << fixed << setprecision(1) << result << " | "
             << setw(8) << fixed << setprecision(1) << expected_enc << " | "
             << (abs(result - expected_enc) < 0.1 ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  OR GATE — OR(a,b) = (a+b+3)/2\n";
    cout << "========================================\n\n";

    cout << "A B | Enc(A) | Enc(B) | OR Enc | Decrypt | Expected | Match?\n";
    cout << "----|--------|--------|--------|---------|----------|-------\n";

    for (auto& test : tests) {
        int a = test.first;
        int b = test.second;
        int expected_or = (a == 1 || b == 1) ? 1 : 0;

        double enc_a = (a == 0) ? L_2 : L_4;
        double enc_b = (b == 0) ? L_2 : L_4;
        double expected_enc = (expected_or == 0) ? L_2 : L_4;

        vector<double> plain_a(slots, enc_a);
        vector<double> plain_b(slots, enc_b);
        auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));
        auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_b));

        vector<double> plain_3(slots, L_2);
        auto ct_3 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_3));

        // OR = (a + b + 3) / 2
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        auto ct_diff = cc->EvalAdd(ct_sum, ct_3);

        vector<double> plain_half(slots, 0.5);
        auto ct_half = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_half));
        auto ct_or = cc->EvalMult(ct_diff, ct_half);

        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_or, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double result = result_complex[0].real();

        cout << a << " " << b << " | "
             << setw(6) << fixed << setprecision(1) << enc_a << " | "
             << setw(6) << fixed << setprecision(1) << enc_b << " | "
             << setw(7) << fixed << setprecision(1) << (enc_a + enc_b + 3) / 2 << " | "
             << setw(7) << fixed << setprecision(1) << result << " | "
             << setw(8) << fixed << setprecision(1) << expected_enc << " | "
             << (abs(result - expected_enc) < 0.1 ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  XOR GATE — XOR(a,b) = a + b - 7\n";
    cout << "========================================\n\n";

    cout << "A B | Enc(A) | Enc(B) | XOR Enc | Decrypt | Expected | Match?\n";
    cout << "----|--------|--------|---------|---------|----------|-------\n";

    for (auto& test : tests) {
        int a = test.first;
        int b = test.second;
        int expected_xor = (a != b) ? 1 : 0;

        double enc_a = (a == 0) ? L_2 : L_4;
        double enc_b = (b == 0) ? L_2 : L_4;
        double expected_enc = (expected_xor == 0) ? L_2 : L_4;

        vector<double> plain_a(slots, enc_a);
        vector<double> plain_b(slots, enc_b);
        auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));
        auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_b));

        vector<double> plain_7(slots, L_4);
        auto ct_7 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_7));

        // XOR = a + b - 7
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        auto ct_xor = cc->EvalSub(ct_sum, ct_7);

        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_xor, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double result = result_complex[0].real();

        cout << a << " " << b << " | "
             << setw(6) << fixed << setprecision(1) << enc_a << " | "
             << setw(6) << fixed << setprecision(1) << enc_b << " | "
             << setw(7) << fixed << setprecision(1) << enc_a + enc_b - 7 << " | "
             << setw(7) << fixed << setprecision(1) << result << " | "
             << setw(8) << fixed << setprecision(1) << expected_enc << " | "
             << (abs(result - expected_enc) < 0.1 ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  XNOR GATE — XNOR(a,b) = 7 - XOR(a,b)\n";
    cout << "========================================\n\n";

    cout << "A B | XOR Enc | XNOR Enc | Decrypt | Expected | Match?\n";
    cout << "----|---------|----------|---------|----------|-------\n";

    for (auto& test : tests) {
        int a = test.first;
        int b = test.second;
        int expected_xnor = (a == b) ? 1 : 0;

        double enc_a = (a == 0) ? L_2 : L_4;
        double enc_b = (b == 0) ? L_2 : L_4;
        double expected_enc = (expected_xnor == 0) ? L_2 : L_4;

        vector<double> plain_a(slots, enc_a);
        vector<double> plain_b(slots, enc_b);
        auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));
        auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_b));

        vector<double> plain_7(slots, L_4);
        auto ct_7 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_7));

        // XOR = a + b - 7
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        auto ct_xor = cc->EvalSub(ct_sum, ct_7);

        // XNOR = 7 - XOR
        auto ct_xnor = cc->EvalSub(ct_7, ct_xor);

        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_xnor, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double result = result_complex[0].real();

        cout << a << " " << b << " | "
             << setw(7) << fixed << setprecision(1) << enc_a + enc_b - 7 << " | "
             << setw(8) << fixed << setprecision(1) << 7 - (enc_a + enc_b - 7) << " | "
             << setw(7) << fixed << setprecision(1) << result << " | "
             << setw(8) << fixed << setprecision(1) << expected_enc << " | "
             << (abs(result - expected_enc) < 0.1 ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  NAND GATE — NAND(a,b) = 7 - AND(a,b)\n";
    cout << "========================================\n\n";

    cout << "A B | AND Enc | NAND Enc | Decrypt | Expected | Match?\n";
    cout << "----|---------|----------|---------|----------|-------\n";

    for (auto& test : tests) {
        int a = test.first;
        int b = test.second;
        int expected_nand = (a == 1 && b == 1) ? 0 : 1;

        double enc_a = (a == 0) ? L_2 : L_4;
        double enc_b = (b == 0) ? L_2 : L_4;
        double expected_enc = (expected_nand == 0) ? L_2 : L_4;

        vector<double> plain_a(slots, enc_a);
        vector<double> plain_b(slots, enc_b);
        auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));
        auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_b));

        vector<double> plain_3(slots, L_2);
        auto ct_3 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_3));
        vector<double> plain_7(slots, L_4);
        auto ct_7 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_7));
        vector<double> plain_half(slots, 0.5);
        auto ct_half = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_half));

        // AND = (a + b - 3) / 2
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        auto ct_diff = cc->EvalSub(ct_sum, ct_3);
        auto ct_and = cc->EvalMult(ct_diff, ct_half);

        // NAND = 7 - AND
        auto ct_nand = cc->EvalSub(ct_7, ct_and);

        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_nand, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double result = result_complex[0].real();

        cout << a << " " << b << " | "
             << setw(7) << fixed << setprecision(1) << (enc_a + enc_b - 3) / 2 << " | "
             << setw(8) << fixed << setprecision(1) << 7 - (enc_a + enc_b - 3) / 2 << " | "
             << setw(7) << fixed << setprecision(1) << result << " | "
             << setw(8) << fixed << setprecision(1) << expected_enc << " | "
             << (abs(result - expected_enc) < 0.1 ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  NOR GATE — NOR(a,b) = 7 - OR(a,b)\n";
    cout << "========================================\n\n";

    cout << "A B | OR Enc | NOR Enc | Decrypt | Expected | Match?\n";
    cout << "----|--------|---------|---------|----------|-------\n";

    for (auto& test : tests) {
        int a = test.first;
        int b = test.second;
        int expected_nor = (a == 0 && b == 0) ? 1 : 0;

        double enc_a = (a == 0) ? L_2 : L_4;
        double enc_b = (b == 0) ? L_2 : L_4;
        double expected_enc = (expected_nor == 0) ? L_2 : L_4;

        vector<double> plain_a(slots, enc_a);
        vector<double> plain_b(slots, enc_b);
        auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));
        auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_b));

        vector<double> plain_3(slots, L_2);
        auto ct_3 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_3));
        vector<double> plain_7(slots, L_4);
        auto ct_7 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_7));
        vector<double> plain_half(slots, 0.5);
        auto ct_half = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_half));

        // OR = (a + b + 3) / 2
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        auto ct_diff = cc->EvalAdd(ct_sum, ct_3);
        auto ct_or = cc->EvalMult(ct_diff, ct_half);

        // NOR = 7 - OR
        auto ct_nor = cc->EvalSub(ct_7, ct_or);

        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_nor, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();
        double result = result_complex[0].real();

        cout << a << " " << b << " | "
             << setw(7) << fixed << setprecision(1) << (enc_a + enc_b + 3) / 2 << " | "
             << setw(8) << fixed << setprecision(1) << 7 - (enc_a + enc_b + 3) / 2 << " | "
             << setw(7) << fixed << setprecision(1) << result << " | "
             << setw(8) << fixed << setprecision(1) << expected_enc << " | "
             << (abs(result - expected_enc) < 0.1 ? "✅" : "❌") << "\n";
    }

    return 0;
}
