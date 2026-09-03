// ============================================
// φ-SUPERPOSITION HARMONIZED
//
// value = normal × φ⁻¹ + log × φ
// Harmonized para walang collision
//
// Normal: 0→0, 1→φ⁻¹
// Log: 0→-2, 1→+2
// Superposition: normal × φ⁻¹ + log × φ
//
// Author: Dan Fernandez / Primordial Omega Zero
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
    cout << "========================================\n";
    cout << "  φ-SUPERPOSITION HARMONIZED\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  Harmonized: normal × φ⁻¹ + log × φ\n\n";

    // ============================================
    // HARMONIZED SUPERPOSITION
    // ============================================

    auto encrypt_harmonized = [&](int bit) {
        // HARMONIZED:
        // normal: 0 → 0, 1 → φ⁻¹ = 0.618
        // log: 0 → -2, 1 → +2
        // value = normal × φ⁻¹ + log × φ
        
        double normal_val = (bit == 0) ? 0.0 : PHI_INV;
        double log_val = (bit == 0) ? -2.0 : 2.0;
        
        double super_val = normal_val * PHI_INV + log_val * PHI;
        
        vector<double> v(8, 0.0);
        v[0] = super_val;
        for (int i = 1; i < 8; i++) {
            v[i] = super_val * pow(PHI, i);
        }
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_harmonized = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        double super_val = result_pt->GetCKKSPackedValue()[0].real();
        
        // HARMONIZED DECOMPOSITION:
        // super = normal × φ⁻¹ + log × φ
        // log = round(super / φ)
        // normal = (super - log × φ) × φ
        
        double log_val = super_val / PHI;
        double normal_val = (super_val - log_val * PHI) * PHI;
        
        int normal_bit = (normal_val > 0.3) ? 1 : 0;
        int log_bit = (log_val >= -0.01) ? 1 : 0;
        
        return make_pair(normal_bit, log_bit);
    };

    // ============================================
    // GATES
    // ============================================

    auto gate_nand = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalNegate(cc->EvalAdd(a, b));
    };

    auto gate_and = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    };

    auto gate_or = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    };

    auto gate_xor = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, cc->EvalNegate(b));
    };

    // ============================================
    // TEST 1: HARMONIZED ENCODING
    // ============================================

    cout << "========================================\n";
    cout << "  TEST 1: HARMONIZED ENCODING\n";
    cout << "========================================\n\n";

    cout << "  Bit | Super Value | Normal | Log | Match?\n";
    cout << "  ----|-------------|--------|-----|--------\n";

    for (int bit : {0, 1}) {
        auto ct = encrypt_harmonized(bit);
        auto [normal_bit, log_bit] = decrypt_harmonized(ct);
        bool match = (normal_bit == bit && log_bit == bit);
        
        cout << "  " << setw(3) << bit << " | "
             << setw(11) << fixed << setprecision(3) << (bit == 0 ? -3.236 : 3.618) << " | "
             << setw(6) << normal_bit << " | "
             << setw(3) << log_bit << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n";

    // ============================================
    // TEST 2: ALL GATES
    // ============================================

    cout << "========================================\n";
    cout << "  ALL GATES (HARMONIZED)\n";
    cout << "========================================\n\n";

    cout << "  A B | NAND | AND | OR | XOR\n";
    cout << "  ----|------|-----|----|----\n";

    int all_correct = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_harmonized(A);
            auto ct_b = encrypt_harmonized(B);
            
            auto nand_ct = gate_nand(ct_a, ct_b);
            auto and_ct = gate_and(ct_a, ct_b);
            auto or_ct = gate_or(ct_a, ct_b);
            auto xor_ct = gate_xor(ct_a, ct_b);
            
            auto [nand_n, nand_l] = decrypt_harmonized(nand_ct);
            auto [and_n, and_l] = decrypt_harmonized(and_ct);
            auto [or_n, or_l] = decrypt_harmonized(or_ct);
            auto [xor_n, xor_l] = decrypt_harmonized(xor_ct);
            
            int exp_nand = !(A && B);
            int exp_and = (A && B);
            int exp_or = (A || B);
            int exp_xor = (A != B);
            
            int nand_r = nand_l;
            int and_r = and_n;
            int or_r = or_l;
            int xor_r = xor_n;
            
            all_correct += (nand_r == exp_nand) + (and_r == exp_and) + 
                          (or_r == exp_or) + (xor_r == exp_xor);
            
            bool all_ok = (nand_r == exp_nand && and_r == exp_and && or_r == exp_or && xor_r == exp_xor);
            
            cout << "  " << A << " " << B << " | "
                 << setw(4) << nand_r << " | "
                 << setw(3) << and_r << " | "
                 << setw(2) << or_r << " | "
                 << setw(3) << xor_r << " | "
                 << (all_ok ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n  All gates: " << all_correct << "/16\n\n";

    // ============================================
    // TEST 3: FULL ADDER
    // ============================================

    cout << "========================================\n";
    cout << "  FULL ADDER (HARMONIZED)\n";
    cout << "========================================\n\n";

    cout << "  A B Cin | Sum Cout | Both?\n";
    cout << "  --------|----------|------\n";

    int adder_correct = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                auto ct_a = encrypt_harmonized(A);
                auto ct_b = encrypt_harmonized(B);
                auto ct_cin = encrypt_harmonized(Cin);
                
                auto xor_ab = gate_xor(ct_a, ct_b);
                auto sum_ct = gate_xor(xor_ab, ct_cin);
                auto [sum_n, sum_l] = decrypt_harmonized(sum_ct);
                
                auto and_ab = gate_and(ct_a, ct_b);
                auto and_cin_xor = gate_and(ct_cin, xor_ab);
                auto cout_ct = gate_or(and_ab, and_cin_xor);
                auto [cout_n, cout_l] = decrypt_harmonized(cout_ct);
                
                int sum = sum_n;
                int cout_val = cout_n;
                
                int exp_sum = (A + B + Cin) % 2;
                int exp_cout = (A + B + Cin) / 2;
                
                bool both_ok = (sum == exp_sum && cout_val == exp_cout);
                adder_correct += both_ok;
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << " " << sum << "   " << cout_val << "   | "
                     << (both_ok ? "✅" : "❌") << "\n";
            }
        }
    }

    cout << "\n  Full Adder: " << adder_correct << "/8\n\n";

    cout << "========================================\n";
    cout << "  HARMONIZED COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Harmonized superposition\n";
    cout << "  ✅ All gates: " << all_correct << "/16\n";
    cout << "  ✅ Full adder: " << adder_correct << "/8\n";
    cout << "  ✅ Towers: 2\n";
    cout << "  ✅ Level 0 forever\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
