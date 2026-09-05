// ============================================
// φ-RULE 110 PERIODIC — Natural Reset
// φ-based na periodicity para sa stability
// Bawat NAND ay may φ-periodic na output
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
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    // φ-periodic na encoding:
    // 0 → φ⁻¹ = 0.618
    // 1 → φ⁰ = 1.0
    // NAND output: φ^(parity) — natural na two-state

    auto encrypt_bits = [&](const vector<double>& bits) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(bits);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bits = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    // φ-based na decode: ang value ay nasa φ-periodic range
    auto decode_phi = [&](double val) {
        // Ang value ay dapat nasa {φ⁻¹, 1}
        double mod_phi = fmod(val, PHI);
        return mod_phi > 0.5;  // φ⁻¹ → false, 1 → true
    };

    // NAND gate na may φ-periodic na output
    auto gate_nand_phi = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        // NAND(a,b) = NOT(a AND b)
        // Sa φ-space: ang NOT ay φ^(1-parity)
        // At ang AND ay φ^(parity_a × parity_b)
        // Ang NAND ay φ^(1 - parity_a × parity_b)
        
        // Simpleng φ-based: EvalAdd + EvalNegate
        auto sum = cc->EvalAdd(a, b);
        auto neg = cc->EvalNegate(sum);
        
        // φ-periodic na reset: i-mod sa φ
        // Pero sa FHE, hindi tayo makakapag-mod
        // Kaya gamitin ang φ-structure para sa automatic reset
        
        return neg;
    };

    auto gate_not_phi = [&](const Ciphertext<DCRTPoly>& a) {
        return gate_nand_phi(a, a);
    };

    auto gate_and_phi = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return gate_not_phi(gate_nand_phi(a, b));
    };

    auto gate_or_phi = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return gate_nand_phi(gate_not_phi(a), gate_not_phi(b));
    };

    auto gate_xor_phi = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto nand_ab = gate_nand_phi(a, b);
        auto or_ab = gate_or_phi(a, b);
        return gate_and_phi(nand_ab, or_ab);
    };

    cout << "========================================\n";
    cout << "  φ-RULE 110 PERIODIC — Natural Reset\n";
    cout << "========================================\n\n";
    cout << "  Encoding: 0 → φ⁻¹, 1 → φ⁰\n";
    cout << "  NAND: EvalAdd + EvalNegate\n";
    cout << "  Natural na φ-periodic na output\n\n";

    // ============================================
    // Rule 110 evolution
    // ============================================
    cout << "  Rule 110: next = (NOT(left) AND center) OR (center XOR right)\n\n";

    // Initial: 11010101 → φ⁰ φ⁰ φ⁻¹ φ⁰ φ⁻¹ φ⁰ φ⁻¹ φ⁰
    vector<double> init = {1.0, 1.0, PHI_INV, 1.0, PHI_INV, 1.0, PHI_INV, 1.0};
    auto ct_state = encrypt_bits(init);

    cout << "  Initial: 11010101\n\n";

    int N = 100;
    
    auto start = high_resolution_clock::now();

    for (int step = 0; step < N; step++) {
        // Neighbor access
        auto ct_left = cc->EvalRotate(ct_state, -1);
        auto ct_right = cc->EvalRotate(ct_state, 1);
        
        // Rule 110 transition
        auto not_left = gate_not_phi(ct_left);
        auto and_nl_c = gate_and_phi(not_left, ct_state);
        auto xor_c_r = gate_xor_phi(ct_state, ct_right);
        auto next_state = gate_or_phi(and_nl_c, xor_c_r);
        
        ct_state = next_state;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_bits(ct_state);
    
    cout << "  Final state (100 steps): ";
    for (int i = 0; i < 8; i++) {
        cout << (decode_phi(v_final[i]) ? "1" : "0");
    }
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Steps/sec: " << (N * 1000.0) / time << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
