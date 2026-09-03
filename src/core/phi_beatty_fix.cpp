// ============================================
// φ-BEATTY FIX — WALANG SLOT 3
// 3 slots lang: mod φ, grupo, log
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

    auto encrypt_state = [&](double F) {
        int q = (int)floor(F / PHI);
        int grupo = q % 2;
        vector<double> v(4, 0.0);
        v[0] = fmod(F, PHI);
        v[1] = grupo;
        v[2] = log(F) / LN_PHI;
        v[3] = 0;  // unused
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-BEATTY FIX — 100 OPS\n";
    cout << "========================================\n\n";

    int N = 100;

    auto ct_state = encrypt_state(100.0);
    double expected = 100.0;

    auto ct_add = encrypt_state(7.0);
    auto ct_sub = encrypt_state(3.0);
    
    vector<double> v_mul(4, 0.0);
    v_mul[2] = log(2.0) / LN_PHI;
    Plaintext pt_mul = cc->MakeCKKSPackedPlaintext(v_mul);
    auto ct_mul = cc->Encrypt(keyPair.publicKey, pt_mul);
    
    vector<double> v_div(4, 0.0);
    v_div[2] = -log(3.0) / LN_PHI;
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

    auto v_final = decrypt_state(ct_state);

    cout << "  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final mod φ: " << v_final[0] << "\n";
    cout << "  Final grupo: " << v_final[1] << "\n";
    cout << "  Final log: " << v_final[2] << "\n";
    cout << "  Expected log: " << log(expected) / LN_PHI << "\n\n";

    double log_error = abs(v_final[2] - log(expected) / LN_PHI);
    cout << "  Log Error: " << fixed << setprecision(6) << log_error << "\n";
    cout << "  Match: " << (log_error < 0.01 ? "✅" : "❌") << "\n\n";

    return 0;
}
