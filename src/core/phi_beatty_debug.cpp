// ============================================
// φ-BEATTY DEBUG
// Tingnan ang state sa bawat checkpoint
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

    auto encrypt_beatty = [&](double F) {
        int q = (int)floor(F / PHI);
        int grupo = q % 2;
        vector<double> v(4, 0.0);
        v[0] = fmod(F, PHI);
        v[1] = grupo;
        v[2] = log(F) / LN_PHI;
        v[3] = F;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_beatty = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "=== φ-BEATTY DEBUG ===\n\n";

    int N = 100;
    int checkpoint = 10;

    auto ct_state = encrypt_beatty(100.0);
    double expected = 100.0;

    auto ct_add = encrypt_beatty(7.0);
    auto ct_sub = encrypt_beatty(3.0);
    
    vector<double> v_mul(4, 0.0);
    v_mul[2] = log(2.0) / LN_PHI;
    Plaintext pt_mul = cc->MakeCKKSPackedPlaintext(v_mul);
    auto ct_mul = cc->Encrypt(keyPair.publicKey, pt_mul);
    
    vector<double> v_div(4, 0.0);
    v_div[2] = -log(3.0) / LN_PHI;
    Plaintext pt_div = cc->MakeCKKSPackedPlaintext(v_div);
    auto ct_div = cc->Encrypt(keyPair.publicKey, pt_div);

    cout << "  Op | Slot0(modφ) | Slot1(grp) | Slot2(log) | Slot3(F) | Expected\n";
    cout << "  ---|-------------|------------|------------|----------|---------\n";

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

        if ((i + 1) % checkpoint == 0) {
            auto v = decrypt_beatty(ct_state);
            cout << "  " << setw(3) << (i+1) << " | "
                 << setw(11) << fixed << setprecision(2) << v[0] << " | "
                 << setw(10) << v[1] << " | "
                 << setw(10) << v[2] << " | "
                 << setw(8) << v[3] << " | "
                 << setw(8) << expected << "\n";
        }
    }

    cout << "\n=== ANALYSIS ===\n";
    cout << "  Tignan kung saan nagdi-diverge ang Slot0 at Slot2\n";
    cout << "  sa Slot3 (na dapat ay laging tugma sa Expected)\n\n";

    return 0;
}
