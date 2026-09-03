// ============================================
// φ-SPACE DEBUG — PRINT ACTUAL VALUES
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
    cout << "  φ-SPACE DEBUG\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;

    cout << "  ✅ CKKS initialized (depth 0!)\n\n";

    auto encrypt_dual = [&](int bit) {
        vector<double> v(8, 0.0);
        v[0] = (bit == 0) ? -2.0 : 2.0;  // Log space
        v[4] = (bit == 0) ? 0.0 : 1.0;   // Normal space
        for (int i = 1; i < 4; i++) v[i] = v[0] * pow(PHI, i);
        for (int i = 5; i < 8; i++) v[i] = v[4] * pow(PHI, i-4);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_raw = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        double log_val = result_pt->GetCKKSPackedValue()[0].real();
        double normal_val = result_pt->GetCKKSPackedValue()[4].real();
        return make_pair(log_val, normal_val);
    };

    // ============================================
    // DEBUG: XOR VALUES
    // ============================================

    cout << "  XOR DEBUG:\n";
    cout << "  A B | Log Val | Normal Val | Decode(Log) | Decode(Normal)\n";
    cout << "  ----|---------|------------|-------------|---------------\n";

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_dual(A);
            auto ct_b = encrypt_dual(B);
            
            // XOR: |a - b| sa normal space
            auto diff = cc->EvalAdd(ct_a, cc->EvalNegate(ct_b));
            
            auto [log_val, normal_val] = decrypt_raw(diff);
            
            int log_bit = (log_val >= -0.01) ? 1 : 0;
            int normal_bit = (normal_val > 0.5) ? 1 : 0;
            
            cout << "  " << A << " " << B << " | "
                 << setw(7) << fixed << setprecision(4) << log_val << " | "
                 << setw(10) << normal_val << " | "
                 << setw(11) << log_bit << " | "
                 << setw(13) << normal_bit << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // DEBUG: AND VALUES
    // ============================================

    cout << "  AND DEBUG:\n";
    cout << "  A B | Log Val | Normal Val | Decode(Log) | Decode(Normal)\n";
    cout << "  ----|---------|------------|-------------|---------------\n";

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_dual(A);
            auto ct_b = encrypt_dual(B);
            
            // AND: a×b sa normal space = add sa log space
            auto sum = cc->EvalAdd(ct_a, ct_b);
            
            auto [log_val, normal_val] = decrypt_raw(sum);
            
            int log_bit = (log_val >= -0.01) ? 1 : 0;
            int normal_bit = (normal_val > 0.5) ? 1 : 0;
            
            cout << "  " << A << " " << B << " | "
                 << setw(7) << fixed << setprecision(4) << log_val << " | "
                 << setw(10) << normal_val << " | "
                 << setw(11) << log_bit << " | "
                 << setw(13) << normal_bit << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // DEBUG: OR VALUES
    // ============================================

    cout << "  OR DEBUG:\n";
    cout << "  A B | Log Val | Normal Val | Decode(Log) | Decode(Normal)\n";
    cout << "  ----|---------|------------|-------------|---------------\n";

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_dual(A);
            auto ct_b = encrypt_dual(B);
            
            // OR: NAND(NOT(A), NOT(B))
            auto not_a = cc->EvalNegate(ct_a);
            auto not_b = cc->EvalNegate(ct_b);
            auto or_ct = cc->EvalNegate(cc->EvalAdd(not_a, not_b));
            
            auto [log_val, normal_val] = decrypt_raw(or_ct);
            
            int log_bit = (log_val >= -0.01) ? 1 : 0;
            int normal_bit = (normal_val > 0.5) ? 1 : 0;
            
            cout << "  " << A << " " << B << " | "
                 << setw(7) << fixed << setprecision(4) << log_val << " | "
                 << setw(10) << normal_val << " | "
                 << setw(11) << log_bit << " | "
                 << setw(13) << normal_bit << "\n";
        }
    }

    cout << "\n";

    return 0;
}
