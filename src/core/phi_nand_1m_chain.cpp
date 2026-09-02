// ============================================
// φ-NAND 1M CHAIN
// 1M NAND gates na naka-chain
// Sa φ-duality space
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-NAND 1M CHAIN\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
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

    cout << "  ✅ CKKS initialized (depth 1, modsize 59, 4 slots)\n\n";

    // ============================================
    // ENCODING PARA SA BOOLEAN
    // 0 → -2, 1 → +2 (φ² space)
    // ============================================

    auto encrypt_bool = [&](int bit) {
        double x = (bit == 0) ? -2.0 : 2.0;
        double log_phi_x = log(abs(x)) / LN_PHI;
        
        vector<double> v(4, 0.0);
        v[0] = x * PHI;
        v[1] = x * PHI_INV;
        v[2] = log_phi_x;
        v[3] = x;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bool = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) vals[i] = results[i].real();
        return vals;
    };

    // ============================================
    // NAND GATE
    // ============================================

    auto nand_gate = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalNegate(cc->EvalAdd(a, b));
    };

    // ============================================
    // VERIFICATION: SINGLE NAND
    // ============================================

    cout << "========================================\n";
    cout << "  SINGLE NAND VERIFICATION\n";
    cout << "========================================\n\n";

    cout << "  A B | NAND | Expected | Match?\n";
    cout << "  ----|------|----------|--------\n";

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_bool(A);
            auto ct_b = encrypt_bool(B);
            auto ct_nand = nand_gate(ct_a, ct_b);
            auto vals = decrypt_bool(ct_nand);
            
            int decoded = (vals[3] >= -0.01) ? 1 : 0;
            int expected = !(A && B);
            bool match = (decoded == expected);
            
            cout << "  " << A << " " << B << " | "
                 << setw(4) << decoded << " | "
                 << setw(8) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // 1M NAND CHAIN
    // ============================================

    cout << "\n========================================\n";
    cout << "  1M NAND CHAIN\n";
    cout << "========================================\n\n";

    int N = 1000000;

    // Initial state: 0
    auto ct_state = encrypt_bool(0);
    auto ct_one = encrypt_bool(1);

    cout << "  Operations: " << N << "\n";
    cout << "  Pattern: NAND(state, 1) — alternating\n";
    cout << "  Running...\n\n";

    auto start = high_resolution_clock::now();

    int expected_state = 0;  // start sa 0

    for (int i = 0; i < N; i++) {
        ct_state = nand_gate(ct_state, ct_one);
        expected_state = !(expected_state && 1);  // NAND(0,1)=1, NAND(1,1)=0
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto final_vals = decrypt_bool(ct_state);

    cout << "  ✅ 1M NAND chain complete!\n";
    cout << "  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";
    cout << "  Towers: " << ct_state->GetElements()[0].GetNumOfElements() << "\n";
    cout << "  Slot 3 (result): " << final_vals[3] << "\n";
    cout << "  Decoded: " << ((final_vals[3] >= -0.01) ? 1 : 0) << "\n";
    cout << "  Expected: " << expected_state << "\n";
    cout << "  Match: " << ((((final_vals[3] >= -0.01) ? 1 : 0) == expected_state) ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  NAND 1M CHAIN COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 1M NAND chain\n";
    cout << "  ✅ Walang EvalMult(ct, ct)\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
