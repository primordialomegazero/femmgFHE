// ============================================
// φ-NAND CHECK
// I-verify ang NAND gates sa φ-duality space
//
// NAND: 0→1, 1→0 (inverted)
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
    cout << "  φ-NAND CHECK\n";
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

    // ============================================
    // ENCODING PARA SA BOOLEAN
    // Slot 0: x×φ (normal value)
    // Slot 1: x×φ⁻¹
    // Slot 2: log_φ(x) (para sa multiplication)
    // Slot 3: x (boolean 0 o 1)
    // ============================================

    auto encrypt_bool = [&](int bit) {
        double x = (bit == 0) ? -2.0 : 2.0;  // φ² space: 0→-2, 1→+2
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

    cout << "========================================\n";
    cout << "  NAND GATE TRUTH TABLE\n";
    cout << "========================================\n\n";

    cout << "  A B | NAND | Expected | Match?\n";
    cout << "  ----|------|----------|--------\n";

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_bool(A);
            auto ct_b = encrypt_bool(B);
            
            // NAND = NOT(A AND B) = Negate(Add(A, B)) sa φ² space
            auto ct_nand = cc->EvalNegate(cc->EvalAdd(ct_a, ct_b));
            auto nand_vals = decrypt_bool(ct_nand);
            
            double result = nand_vals[3];  // Slot 3 — normal value
            int decoded = (result >= -0.01) ? 1 : 0;  // φ²: ≥-0.01 → 1
            int expected = !(A && B);
            bool match = (decoded == expected);
            
            cout << "  " << A << " " << B << " | "
                 << setw(4) << decoded << " | "
                 << setw(8) << expected << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // ALL GATES CHECK
    // ============================================

    cout << "\n========================================\n";
    cout << "  ALL GATES CHECK\n";
    cout << "========================================\n\n";

    cout << "  A B | NAND | AND | OR | XOR\n";
    cout << "  ----|------|-----|----|----\n";

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_bool(A);
            auto ct_b = encrypt_bool(B);
            
            // NAND
            auto ct_nand = cc->EvalNegate(cc->EvalAdd(ct_a, ct_b));
            auto nand_vals = decrypt_bool(ct_nand);
            int nand = (nand_vals[3] >= -0.01) ? 1 : 0;
            
            // AND
            auto ct_and = cc->EvalAdd(ct_a, ct_b);
            auto and_vals = decrypt_bool(ct_and);
            int and_r = (and_vals[3] > 1.5) ? 1 : 0;
            
            // OR
            auto ct_or = cc->EvalAdd(ct_a, ct_b);
            auto or_vals = decrypt_bool(ct_or);
            int or_r = (or_vals[3] > 0.5) ? 1 : 0;
            
            // XOR
            auto ct_xor = cc->EvalSub(ct_a, ct_b);
            auto xor_vals = decrypt_bool(ct_xor);
            int xor_r = (abs(xor_vals[3]) > 0.5) ? 1 : 0;
            
            cout << "  " << A << " " << B << " | "
                 << setw(4) << nand << " | "
                 << setw(3) << and_r << " | "
                 << setw(2) << or_r << " | "
                 << setw(3) << xor_r << "\n";
        }
    }

    cout << "\n========================================\n";
    cout << "  NAND CHECK COMPLETE\n";
    cout << "========================================\n\n";

    return 0;
}
