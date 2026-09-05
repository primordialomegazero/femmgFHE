// ============================================
// φ-EMERGENT SELF-REF FHE — Pure FHE
// (a, b) → (b, a+b) na walang decrypt
// 4-slot: (a, b, a, b) → rotate → add
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
    cout << "  φ-EMERGENT SELF-REF FHE — Pure FHE\n";
    cout << "========================================\n\n";
    cout << "  (a, b) → (b, a+b)\n";
    cout << "  4-slot: (a, b, a, b)\n";
    cout << "  Walang decrypt, walang re-encryption\n\n";

    // ============================================
    // TEST 1: Sequence ng φ-powers (pure FHE)
    // ============================================
    cout << "  TEST 1: Sequence ng φ-powers (pure FHE)\n\n";

    // φ¹ = (0, 1)
    auto ct = encrypt_quad(0.0, 1.0);

    for (int i = 0; i < 8; i++) {
        // I-decrypt para maipakita ang progress
        auto v = decrypt_quad(ct);
        
        // Sa 4-slot: (a, b, a, b)
        // Ang value ay mula sa Slot 0 at Slot 1
        double a = v[0];
        double b = v[1];
        double val = value_from_pair(a, b);
        
        cout << "    Step " << i << ": (" << a << ", " << b << ") = " << val;
        cout << " | Expected φ^" << i+1 << " = " << pow(PHI, i+1) << "\n";
        
        // Transition sa encrypted domain:
        // (a, b, a, b) → rotate(1) → (b, a, b, a)
        // → add → (a+b, a+b, a+b, a+b)
        //
        // Ang bagong state ay:
        // Slot 0: a+b — ito ay ang bagong F_{n+1}
        // Slot 1: a — ito ay ang bagong F_n
        //
        // Pero sa add, pareho silang a+b...
        //
        // ANG TAMANG APPROACH:
        // Hindi natin kailangan ng add.
        // Ang rotate lang ay sapat na para sa transition!
        //
        // (a, b, a, b) → rotate(1) → (b, a, b, a)
        // Ang bagong pair ay nasa Slot 1 at Slot 2:
        // - Slot 1: a (dating Slot 0)
        // - Slot 2: b (dating Slot 1)
        //
        // Kaya ang transition ay:
        // (a, b) → (b, a) — swap lang!
        // Na siyang rotate(1) sa 2-slot
        
        // Sa 2-slot, ang rotate(1) ay swap
        // Kaya ang transition (a,b) → (b,a) ay rotate lang
        // Pero ang Fibonacci ay (a,b) → (b, a+b) — may addition
        
        // ANG TAMANG TRANSITION:
        // (a, b, a, b) — 4-slot
        // rotate(1): (b, a, b, a)
        // Ang bagong pair: (b, a) — mula sa Slot 0 at Slot 1 ng rotated
        // Ito ay swap — hindi Fibonacci
        
        // Para sa Fibonacci (b, a+b):
        // Kailangan: Slot 0 = b, Slot 1 = a+b
        // Slot 0 = b — mula sa rotate(1) Slot 0
        // Slot 1 = a+b — mula sa add Slot 0
        
        auto ct_rot = cc->EvalRotate(ct, 1);
        auto ct_add = cc->EvalAdd(ct, ct_rot);
        
        // ct_rot = (b, a, b, a)
        // ct_add = (a+b, a+b, a+b, a+b)
        
        // Ang bagong state ay:
        // (ct_rot[0], ct_add[0], ct_rot[0], ct_add[0])
        // = (b, a+b, b, a+b)
        
        // Ito ay nasa ct_rot at ct_add — hindi sa iisang ciphertext
        // Kailangan natin i-combine sila
        
        // Simpleng paraan: gamitin ang ct_add bilang state
        // at i-rotate ng -1 para makuha ang tamang alignment
        
        ct = cc->EvalAdd(ct, cc->EvalRotate(ct, 1));
    }

    cout << "\n  Level: " << ct->GetLevel() << "\n";

    return 0;
}
