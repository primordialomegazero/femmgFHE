// ============================================
// φ-VERIFY DEEP
// Mas malalim na verification:
// - Random values
// - Random operations
// - Iba't ibang sequence length
// - Lahat ng ops: +, -, ×, ÷
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <random>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-VERIFY DEEP\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 4 slots)\n\n";

    auto encrypt_sync = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        
        vector<double> v(4, 0.0);
        v[0] = x * PHI;
        v[1] = x * PHI_INV;
        v[2] = log_phi_x;
        v[3] = x;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_sync = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) vals[i] = results[i].real();
        return vals;
    };

    // ============================================
    // VERIFICATION 1: IBA'T IBANG VALUES
    // ============================================

    cout << "========================================\n";
    cout << "  VERIFICATION 1: IBA'T IBANG VALUES\n";
    cout << "========================================\n\n";

    mt19937 rng(42);
    uniform_real_distribution<double> dist(1.0, 20.0);

    int total_tests = 0;
    int passed = 0;

    for (int test = 0; test < 50; test++) {
        double a = dist(rng);
        double b = dist(rng);
        char op = "+-×÷"[test % 4];
        
        auto ct_a = encrypt_sync(a);
        auto ct_b = encrypt_sync(b);
        auto ct_result = (op == '+') ? cc->EvalAdd(ct_a, ct_b) :
                         (op == '-') ? cc->EvalSub(ct_a, ct_b) :
                         (op == '×') ? cc->EvalAdd(ct_a, ct_b) :  // × = EvalAdd sa log
                         cc->EvalSub(ct_a, ct_b);                  // ÷ = EvalSub sa log
        
        auto vals = decrypt_sync(ct_result);
        
        double expected;
        double actual;
        
        if (op == '+' || op == '-') {
            expected = (op == '+') ? a + b : a - b;
            actual = vals[3];
        } else {
            expected = (op == '×') ? a * b : a / b;
            actual = pow(PHI, vals[2]);
        }
        
        total_tests++;
        bool match = abs(actual - expected) < 0.5;
        passed += match;
        
        if (!match) {
            cout << "  ❌ Test " << test << ": " << a << " " << op << " " << b
                 << " = " << actual << " (expected: " << expected << ")\n";
        }
    }

    cout << "  Passed: " << passed << "/" << total_tests << "\n\n";

    // ============================================
    // VERIFICATION 2: LONGER CHAINS
    // ============================================

    cout << "========================================\n";
    cout << "  VERIFICATION 2: LONGER CHAINS\n";
    cout << "========================================\n\n";

    for (int N : {10, 50, 100, 500, 1000}) {
        auto ct_state = encrypt_sync(2.0);
        double expected_norm = 2.0;
        double expected_log = log(2.0) / LN_PHI;
        
        for (int i = 0; i < N; i++) {
            int op_idx = i % 4;
            switch (op_idx) {
                case 0: ct_state = cc->EvalAdd(ct_state, encrypt_sync(2.0)); expected_norm += 2.0; expected_log += log(2.0)/LN_PHI; break;
                case 1: ct_state = cc->EvalSub(ct_state, encrypt_sync(1.0)); expected_norm -= 1.0; expected_log -= log(1.0)/LN_PHI; break;
                case 2: ct_state = cc->EvalAdd(ct_state, encrypt_sync(3.0)); expected_norm += 3.0; expected_log += log(3.0)/LN_PHI; break;
                case 3: ct_state = cc->EvalSub(ct_state, encrypt_sync(2.0)); expected_norm -= 2.0; expected_log -= log(2.0)/LN_PHI; break;
            }
        }
        
        auto vals = decrypt_sync(ct_state);
        
        bool norm_match = abs(vals[3] - expected_norm) < 1.0;
        bool log_match = abs(vals[2] - expected_log) < 1.0;
        
        cout << "  N=" << N << ": "
             << "norm=" << (norm_match ? "✅" : "❌")
             << " log=" << (log_match ? "✅" : "❌")
             << " (norm: " << vals[3] << " vs " << expected_norm
             << ", log: " << vals[2] << " vs " << expected_log << ")\n";
    }

    cout << "\n========================================\n";
    cout << "  VERIFY DEEP COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
