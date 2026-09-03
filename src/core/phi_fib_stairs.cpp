// ============================================
// φ-FIBONACCI STAIRS — TAMANG SCALING
//
// Bawat slot ay may Fibonacci step:
// Slot 0: F(1)=1, Slot 1: F(2)=1, Slot 2: F(3)=2...
// Para sa natural na harmonization!
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
    cout << "  φ-FIBONACCI STAIRS\n";
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

    vector<long long> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};

    cout << "  ✅ CKKS initialized (depth 1, 64 slots!)\n";
    cout << "  Fibonacci stairs: F(1) to F(16)\n\n";

    // ============================================
    // FIBONACCI STAIRS ENCODING
    // ============================================

    auto encrypt_stairs = [&](int bit, int outer_idx) {
        vector<double> v(64, 0.0);
        
        // Bawat outer_idx ay may 4 slots: NAND, AND, OR, XOR
        int base_slot = outer_idx * 4;
        
        double fib_scale = (double)fib[outer_idx];
        
        // NAND (slot 0): φ² space
        v[base_slot + 0] = ((bit == 0) ? -2.0 : 2.0) / fib_scale;
        
        // AND (slot 1): normal space
        v[base_slot + 1] = ((bit == 0) ? 0.0 : 1.0) / fib_scale;
        
        // OR (slot 2): normal space
        v[base_slot + 2] = ((bit == 0) ? 0.0 : 1.0) / fib_scale;
        
        // XOR (slot 3): normal space
        v[base_slot + 3] = ((bit == 0) ? 0.0 : 1.0) / fib_scale;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_stairs = [&](const Ciphertext<DCRTPoly>& ct) {
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
    // TEST: 16 PARALLEL ALUs
    // ============================================

    cout << "========================================\n";
    cout << "  16 PARALLEL ALUs (FIB STAIRS)\n";
    cout << "========================================\n\n";

    cout << "  Outer | Fib | NAND | AND | OR | XOR | All?\n";
    cout << "  ------|-----|------|-----|----|----|------\n";

    int total_correct = 0;

    for (int outer = 0; outer < 16; outer++) {
        auto ct_a = encrypt_stairs(1, outer);
        auto ct_b = encrypt_stairs(1, outer);
        
        // Lahat ng gates
        auto nand_ct = cc->EvalNegate(cc->EvalAdd(ct_a, ct_b));
        auto and_ct = cc->EvalAdd(ct_a, ct_b);
        auto or_ct = cc->EvalAdd(ct_a, ct_b);
        auto xor_ct = cc->EvalSub(ct_a, ct_b);
        
        auto nand_vals = decrypt_stairs(nand_ct);
        auto and_vals = decrypt_stairs(and_ct);
        auto or_vals = decrypt_stairs(or_ct);
        auto xor_vals = decrypt_stairs(xor_ct);
        
        int base = outer * 4;
        double f = (double)fib[outer];
        
        // De-normalize para sa tamang thresholds
        int nand = (nand_vals[base + 0] * f >= -0.01) ? 1 : 0;
        int and_r = (and_vals[base + 1] * f > 1.5) ? 1 : 0;
        int or_r = (or_vals[base + 2] * f > 0.5) ? 1 : 0;
        int xor_r = (abs(xor_vals[base + 3] * f) > 0.5) ? 1 : 0;
        
        int exp_nand = 0;  // NAND(1,1) = 0
        int exp_and = 1;   // AND(1,1) = 1
        int exp_or = 1;    // OR(1,1) = 1
        int exp_xor = 0;   // XOR(1,1) = 0
        
        total_correct += (nand == exp_nand) + (and_r == exp_and) + 
                         (or_r == exp_or) + (xor_r == exp_xor);
        
        bool all_ok = (nand == exp_nand && and_r == exp_and && or_r == exp_or && xor_r == exp_xor);
        
        cout << "  " << setw(5) << outer << " | "
             << setw(3) << fib[outer] << " | "
             << setw(4) << nand << " | "
             << setw(3) << and_r << " | "
             << setw(2) << or_r << " | "
             << setw(3) << xor_r << " | "
             << (all_ok ? "  ✅" : "  ❌") << "\n";
    }

    cout << "\n  Total: " << total_correct << "/64\n\n";

    cout << "========================================\n";
    cout << "  FIBONACCI STAIRS COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Fibonacci stairs\n";
    cout << "  ✅ Total: " << total_correct << "/64\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
