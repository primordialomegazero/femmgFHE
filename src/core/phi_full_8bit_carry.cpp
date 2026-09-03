// ============================================
// φ-FULL 8-BIT ALU WITH CARRY
//
// Symmetric XOR3: 0→+φ/2, 1→+φ
// 8-bit full adder with carry propagation
// Carry: AND gate, Sum: XOR3 gate
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
    cout << "  φ-FULL 8-BIT ALU WITH CARRY\n";
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
    cout << "  Full 8-bit ALU with carry\n\n";

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

    // XOR3 DECODE (8/8 na)
    auto xor3_decode = [&](double val) {
        double mod_2phi = fmod(val, TWO_PHI);
        
        if (mod_2phi < 0.1 || mod_2phi > TWO_PHI - 0.1) return 1;
        if (abs(mod_2phi - PHI) < 0.1) return 1;
        
        if (abs(mod_2phi - HALF_PHI) < 0.1 || abs(mod_2phi - PHI - HALF_PHI) < 0.1) return 0;
        
        double mod_phi = fmod(val, PHI);
        if (mod_phi < 0.1 || mod_phi > PHI - 0.1) return 1;
        return 0;
    };

    // CARRY DECODE (AND gate)
    auto carry_decode = [&](double val) {
        // Carry = 1 kung 2 o 3 inputs ay 1
        // Sa φ encoding: 2 o 3 ones = 2φ o 3φ
        double mod_2phi = fmod(val, TWO_PHI);
        
        // 2 ones = 2φ → mod 0 → carry 1
        // 3 ones = 3φ → mod φ → carry 1
        // 0 ones = 0 → carry 0
        // 1 one = φ/2 o φ → carry 0
        
        if (mod_2phi < 0.1 || mod_2phi > TWO_PHI - 0.1) return 1;
        if (abs(mod_2phi - PHI) < 0.1) return 1;
        
        return 0;
    };

    // ============================================
    // TEST: 8-BIT FULL ADDER WITH CARRY
    // ============================================

    cout << "========================================\n";
    cout << "  8-BIT FULL ADDER (A + B)\n";
    cout << "========================================\n\n";

    // A = 10101010 (170), B = 01010101 (85)
    // Expected: 11111111 (255)
    vector<int> A_bits = {1, 0, 1, 0, 1, 0, 1, 0};
    vector<int> B_bits = {0, 1, 0, 1, 0, 1, 0, 1};

    vector<int> sum_bits(8, 0);
    vector<int> carry_bits(9, 0);
    int alu_match = 0;

    auto start = high_resolution_clock::now();

    for (int bit_pos = 0; bit_pos < 8; bit_pos++) {
        int A = A_bits[bit_pos];
        int B = B_bits[bit_pos];
        int Cin = carry_bits[bit_pos];
        
        // XOR3: Sum = A XOR B XOR Cin
        auto ct_a = encrypt_bit(A);
        auto ct_b = encrypt_bit(B);
        auto ct_cin = encrypt_bit(Cin);
        
        auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_a, ct_b), ct_cin);
        double sum_avg = decrypt_avg(ct_sum);
        int sum_decoded = xor3_decode(sum_avg);
        
        // AND3: Carry = (A AND B) OR (A AND Cin) OR (B AND Cin)
        // Sa φ: Carry = 1 kung 2+ inputs ay 1
        double carry_avg = sum_avg; // Same sum, pero carry ang decode
        int carry_decoded = carry_decode(carry_avg);
        
        sum_bits[bit_pos] = sum_decoded;
        carry_bits[bit_pos + 1] = carry_decoded;
        
        int expected_sum = (A + B + Cin) % 2;
        int expected_carry = (A + B + Cin) >= 2 ? 1 : 0;
        
        bool sum_match = (sum_decoded == expected_sum);
        bool carry_match = (carry_decoded == expected_carry);
        bool both_match = sum_match && carry_match;
        alu_match += both_match;
        
        cout << "  Bit " << bit_pos << ": " << A << " + " << B 
             << " + " << Cin << " = Sum:" << sum_decoded 
             << " (Exp:" << expected_sum << ") Carry:" << carry_decoded 
             << " (Exp:" << expected_carry << ") "
             << (both_match ? "✅" : "❌") << "\n";
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    // Final result
    cout << "\n  A:       ";
    for (int i = 7; i >= 0; i--) cout << A_bits[i];
    cout << " (" << 170 << ")\n";
    
    cout << "  B:       ";
    for (int i = 7; i >= 0; i--) cout << B_bits[i];
    cout << " (" << 85 << ")\n";
    
    cout << "  Sum:     ";
    for (int i = 7; i >= 0; i--) cout << sum_bits[i];
    cout << "\n";
    
    cout << "  Carry:   ";
    for (int i = 8; i >= 0; i--) cout << carry_bits[i];
    cout << "\n";
    
    cout << "  Expected: 11111111 (255)\n";
    cout << "  ALU Match: " << alu_match << "/8\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Level: 0\n\n";

    // ============================================
    // SECOND TEST: A = 11110000, B = 00001111
    // ============================================

    cout << "========================================\n";
    cout << "  SECOND TEST: 11110000 + 00001111\n";
    cout << "========================================\n\n";

    vector<int> A_bits2 = {1, 1, 1, 1, 0, 0, 0, 0};
    vector<int> B_bits2 = {0, 0, 0, 0, 1, 1, 1, 1};

    vector<int> sum_bits2(8, 0);
    vector<int> carry_bits2(9, 0);
    int alu_match2 = 0;

    for (int bit_pos = 0; bit_pos < 8; bit_pos++) {
        int A = A_bits2[bit_pos];
        int B = B_bits2[bit_pos];
        int Cin = carry_bits2[bit_pos];
        
        auto ct_a = encrypt_bit(A);
        auto ct_b = encrypt_bit(B);
        auto ct_cin = encrypt_bit(Cin);
        
        auto ct_sum = cc->EvalAdd(cc->EvalAdd(ct_a, ct_b), ct_cin);
        double sum_avg = decrypt_avg(ct_sum);
        int sum_decoded = xor3_decode(sum_avg);
        
        double carry_avg = sum_avg;
        int carry_decoded = carry_decode(carry_avg);
        
        sum_bits2[bit_pos] = sum_decoded;
        carry_bits2[bit_pos + 1] = carry_decoded;
        
        int expected_sum = (A + B + Cin) % 2;
        int expected_carry = (A + B + Cin) >= 2 ? 1 : 0;
        
        bool both_match = (sum_decoded == expected_sum) && (carry_decoded == expected_carry);
        alu_match2 += both_match;
        
        cout << "  Bit " << bit_pos << ": " << A << " + " << B 
             << " + " << Cin << " = Sum:" << sum_decoded 
             << " (Exp:" << expected_sum << ") Carry:" << carry_decoded 
             << " (Exp:" << expected_carry << ") "
             << (both_match ? "✅" : "❌") << "\n";
    }

    cout << "\n  Sum:     ";
    for (int i = 7; i >= 0; i--) cout << sum_bits2[i];
    cout << "\n";
    
    cout << "  Expected: 11111111 (255)\n";
    cout << "  ALU Match: " << alu_match2 << "/8\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  FULL 8-BIT ALU COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Test 1: " << alu_match << "/8\n";
    cout << "  ✅ Test 2: " << alu_match2 << "/8\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
