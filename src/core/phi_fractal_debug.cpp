// ============================================
// φ-FRACTAL DEBUG — RAW VALUES PER SLOT
//
// Ipakita ang lahat ng 16 slots values
// Para makita ang cross-talk at drift
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
    cout << "  φ-FRACTAL DEBUG\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    cout << "  ✅ CKKS initialized (depth 1!)\n\n";

    auto encrypt_dual = [&](int bit) {
        vector<double> v(16, 0.0);
        v[0] = (bit == 0) ? -2.0 : 2.0;
        for (int i = 1; i < 8; i++) v[i] = v[0];
        for (int i = 8; i < 16; i++) v[i] = (bit == 0) ? 0.0 : 2.0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_all = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        vector<double> results(16);
        for (int i = 0; i < 16; i++) {
            results[i] = result_pt->GetCKKSPackedValue()[i].real();
        }
        return results;
    };

    // ============================================
    // DEBUG: A=0, B=1 — DAPAT NAND=1, AND=0, OR=1, XOR=1
    // ============================================

    cout << "========================================\n";
    cout << "  DEBUG: A=0, B=1\n";
    cout << "  Expected: NAND=1, AND=0, OR=1, XOR=1\n";
    cout << "========================================\n\n";

    auto ct_a = encrypt_dual(0);
    auto ct_b = encrypt_dual(1);

    cout << "  A VALUES (encrypted 0):\n";
    auto a_vals = decrypt_all(ct_a);
    for (int i = 0; i < 16; i++) {
        cout << "  Slot " << setw(2) << i << ": " << fixed << setprecision(4) << a_vals[i] << "\n";
    }
    cout << "\n";

    cout << "  B VALUES (encrypted 1):\n";
    auto b_vals = decrypt_all(ct_b);
    for (int i = 0; i < 16; i++) {
        cout << "  Slot " << setw(2) << i << ": " << fixed << setprecision(4) << b_vals[i] << "\n";
    }
    cout << "\n";

    // NAND: Negate(Add)
    auto nand_ct = cc->EvalNegate(cc->EvalAdd(ct_a, ct_b));
    auto nand_vals = decrypt_all(nand_ct);
    cout << "  NAND RESULT (Negate(Add)):\n";
    for (int i = 0; i < 16; i++) {
        cout << "  Slot " << setw(2) << i << ": " << fixed << setprecision(4) << nand_vals[i] << "\n";
    }
    cout << "  Slot 0 decode: " << (nand_vals[0] >= -0.01 ? 1 : 0) << " (Expected: 1)\n\n";

    // AND: Add
    auto and_ct = cc->EvalAdd(ct_a, ct_b);
    auto and_vals = decrypt_all(and_ct);
    cout << "  AND RESULT (Add):\n";
    for (int i = 0; i < 16; i++) {
        cout << "  Slot " << setw(2) << i << ": " << fixed << setprecision(4) << and_vals[i] << "\n";
    }
    cout << "  Slot 8 decode: " << (and_vals[8] > 3.5 ? 1 : 0) << " (Expected: 0)\n\n";

    // OR: Add
    auto or_ct = cc->EvalAdd(ct_a, ct_b);
    auto or_vals = decrypt_all(or_ct);
    cout << "  OR RESULT (Add):\n";
    for (int i = 0; i < 16; i++) {
        cout << "  Slot " << setw(2) << i << ": " << fixed << setprecision(4) << or_vals[i] << "\n";
    }
    cout << "  Slot 8 decode: " << (or_vals[8] > 0.5 ? 1 : 0) << " (Expected: 1)\n\n";

    // XOR: Sub
    auto xor_ct = cc->EvalSub(ct_a, ct_b);
    auto xor_vals = decrypt_all(xor_ct);
    cout << "  XOR RESULT (Sub):\n";
    for (int i = 0; i < 16; i++) {
        cout << "  Slot " << setw(2) << i << ": " << fixed << setprecision(4) << xor_vals[i] << "\n";
    }
    cout << "  Slot 8 decode: " << (abs(xor_vals[8]) > 0.5 ? 1 : 0) << " (Expected: 1)\n\n";

    // ============================================
    // DEBUG: A=1, B=0 — DAPAT NAND=1, AND=0, OR=1, XOR=1
    // ============================================

    cout << "========================================\n";
    cout << "  DEBUG: A=1, B=0\n";
    cout << "  Expected: NAND=1, AND=0, OR=1, XOR=1\n";
    cout << "========================================\n\n";

    ct_a = encrypt_dual(1);
    ct_b = encrypt_dual(0);

    nand_ct = cc->EvalNegate(cc->EvalAdd(ct_a, ct_b));
    nand_vals = decrypt_all(nand_ct);
    cout << "  NAND Slot 0: " << fixed << setprecision(4) << nand_vals[0] 
         << " → " << (nand_vals[0] >= -0.01 ? 1 : 0) << " (Expected: 1)\n";

    and_ct = cc->EvalAdd(ct_a, ct_b);
    and_vals = decrypt_all(and_ct);
    cout << "  AND Slot 8: " << fixed << setprecision(4) << and_vals[8] 
         << " → " << (and_vals[8] > 3.5 ? 1 : 0) << " (Expected: 0)\n";

    or_ct = cc->EvalAdd(ct_a, ct_b);
    or_vals = decrypt_all(or_ct);
    cout << "  OR Slot 8: " << fixed << setprecision(4) << or_vals[8] 
         << " → " << (or_vals[8] > 0.5 ? 1 : 0) << " (Expected: 1)\n";

    xor_ct = cc->EvalSub(ct_a, ct_b);
    xor_vals = decrypt_all(xor_ct);
    cout << "  XOR Slot 8: " << fixed << setprecision(4) << xor_vals[8] 
         << " → " << (abs(xor_vals[8]) > 0.5 ? 1 : 0) << " (Expected: 1)\n\n";

    // ============================================
    // POSSIBLE FIX: HIWALAY NA ENCRYPTION PER GATE
    // ============================================

    cout << "========================================\n";
    cout << "  FIX: HIWALAY NA ENCRYPTION PER GATE\n";
    cout << "========================================\n\n";

    // Simplest fix: bawat gate may sariling encryption
    auto encrypt_single = [&](int bit) {
        double val = (bit == 0) ? -2.0 : 2.0;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto encrypt_normal = [&](int bit) {
        double val = (bit == 0) ? 0.0 : 2.0;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    // A=0, B=1
    auto sa = encrypt_single(0);
    auto sb = encrypt_single(1);
    auto na = encrypt_normal(0);
    auto nb = encrypt_normal(1);

    // NAND: sa φ² space
    auto snand = cc->EvalNegate(cc->EvalAdd(sa, sb));
    auto snand_vals = decrypt_all(snand);
    cout << "  NAND(0,1): " << snand_vals[0] << " → " 
         << (snand_vals[0] >= -0.01 ? 1 : 0) << " (Expected: 1)\n";

    // AND: sa normal space
    auto sand = cc->EvalAdd(na, nb);
    auto sand_vals = decrypt_all(sand);
    cout << "  AND(0,1): " << sand_vals[0] << " → " 
         << (sand_vals[0] > 3.5 ? 1 : 0) << " (Expected: 0)\n";

    // OR: sa normal space
    auto sor = cc->EvalAdd(na, nb);
    auto sor_vals = decrypt_all(sor);
    cout << "  OR(0,1): " << sor_vals[0] << " → " 
         << (sor_vals[0] > 0.5 ? 1 : 0) << " (Expected: 1)\n";

    // XOR: sa normal space
    auto sxor = cc->EvalSub(na, nb);
    auto sxor_vals = decrypt_all(sxor);
    cout << "  XOR(0,1): " << sxor_vals[0] << " → " 
         << (abs(sxor_vals[0]) > 0.5 ? 1 : 0) << " (Expected: 1)\n\n";

    return 0;
}
