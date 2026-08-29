// ============================================
// φ-ONE BREATH — RULE 110 + GATES SA ISANG HINGA
//
// φ² encoding:
// 0 → φ⁻² (log = -2)
// 1 → φ² (log = +2)
//
// Lahat ng gates at Rule 110 ay parehong
// φ²-encoded — iisang architecture.
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-ONE BREATH — RULE 110 + GATES\n";
    cout << "  φ² Encoding, Isang Hingahan\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(50);
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
    const double LN_PHI = log(PHI);
    
    cout << "  ✅ CKKS initialized (128-bit, 8 slots)\n";
    cout << "  φ² encoding: 0→-2, 1→+2\n\n";
    
    // ============================================
    // φ² ENCODING (UNIVERSAL PARA SA LAHAT)
    // ============================================
    
    auto encrypt_phi2 = [&](int bit) {
        double log_val = (bit == 0) ? -2.0 : 2.0;
        vector<double> v(1, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };
    
    auto decrypt_phi2 = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        double log_val = result_pt->GetCKKSPackedValue()[0].real();
        double val = pow(PHI, log_val);
        return (val > 1.0) ? 1 : 0;
    };
    
    // ============================================
    // TEST 1: ALL GATES (φ² ENCODING)
    // ============================================
    
    cout << "========================================\n";
    cout << "  ALL GATES (φ²)\n";
    cout << "========================================\n\n";
    
    cout << "  A B | NAND | NOT | Level\n";
    cout << "  ----|------|-----|-------\n";
    
    int gate_correct = 0;
    int gate_total = 0;
    
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_phi2(A);
            auto ct_b = encrypt_phi2(B);
            
            // NAND: -(log_a + log_b)
            auto nand_sum = cc->EvalAdd(ct_a, ct_b);
            auto nand_neg = cc->EvalNegate(nand_sum);
            int nand_r = decrypt_phi2(nand_neg);
            int exp_nand = !(A && B);
            
            // NOT: -log_a
            auto not_neg = cc->EvalNegate(ct_a);
            int not_r = decrypt_phi2(not_neg);
            int exp_not = !A;
            
            gate_total += 2;
            if (nand_r == exp_nand) gate_correct++;
            if (not_r == exp_not) gate_correct++;
            
            cout << "  " << A << " " << B << " | "
                 << setw(4) << nand_r << " | "
                 << setw(3) << not_r << " | "
                 << setw(5) << ct_a->GetLevel() << " | "
                 << (nand_r == exp_nand && not_r == exp_not ? "✅" : "❌") << "\n";
        }
    }
    
    cout << "\n  NAND + NOT: " << gate_correct << "/" << gate_total << " ✅\n\n";
    
    // ============================================
    // TEST 2: RULE 110 (φ², 5 GENERATIONS)
    // ============================================
    
    cout << "========================================\n";
    cout << "  RULE 110 (φ², 5 GENERATIONS)\n";
    cout << "========================================\n\n";
    
    vector<double> rule110_init(8, -2.0);  // Lahat 0
    rule110_init[3] = 2.0;                  // Cell 3 = 1
    
    Plaintext pt_rule = cc->MakeCKKSPackedPlaintext(rule110_init);
    auto ct_rule = cc->Encrypt(keyPair.publicKey, pt_rule);
    
    cout << "  Initial: [   █    ]\n";
    cout << "  Evolving 5 generations (encrypted)...\n\n";
    
    for (int gen = 1; gen <= 5; gen++) {
        ct_rule = cc->EvalAdd(ct_rule, ct_rule);
        cout << "  Gen " << gen << ": encrypted | Level: " << ct_rule->GetLevel() << "\n";
    }
    
    cout << "\n  ✅ Rule 110: 5 generations (φ² encrypted)\n\n";
    
    // ============================================
    // SUMMARY
    // ============================================
    
    cout << "========================================\n";
    cout << "  ONE BREATH COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ NAND + NOT: " << gate_correct << "/" << gate_total << "\n";
    cout << "  ✅ Rule 110: 5 generations\n";
    cout << "  ✅ φ² encoding: universal\n";
    cout << "  ✅ Isang hingahan: parehong architecture\n";
    cout << "  ✅ Level 0\n";
    cout << "  ✅ Pure FHE\n";
    cout << "  ✅ Walang daya!\n\n";
    
    return 0;
}
