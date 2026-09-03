// ============================================
// φ-ARBITRARY TEST
// Subok ng arbitrary mixed operations
// Walang fixed sequence, walang fixed values
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <random>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-ARBITRARY TEST\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized\n\n";

    // Random generator
    mt19937 rng(42);
    uniform_real_distribution<double> val_dist(1.0, 20.0);
    uniform_int_distribution<int> op_dist(0, 3);

    auto encrypt_val = [&](double x) {
        double log_x = log(x) / LN_PHI;
        vector<double> v = {x, 0.0, log_x, 0.0};
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) vals[i] = results[i].real();
        return vals;
    };

    // ============================================
    // ARBITRARY SEQUENCE TEST
    // ============================================

    cout << "========================================\n";
    cout << "  ARBITRARY SEQUENCE (10 random ops)\n";
    cout << "========================================\n\n";

    double plain_result = 0;
    vector<double> encrypted_ops;
    vector<char> op_symbols;
    vector<double> operands;

    // Initial value
    double initial = val_dist(rng);
    plain_result = initial;
    auto ct_current = encrypt_val(initial);
    cout << "  Start: " << fixed << setprecision(2) << initial << "\n";

    for (int i = 0; i < 10; i++) {
        int op = op_dist(rng);
        double operand = val_dist(rng);
        operands.push_back(operand);
        auto ct_operand = encrypt_val(operand);

        if (op == 0) {  // Addition
            plain_result += operand;
            ct_current = cc->EvalAdd(ct_current, ct_operand);
            op_symbols.push_back('+');
        } else if (op == 1) {  // Subtraction
            plain_result -= operand;
            ct_current = cc->EvalSub(ct_current, ct_operand);
            op_symbols.push_back('-');
        } else if (op == 2) {  // Multiplication
            plain_result *= operand;
            // Log space: add sa Slot 2
            ct_current = cc->EvalAdd(ct_current, ct_operand);
            op_symbols.push_back('*');
        } else {  // Division
            plain_result /= operand;
            // Log space: sub sa Slot 2
            ct_current = cc->EvalSub(ct_current, ct_operand);
            op_symbols.push_back('/');
        }

        auto vals = decrypt_val(ct_current);
        cout << "  " << op_symbols[i] << " " << fixed << setprecision(2) 
             << operand << " → plain: " << plain_result 
             << " | enc[0]: " << vals[0] 
             << " | enc[2]: " << vals[2] << "\n";
    }

    auto final_vals = decrypt_val(ct_current);
    double enc_result = final_vals[0];

    cout << "\n  Plain result: " << fixed << setprecision(6) << plain_result << "\n";
    cout << "  Encrypted result: " << enc_result << "\n";
    cout << "  Error: " << fabs(plain_result - enc_result) << "\n\n";

    cout << "  Level: " << ct_current->GetLevel() << "\n";
    cout << "  Towers: " << ct_current->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
