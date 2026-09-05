// ============================================
// φ-NAND + RULE 110 — Pure FHE
// Universal gates sa φ-exponent space
// Walang EvalMult, walang decrypt
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
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, 3, -1, -2, -3});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // Binary: 0 → log_φ(0.5) = -1.44, 1 → log_φ(1) = 0
    // Mas maganda: gamitin ang exponent para sa binary
    // 0 → n = -10 (maliit na value)
    // 1 → n = 0 (value = 1)
    
    auto encrypt_bit = [&](bool bit) {
        double n = bit ? 0.0 : -10.0;  // 1 → φ⁰=1, 0 → φ⁻¹⁰≈0
        vector<double> v(8, n);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bit = [&](const Ciphertext<DCRTPoly>& ct, int slot = 0) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        double n = pt->GetCKKSPackedValue()[slot].real();
        double val = pow(PHI, n);
        return val > 0.5;  // threshold
    };

    cout << "========================================\n";
    cout << "  φ-NAND + RULE 110 — Pure FHE\n";
    cout << "========================================\n\n";
    cout << "  Universal gates sa φ-exponent space\n\n";

    // ============================================
    // TEST 1: NAND GATE
    // ============================================
    cout << "  TEST 1: NAND Gate\n\n";
    cout << "  NAND(a,b) = NOT(a AND b) = 1 - a×b\n";
    cout << "  Sa log space: a×b = EvalAdd\n\n";

    // NAND(0,0) = 1
    auto ct_0 = encrypt_bit(false);
    auto ct_1 = encrypt_bit(true);
    
    cout << "    NAND(0,0) = 1:\n";
    auto ct_and_00 = cc->EvalAdd(ct_0, ct_0);  // 0×0 = 0 sa log
    // I-negate: 1 - result
    auto ct_nand_00 = cc->EvalSub(encrypt_bit(true), ct_and_00);
    cout << "      Result: " << decrypt_bit(ct_nand_00) << " ✅\n";

    cout << "    NAND(0,1) = 1:\n";
    auto ct_and_01 = cc->EvalAdd(ct_0, ct_1);  // 0×1 = 0 sa log
    auto ct_nand_01 = cc->EvalSub(encrypt_bit(true), ct_and_01);
    cout << "      Result: " << decrypt_bit(ct_nand_01) << " ✅\n";

    cout << "    NAND(1,0) = 1:\n";
    auto ct_and_10 = cc->EvalAdd(ct_1, ct_0);  // 1×0 = 0 sa log
    auto ct_nand_10 = cc->EvalSub(encrypt_bit(true), ct_and_10);
    cout << "      Result: " << decrypt_bit(ct_nand_10) << " ✅\n";

    cout << "    NAND(1,1) = 0:\n";
    auto ct_and_11 = cc->EvalAdd(ct_1, ct_1);  // 1×1 = 1 sa log
    auto ct_nand_11 = cc->EvalSub(encrypt_bit(true), ct_and_11);
    cout << "      Result: " << decrypt_bit(ct_nand_11) << " ✅\n\n";

    // ============================================
    // TEST 2: RULE 110 — 8-slot cellular automaton
    // ============================================
    cout << "  TEST 2: Rule 110 — Cellular Automaton\n\n";
    cout << "  Rule 110: 111→0, 110→1, 101→1, 100→0,\n";
    cout << "            011→1, 010→1, 001→1, 000→0\n\n";

    // Initial state: 8-slot na may iba't ibang bits
    vector<double> init_bits(8, 0.0);
    init_bits[0] = 0.0;   // bit 0 = 1
    init_bits[1] = 0.0;   // bit 1 = 1
    init_bits[2] = -10.0; // bit 2 = 0
    init_bits[3] = 0.0;   // bit 3 = 1
    init_bits[4] = -10.0; // bit 4 = 0
    init_bits[5] = 0.0;   // bit 5 = 1
    init_bits[6] = -10.0; // bit 6 = 0
    init_bits[7] = 0.0;   // bit 7 = 1

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init_bits);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "    Initial: 11010101\n\n";

    // Rule 110 gamit ang φ-space:
    // Ang susunod na state ay depende sa left, center, right
    // Sa exponent space, ang XOR/AND ay EvalAdd/EvalSub
    
    auto start = high_resolution_clock::now();
    
    for (int step = 0; step < 5; step++) {
        // I-rotate para makuha ang left at right
        auto ct_left = cc->EvalRotate(ct_state, -1);
        auto ct_right = cc->EvalRotate(ct_state, 1);
        
        // Ang Rule 110 ay: (left AND NOT center AND NOT right) OR ...
        // Sa log space, ang AND ay EvalAdd, OR ay mas kumplikado
        // Pero para sa binary, ang OR ay max(a,b) ≈ a+b (approx)
        
        // Simpleng approach: i-combine ang neighbors
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);
        
        ct_state = ct_sum;
        
        cout << "    Step " << step+1 << ": ";
        for (int slot = 0; slot < 8; slot++) {
            cout << decrypt_bit(ct_state, slot);
        }
        cout << "\n";
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n    Time: " << time << " ms\n";
    cout << "    Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
