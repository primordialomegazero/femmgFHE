// ============================================
// φ-HONEST MAX — WALANG DAYAAN
// Hayaan ang φ-structure na mag-decide
// Kung bumagsak, bumagsak
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

    auto encrypt_state = [&](double n_a, double n_b) {
        vector<double> v(4, 0.0);
        v[0] = n_a;
        v[1] = pow(PHI, n_a);
        v[2] = n_b;
        v[3] = pow(PHI, n_b);
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
    cout << "  φ-HONEST MAX — WALANG DAYAAN\n";
    cout << "========================================\n\n";

    int N = 100;

    cout << "  Operations: " << N << "\n";
    cout << "  State: (n_a, F_a, n_b, F_b)\n";
    cout << "  Walang hardcode, walang alternation\n";
    cout << "  Hayaan ang φ-structure na mag-decide\n";
    cout << "  Running...\n\n";

    // Initial: a = φ^3, b = φ^5
    auto ct_state = encrypt_state(3.0, 5.0);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        // HONEST NA OPERASYON:
        // Walang hardcoded delta
        // Walang alternating pattern
        // Hayaan ang EvalAdd na gumana sa lahat ng slots
        // 
        // Ang φ-structure ng Slot 1 at Slot 3 ay dapat
        // magbigay ng natural na pagpili ng max
        //
        // Ito ay: EvalAdd ng state sa sarili nito
        // (Hindi ito dayaan — ito ay kung ano ang
        // natural na mangyayari sa 4-slot state)
        
        ct_state = cc->EvalAdd(ct_state, ct_state);
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    // Tanggapin ang resulta — mabagsak man o hindi
    auto v_final = decrypt_state(ct_state);

    cout << "  ✅ Chain complete!\n";
    cout << "  Time: " << time << " ms\n\n";
    cout << "  Final Slot 0 (n_a): " << v_final[0] << "\n";
    cout << "  Final Slot 1 (F_a): " << v_final[1] << "\n";
    cout << "  Final Slot 2 (n_b): " << v_final[2] << "\n";
    cout << "  Final Slot 3 (F_b): " << v_final[3] << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  HONEST RESULT\n";
    cout << "========================================\n\n";

    return 0;
}
