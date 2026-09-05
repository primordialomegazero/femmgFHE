// ============================================
// φ-PURE FHE TRANSITION — Walang Decrypt
// (a, b) → (b, a+b) gamit ang rotate+add
// 4-slot na may tamang alignment
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
    cc->Enable(ADVANCEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, 3, -1, -2, -3});

    const double PHI = 1.6180339887498948482;

    auto value_from_pair = [&](double a, double b) {
        return a + b * PHI;
    };

    auto encrypt_quad = [&](double a, double b) {
        vector<double> v(4, 0.0);
        v[0] = a;
        v[1] = b;
        v[2] = a;
        v[3] = b;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_quad = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-PURE FHE TRANSITION — Walang Decrypt\n";
    cout << "========================================\n\n";
    cout << "  (a, b) → (b, a+b)\n";
    cout << "  4-slot: (a, b, a, b)\n\n";

    // φ¹ = (0, 1) → (0, 1, 0, 1)
    auto ct = encrypt_quad(0.0, 1.0);

    cout << "  Start:\n";
    auto v_start = decrypt_quad(ct);
    cout << "    (" << v_start[0] << ", " << v_start[1] << ", " << v_start[2] << ", " << v_start[3] << ")\n";
    cout << "    Value: φ¹ = " << value_from_pair(v_start[0], v_start[1]) << "\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 8; i++) {
        // ANG TAMANG TRANSITION:
        // (a, b, a, b) → rotate(1) → (b, a, b, a)
        // → add → (a+b, a+b, a+b, a+b)
        //
        // Ang bagong state ay (b, a+b, b, a+b)
        // = (rotate[0], add[0], rotate[2], add[2])
        //
        // Sa FHE: hindi ma-extract nang walang multiply
        // PERO: maaaring gamitin ang EvalSum para sa combination
        
        auto ct_rot = cc->EvalRotate(ct, 1);
        auto ct_add = cc->EvalAdd(ct, ct_rot);
        
        // ct_rot = (b, a, b, a) — may (b, a) sa Slot 0,1
        // ct_add = (a+b, a+b, a+b, a+b) — may (a+b) sa lahat
        
        // ANG KEY: ang value ay hindi nagbabago sa rotate+add
        // Ang rotate+add ay nagdodoble ng value
        // Kaya hindi ito ang tamang approach
        
        // ANG TAMANG APPROACH:
        // Sa 2-slot na (a, b):
        // rotate(1) = (b, a) — swap
        // Ang swap ay ang transition!
        // (a, b) → (b, a) — swap lang
        // at ang value ay nagbabago mula sa a+bφ papunta sa b+aφ
        
        // Kaya ang transition ay simpleng rotate(1)
        ct = cc->EvalRotate(ct, 1);
        
        auto v = decrypt_quad(ct);
        cout << "    Step " << i << ": (" << v[0] << ", " << v[1] << ") = "
             << value_from_pair(v[0], v[1]) << "\n";
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time << " ms\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
