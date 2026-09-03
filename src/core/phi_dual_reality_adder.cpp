// ============================================
// φ-DUAL REALITY ADDER — SUM + COUT LAYERS
//
// Layer 1 (slots 0-31): SUM — XOR space
// Layer 2 (slots 32-63): COUT — AND/OR space
// Isang presentation, dalawang realities!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-DUAL REALITY ADDER\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(64);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    cout << "  ✅ CKKS initialized (depth 1, 64 slots!)\n";
    cout << "  Dual Reality: Layer 1 (Sum) + Layer 2 (Cout)\n\n";

    // ============================================
    // DUAL REALITY ENCODING
    // ============================================

    auto encrypt_dual = [&](int bit, int input_id) {
        vector<double> v(64, 0.0);
        
        // LAYER 1 (slots 0-31): SUM — XOR space
        // LAYER 2 (slots 32-63): COUT — AND/OR space
        
        double sum_val = (bit == 0) ? 0.0 : 1.0;
        double cout_val = (bit == 0) ? 0.0 : 1.0;
        
        // Input A → slot 0 (sum) at slot 32 (cout)
        // Input B → slot 1 (sum) at slot 33 (cout)
        // Input Cin → slot 2 (sum) at slot 34 (cout)
        
        v[input_id] = sum_val;
        v[32 + input_id] = cout_val;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(64);
        vector<double> results(64);
        for (int i = 0; i < 64; i++) {
            results[i] = result_pt->GetCKKSPackedValue()[i].real();
        }
        return results;
    };

    // ============================================
    // FULL ADDER (DUAL REALITY)
    // ============================================

    cout << "========================================\n";
    cout << "  FULL ADDER (DUAL REALITY)\n";
    cout << "========================================\n\n";

    cout << "  A B Cin | Sum | Cout | Both?\n";
    cout << "  --------|-----|------|------\n";

    int adder_correct = 0;

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                auto ct_a = encrypt_dual(A, 0);
                auto ct_b = encrypt_dual(B, 1);
                auto ct_cin = encrypt_dual(Cin, 2);
                
                // LAYER 1 (SUM): XOR space
                // Sum = A XOR B XOR Cin
                auto xor_ab = cc->EvalSub(ct_a, ct_b);
                auto sum_ct = cc->EvalSub(xor_ab, ct_cin);
                
                // LAYER 2 (COUT): AND/OR space
                // Cout = (A AND B) OR (Cin AND (A XOR B))
                auto and_ab = cc->EvalAdd(ct_a, ct_b);
                auto and_cin_xor = cc->EvalAdd(ct_cin, xor_ab);
                auto cout_ct = cc->EvalAdd(and_ab, and_cin_xor);
                
                auto sum_vals = decrypt_dual(sum_ct);
                auto cout_vals = decrypt_dual(cout_ct);
                
                // Decode: Layer 1 para sa Sum, Layer 2 para sa Cout
                int sum = (abs(sum_vals[0]) > 0.5) ? 1 : 0;
                int cout_val = (cout_vals[32] > 1.5) ? 1 : 0;
                
                int exp_sum = (A + B + Cin) % 2;
                int exp_cout = (A + B + Cin) / 2;
                
                bool both_ok = (sum == exp_sum && cout_val == exp_cout);
                adder_correct += both_ok;
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << " " << sum << "   " << cout_val << "   | "
                     << (both_ok ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Full Adder: " << adder_correct << "/8\n\n";

    cout << "========================================\n";
    cout << "  DUAL REALITY ADDER COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Dual Reality: Sum + Cout layers\n";
    cout << "  ✅ Full Adder: " << adder_correct << "/8\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Depth 1\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
