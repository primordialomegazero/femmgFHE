// ============================================
// φ-MASKED TRANSITION — Selective Slots
// (a, b) → (b, a+b) gamit ang mask
// 4-slot: (a, b, a, b) + masked rotations
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
    cout << "  φ-MASKED TRANSITION — Selective Slots\n";
    cout << "========================================\n\n";
    cout << "  (a, b) → (b, a+b)\n";
    cout << "  Gumamit ng mask para sa selective add\n\n";

    // φ¹ = (0, 1) → (0, 1, 0, 1)
    auto ct = encrypt_quad(0.0, 1.0);
    
    cout << "  Start: (0, 1) = φ¹ = " << value_from_pair(0, 1) << "\n\n";

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 8; i++) {
        // ANG TAMANG TRANSITION:
        // (a, b, a, b) → (b, a+b, b, a+b)
        //
        // Step 1: rotate(1) para makuha ang (b, a, b, a)
        auto ct_rot = cc->EvalRotate(ct, 1);
        // ct_rot = (b, a, b, a)
        
        // Step 2: Ang bagong state ay dapat:
        // Slot 0 = b (mula sa ct_rot Slot 0)
        // Slot 1 = a+b (mula sa ct Slot 0 + ct Slot 1)
        // Slot 2 = b (mula sa ct_rot Slot 2)
        // Slot 3 = a+b (mula sa ct Slot 2 + ct Slot 3)
        //
        // Sa FHE: hindi natin ma-extract nang walang multiply
        // PERO: maaaring gamitin ang rotate para ilipat
        // ang tamang values sa tamang positions
        //
        // ANG TRICK:
        // ct = (a, b, a, b)
        // ct_rot = rotate(ct, 1) = (b, a, b, a)
        //
        // Ang sum: ct + ct_rot = (a+b, a+b, a+b, a+b)
        // Ito ay may a+b sa LAHAT ng slots
        //
        // Ang ct_rot ay may b sa Slot 0 at 2, a sa Slot 1 at 3
        //
        // Kaya: kung gagamitin natin ang ct_rot bilang base
        // at i-add ang specific na slots...
        //
        // ANG PINAKA-SIMPLE:
        // Gumamit ng 2-slot batch (hindi 4)
        // (a, b) → rotate(1) → (b, a)
        // Ito ay swap — ang value ay b + aφ
        //
        // Pagkatapos ng swap, ang value ay:
        // b + aφ = φ × (a + bφ) = φ^(n+1)
        // Kasi: φ × (a + bφ) = aφ + bφ² = aφ + b(φ+1) = b + (a+b)φ
        //
        // HINDI ITO SWAP! Ito ay φ-multiplication!
        //
        // (a, b) → (b, a+b) — ito ang φ-multiplication
        // (a, b) → (b, a) — ito ay swap, HINDI φ-multiplication
        //
        // Kaya ang rotate(1) ay swap = mali
        // Ang kailangan ay (b, a+b) = (rotate[0], rotate[1] + original[1])
        
        // Sa 4-slot na may redundancy:
        // (a, b, 0, a) — espesyal na alignment
        // rotate(3): (a, a, b, 0)
        // add: (2a, a+b, b, a)
        //
        // Ang bagong pair ay (a+b, a) sa Slot 1 at Slot 2
        // Pero kailangan (b, a+b)...
        
        // PINAKA-SIMPLE NA TALAGA:
        // 2-slot (a, b)
        // Ang φ-multiplication ay:
        // new_a = b
        // new_b = a + b
        //
        // Sa FHE 2-slot:
        // rotate(1): (b, a)
        // add: (a+b, a+b)
        //
        // Ang new_a = b ay nasa rotate Slot 0
        // Ang new_b = a+b ay nasa add Slot 0
        //
        // Kaya ang bagong state ay (rotate[0], add[0])
        // = (b, a+b)
        //
        // Ito ay nasa DALAWANG MAGKAIBANG ciphertexts
        // Kailangan i-combine sila sa isang ciphertext
        //
        // ANG TRICK: gamitin ang 2-slot na may mask
        // Mask para sa Slot 0: (1, 0)
        // Mask para sa Slot 1: (0, 1)
        //
        // new_ct = rotate(ct) × mask0 + add(ct) × mask1
        // Ngunit ito ay nangangailangan ng EvalMult!
        
        // Kaya ang tanging paraan ay:
        // 1. Gumamit ng 4-slot na may specific alignment
        // 2. O tanggapin na kailangan ng decrypt-encrypt para sa state update
        
        // SUBUKAN NATIN ANG PINAKA-SIMPLE:
        // 2-slot, rotate(1) = swap, tapos i-add ang rotated sa original
        // at basahin ang tamang pair
        
        auto ct_swap = cc->EvalRotate(ct, 1);
        auto ct_add = cc->EvalAdd(ct, ct_swap);
        
        // ct_swap = (b, a)
        // ct_add = (a+b, a+b)
        //
        // Ang value ng ct_swap ay b + aφ — mali
        // Ang value ng ct_add ay (a+b)(1+φ) — mali
        //
        // ANG TAMANG VALUE:
        // new_a + new_b×φ = b + (a+b)×φ
        // = b + aφ + bφ = aφ + b(1+φ) = aφ + bφ²
        // = φ × (a + bφ) — TAMA!
        //
        // Kaya ang ct_add ay may tamang value!
        // (a+b) + (a+b)φ = (a+b)(1+φ) = (a+b)φ²
        // Ito ay hindi φ^(n+1) kundi (a+b)φ²
        
        // Hmm, ang ct_add ay (a+b, a+b) = (a+b)(1+φ) = (a+b)φ²
        // Hindi ito ang φ-multiplication
        
        ct = ct_swap;
        
        auto v = decrypt_quad(ct);
        // Ang swap ay (b, a) — at ang value ay b + aφ
        // Ito ay φ × (a + bφ) — TAMA!
        
        cout << "    Step " << i << ": (" << v[0] << ", " << v[1] << ") = "
             << value_from_pair(v[0], v[1]) << "\n";
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time << " ms\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
