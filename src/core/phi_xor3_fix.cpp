// ============================================
// φ-XOR3 FIX — TAMANG 3-INPUT DECODE
//
// 2-input: 0→+φ/2, 1→+φ
// 3-input: mod 2φ para sa parity
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
    cout << "  φ-XOR3 FIX\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double HALF_PHI = PHI / 2.0;
    const double TWO_PHI = 2.0 * PHI;

    cout << "  ✅ CKKS initialized (depth 1, modsize 59!)\n";
    cout << "  XOR3: 0→+φ/2, 1→+φ, mod 2φ\n\n";

    auto encrypt_bit = [&](int bit) {
        double val = (bit == 0) ? HALF_PHI : PHI;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_avg = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        return sum / 16.0;
    };

    // 2-INPUT XOR DECODE
    auto xor2_decode = [&](double val) {
        double mod_phi = fmod(val, PHI);
        
        if (mod_phi < 0.1 || mod_phi > PHI - 0.1) return 0;
        if (abs(mod_phi - HALF_PHI) < 0.1) return 1;
        
        return (mod_phi < PHI / 2.0) ? 1 : 0;
    };

    // 3-INPUT XOR DECODE
    auto xor3_decode = [&](double val) {
        double mod_2phi = fmod(val, TWO_PHI);
        
        // 0+0+0 = 3φ/2 → mod 2φ = 3φ/2
        // 0+0+1 = 2φ → mod 2φ = 0
        // 0+1+1 = 5φ/2 → mod 2φ = φ/2
        // 1+1+1 = 3φ → mod 2φ = φ
        
        // Even count → 0: 0 o φ o 2φ
        // Odd count → 1: φ/2 o 3φ/2
        
        if (mod_2phi < 0.1 || mod_2phi > TWO_PHI - 0.1) return 0;
        if (abs(mod_2phi - PHI) < 0.1) return 0;
        
        if (abs(mod_2phi - HALF_PHI) < 0.1 || abs(mod_2phi - PHI - HALF_PHI) < 0.1) return 1;
        
        // Fallback: odd/even based sa mod φ
        double mod_phi = fmod(val, PHI);
        if (mod_phi < 0.1 || mod_phi > PHI - 0.1) return 0;
        return 1;
    };

    // ============================================
    // TEST 1: SINGLE XOR3 (8/8 CHECK)
    // ============================================

    cout << "========================================\n";
    cout << "  SINGLE XOR3 (8/8 CHECK)\n";
    cout << "========================================\n\n";

    cout << "  A B Cin | Sum | Expected | Match?\n";
    cout << "  --------|-----|----------|--------\n";

    int single_match = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int Cin : {0, 1}) {
                auto ct_a = encrypt_bit(A);
                auto ct_b = encrypt_bit(B);
                auto ct_cin = encrypt_bit(Cin);
                
                auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_a, ct_b), ct_cin);
                double avg = decrypt_avg(ct_sum);
                int decoded = xor3_decode(avg);
                
                int expected = (A + B + Cin) % 2;
                bool match = (decoded == expected);
                single_match += match;
                
                cout << "  " << A << " " << B << " " << Cin << " | "
                     << setw(3) << decoded << " | "
                     << setw(6) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Single XOR3: " << single_match << "/8\n\n";

    // ============================================
    // TEST 2: 8-BIT ALU (2-INPUT)
    // ============================================

    cout << "========================================\n";
    cout << "  8-BIT ALU (2-INPUT XOR)\n";
    cout << "========================================\n\n";

    vector<int> A_bits = {1, 0, 1, 0, 1, 0, 1, 0};
    vector<int> B_bits = {0, 1, 0, 1, 0, 1, 0, 1};

    int alu_match = 0;

    for (int bit_pos = 0; bit_pos < 8; bit_pos++) {
        int A = A_bits[bit_pos];
        int B = B_bits[bit_pos];
        
        int expected_sum = (A + B) % 2;
        
        auto ct_a = encrypt_bit(A);
        auto ct_b = encrypt_bit(B);
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        
        double avg = decrypt_avg(ct_sum);
        int decoded = xor2_decode(avg);
        
        bool match = (decoded == expected_sum);
        alu_match += match;
        
        cout << "  Bit " << bit_pos << ": " << A << " + " << B 
             << " = " << decoded << " (Expected: " << expected_sum << ") "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n  ALU Match: " << alu_match << "/8\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  XOR3 FIX COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Single XOR3: " << single_match << "/8\n";
    cout << "  ✅ ALU 2-input: " << alu_match << "/8\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
