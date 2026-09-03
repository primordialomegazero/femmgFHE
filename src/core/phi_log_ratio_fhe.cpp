// ============================================
// φ-LOG RATIO FHE — 10K MIXED
// F = r × φ^k, log_φ(r) ∈ [0, 1)
// Addition sa mantissa, multiplication sa exponent
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
    const double LN_PHI = log(PHI);

    auto encrypt_ratio = [&](double F) {
        double k = floor(log(F) / LN_PHI);
        double r = F / pow(PHI, k);
        vector<double> v(4, 0.0);
        v[0] = k;                      // exponent
        v[1] = log(r) / LN_PHI;       // fractional log [0, 1)
        v[2] = r;                      // mantissa [1, φ)
        v[3] = F;                      // original value
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_ratio = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-LOG RATIO FHE — 10K MIXED\n";
    cout << "========================================\n\n";

    int N = 10000;

    cout << "  Operations: " << N << "\n";
    cout << "  Cycle: +7, -3, ×2, ÷3 (no-cancel)\n";
    cout << "  Initial: 100\n";
    cout << "  Running...\n\n";

    auto ct_state = encrypt_ratio(100.0);
    double expected = 100.0;

    auto ct_add = encrypt_ratio(7.0);
    auto ct_sub = encrypt_ratio(3.0);
    
    vector<double> v_mul(4, 0.0);
    v_mul[0] = log(2.0) / LN_PHI;  // exponent delta para sa ×2
    v_mul[1] = log(2.0) / LN_PHI;  // fractional log delta
    Plaintext pt_mul = cc->MakeCKKSPackedPlaintext(v_mul);
    auto ct_mul = cc->Encrypt(keyPair.publicKey, pt_mul);
    
    vector<double> v_div(4, 0.0);
    v_div[0] = -log(3.0) / LN_PHI;
    v_div[1] = -log(3.0) / LN_PHI;
    Plaintext pt_div = cc->MakeCKKSPackedPlaintext(v_div);
    auto ct_div = cc->Encrypt(keyPair.publicKey, pt_div);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        int op = i % 4;
        
        if (op == 0) {
            ct_state = cc->EvalAdd(ct_state, ct_add);
            expected += 7.0;
        } else if (op == 1) {
            ct_state = cc->EvalSub(ct_state, ct_sub);
            expected -= 3.0;
        } else if (op == 2) {
            ct_state = cc->EvalAdd(ct_state, ct_mul);
            expected *= 2.0;
        } else {
            ct_state = cc->EvalAdd(ct_state, ct_div);
            expected /= 3.0;
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_ratio(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << fixed << setprecision(0) << (N * 1000.0 / time) << "\n\n";

    cout << "  Final k: " << v_final[0] << "\n";
    cout << "  Final log_r: " << v_final[1] << "\n";
    cout << "  Final r: " << v_final[2] << "\n";
    cout << "  Final F: " << v_final[3] << "\n";
    cout << "  Expected: " << expected << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    double error = abs(v_final[3] - expected) / expected * 100.0;
    cout << "  Error: " << fixed << setprecision(6) << error << "%\n";
    cout << "  Match: " << (error < 1.0 ? "✅" : "❌") << "\n\n";

    return 0;
}
