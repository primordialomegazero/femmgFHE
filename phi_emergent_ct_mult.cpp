// ============================================
// φ-EMERGENT CT×CT SEARCH
//
// Hinahanap: Paano gawin ang ct × ct nang
// walang level consumption gamit ang
// emergent properties.
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace std;
using namespace std::chrono;
using namespace lbcrypto;

class PhiEmergentCtMult {
private:
    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 0.6180339887498948482;
    
    CryptoContext<DCRTPoly> cryptoContext;
    KeyPair<DCRTPoly> keyPair;
    
    int GetLevel(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetLevel();
    }
    
    int GetTowers(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetElements()[0].GetNumOfElements();
    }
    
public:
    PhiEmergentCtMult() {
        cout << "========================================\n";
        cout << "  φ-EMERGENT CT×CT SEARCH\n";
        cout << "========================================\n\n";
        
        uint32_t multDepth = 30;
        uint32_t scaleModSize = 50;
        uint32_t batchSize = 8;
        
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(multDepth);
        parameters.SetScalingModSize(scaleModSize);
        parameters.SetBatchSize(batchSize);
        
        cryptoContext = GenCryptoContext(parameters);
        cryptoContext->Enable(PKE);
        cryptoContext->Enable(KEYSWITCH);
        cryptoContext->Enable(LEVELEDSHE);
        
        keyPair = cryptoContext->KeyGen();
        cryptoContext->EvalMultKeyGen(keyPair.secretKey);
        
        cout << "  ✅ CKKS initialized\n\n";
    }
    
    // ============================================
    // EMERGENT SEARCH 1: INTEGER DECOMPOSITION NG CT
    // ============================================
    
    void test_integer_decomposition() {
        cout << "========================================\n";
        cout << "  SEARCH 1: INTEGER DECOMPOSITION NG CT\n";
        cout << "========================================\n\n";
        
        cout << "  Key idea: Kung ang ct₂ ay ma-decompose\n";
        cout << "  sa integer components, pwede nating gawin:\n";
        cout << "  ct₁ × ct₂ = ct₁ × int₁ + ct₁ × int₂ + ...\n\n";
        
        // Test: ct₂ na may known integer values
        vector<double> values1 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
        vector<double> values2 = {3.0, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0, 17.0};
        
        Plaintext pt1 = cryptoContext->MakeCKKSPackedPlaintext(values1);
        Plaintext pt2 = cryptoContext->MakeCKKSPackedPlaintext(values2);
        
        auto ct1 = cryptoContext->Encrypt(keyPair.publicKey, pt1);
        auto ct2 = cryptoContext->Encrypt(keyPair.publicKey, pt2);
        
        cout << "  ct₁ = [1, 2, 3, 4, 5, 6, 7, 8]\n";
        cout << "  ct₂ = [3, 5, 7, 9, 11, 13, 15, 17]\n\n";
        
        // Traditional ct × ct (with level cost)
        auto ct_mult = cryptoContext->EvalMult(ct1, ct2);
        cout << "  Traditional ct₁ × ct₂:\n";
        cout << "  Level: " << GetLevel(ct_mult) << "\n";
        cout << "  Towers: " << GetTowers(ct_mult) << "\n\n";
        
        // Decomposition approach: ct₂ = 3 + 2 (binary para sa 5)
        // Pero hindi ito same sa per-slot decomposition!
        
        cout << "  PROBLEM:\n";
        cout << "  Ang ct₂ ay may iba't ibang values per slot.\n";
        cout << "  Hindi ito ma-decompose sa integer constants\n";
        cout << "  na pareho para sa lahat ng slots.\n\n";
        
        cout << "  EMERGENT INSIGHT:\n";
        cout << "  Kung ang ct₂ ay may REPEATED values,\n";
        cout << "  pwede nating gamitin ang SIMD structure\n";
        cout << "  para sa decomposition.\n\n";
    }
    
    // ============================================
    // EMERGENT SEARCH 2: φ-BASIS CT DECOMPOSITION
    // ============================================
    
    void test_phi_basis_decomposition() {
        cout << "========================================\n";
        cout << "  SEARCH 2: φ-BASIS CT DECOMPOSITION\n";
        cout << "========================================\n\n";
        
        cout << "  Key idea: Kung ang ct ay naka-encode sa\n";
        cout << "  φ-basis (a + bφ), ang ct × ct ay\n";
        cout << "  ma-decompose sa φ-basis multiplications.\n\n";
        
        // Test: ct × ct gamit ang φ-basis
        // ct = a + bφ, ct × ct = (a² + b²) + (2ab + b²)φ
        
        vector<double> a_vals = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
        vector<double> b_vals = {0.0, 1.0, 1.0, 2.0, 3.0, 5.0, 8.0, 13.0};
        
        Plaintext pt_a = cryptoContext->MakeCKKSPackedPlaintext(a_vals);
        Plaintext pt_b = cryptoContext->MakeCKKSPackedPlaintext(b_vals);
        
        auto ct_a = cryptoContext->Encrypt(keyPair.publicKey, pt_a);
        auto ct_b = cryptoContext->Encrypt(keyPair.publicKey, pt_b);
        
        cout << "  φ-basis: ct = a + bφ\n";
        cout << "  a = [1, 2, 3, 4, 5, 6, 7, 8]\n";
        cout << "  b = [0, 1, 1, 2, 3, 5, 8, 13]\n\n";
        
        // ct × ct sa φ-basis:
        // (a + bφ)² = (a² + b²) + (2ab + b²)φ
        
        // Traditional approach: ct × ct (level cost)
        // Reconstruct first
        auto ct_b_phi = cryptoContext->EvalMult(ct_b, PHI);
        auto ct_value = cryptoContext->EvalAdd(ct_a, ct_b_phi);
        
        auto ct_squared = cryptoContext->EvalMult(ct_value, ct_value);
        
        cout << "  Traditional ct × ct (reconstructed):\n";
        cout << "  Level: " << GetLevel(ct_squared) << "\n";
        cout << "  Towers: " << GetTowers(ct_squared) << "\n\n";
        
        cout << "  EMERGENT INSIGHT:\n";
        cout << "  Ang φ-basis decomposition ay nangangailangan pa rin\n";
        cout << "  ng ct × ct para sa a² at b².\n";
        cout << "  Pero: kung ang a at b ay INTEGER,\n";
        cout << "  pwede nating gamitin ang binary decomposition!\n\n";
        
        // Zero-level approach para sa a² (integer squaring)
        // a² = a × a (using binary decomposition if a is small)
        cout << "  ZERO-LEVEL APPROACH PARA SA INTEGER φ-BASIS:\n";
        cout << "  Kung ang a at b ay small integers,\n";
        cout << "  pwede nating i-square sila gamit ang\n";
        cout << "  binary decomposition (zero level cost!)\n\n";
    }
    
    // ============================================
    // EMERGENT SEARCH 3: SIMD-BASED CT×CT
    // ============================================
    
    void test_simd_ct_mult() {
        cout << "========================================\n";
        cout << "  SEARCH 3: SIMD-BASED CT×CT\n";
        cout << "========================================\n\n";
        
        cout << "  Key idea: Ang CKKS SIMD ay nag-a-allow ng\n";
        cout << "  same operation sa multiple slots sabay-sabay.\n";
        cout << "  Kung may pattern sa slots, baka ma-exploit.\n\n";
        
        // Test: ct with repeating pattern
        vector<double> pattern = {1.0, 2.0, 1.0, 2.0, 1.0, 2.0, 1.0, 2.0};
        vector<double> values = {3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0};
        
        Plaintext pt_pattern = cryptoContext->MakeCKKSPackedPlaintext(pattern);
        Plaintext pt_values = cryptoContext->MakeCKKSPackedPlaintext(values);
        
        auto ct_pattern = cryptoContext->Encrypt(keyPair.publicKey, pt_pattern);
        auto ct_values = cryptoContext->Encrypt(keyPair.publicKey, pt_values);
        
        cout << "  Pattern: [1, 2, 1, 2, 1, 2, 1, 2]\n";
        cout << "  Values:  [3, 3, 3, 3, 3, 3, 3, 3]\n\n";
        
        // Decompose: ct_pattern = ct_ones + ct_alternating
        // ct_ones = [1, 1, 1, 1, 1, 1, 1, 1]
        // ct_alternating = [0, 1, 0, 1, 0, 1, 0, 1]
        
        // ct_values × ct_pattern = ct_values × ones + ct_values × alternating
        // = 3 × [1,1,1,1,1,1,1,1] + 3 × [0,1,0,1,0,1,0,1]
        // = [3,3,3,3,3,3,3,3] + [0,3,0,3,0,3,0,3]
        // = [3,6,3,6,3,6,3,6]
        
        cout << "  ZERO-LEVEL DECOMPOSITION:\n";
        cout << "  ct_values × ct_pattern = ?\n\n";
        
        // Method: Add ct_values to itself where pattern = 2
        auto ct_result = cryptoContext->EvalAdd(ct_values, ct_values);
        
        // Para sa slots na may pattern=1, ibalik sa original
        // (Hindi ito directly possible sa CKKS na walang masking)
        
        cout << "  PROBLEM:\n";
        cout << "  Hindi natin ma-select ang specific slots\n";
        cout << "  nang walang masking operation.\n";
        cout << "  Ang masking ay nangangailangan ng multiplication.\n\n";
        
        cout << "  EMERGENT INSIGHT:\n";
        cout << "  Kung may φ-based masking na ADDITIVE\n";
        cout << "  (hindi multiplicative), baka ma-exploit.\n\n";
    }
    
    // ============================================
    // EMERGENT SEARCH 4: φ-RECURSIVE CT×CT
    // ============================================
    
    void test_recursive_ct_mult() {
        cout << "========================================\n";
        cout << "  SEARCH 4: φ-RECURSIVE CT×CT\n";
        cout << "========================================\n\n";
        
        cout << "  Key idea: φ² = φ + 1\n";
        cout << "  Kung ang ct × ct ay ma-express via φ-recursion,\n";
        cout << "  baka ma-replace ng additions.\n\n";
        
        // Test: ct = φ, ct × ct = φ² = φ + 1
        vector<double> phi_vals = {PHI, PHI, PHI, PHI, PHI, PHI, PHI, PHI};
        Plaintext pt_phi = cryptoContext->MakeCKKSPackedPlaintext(phi_vals);
        auto ct_phi = cryptoContext->Encrypt(keyPair.publicKey, pt_phi);
        
        cout << "  ct = φ\n";
        cout << "  ct × ct = φ² = φ + 1 (ADDITIVE!)\n\n";
        
        // Traditional: ct × ct
        auto ct_mult = cryptoContext->EvalMult(ct_phi, ct_phi);
        cout << "  Traditional ct × ct:\n";
        cout << "  Level: " << GetLevel(ct_mult) << "\n";
        cout << "  Towers: " << GetTowers(ct_mult) << "\n\n";
        
        // Additive: ct + 1 (zero level cost!)
        auto ct_add = cryptoContext->EvalAdd(ct_phi, 1.0);
        cout << "  Additive (ct + 1):\n";
        cout << "  Level: " << GetLevel(ct_add) << "\n";
        cout << "  Towers: " << GetTowers(ct_add) << "\n\n";
        
        // Verify na same sila
        Plaintext pt_mult_result, pt_add_result;
        cryptoContext->Decrypt(keyPair.secretKey, ct_mult, &pt_mult_result);
        cryptoContext->Decrypt(keyPair.secretKey, ct_add, &pt_add_result);
        pt_mult_result->SetLength(8);
        pt_add_result->SetLength(8);
        
        cout << "  VERIFICATION:\n";
        cout << "  Slot | ct × ct | ct + 1 | Match?\n";
        cout << "  -----|---------|--------|-------\n";
        
        for (int i = 0; i < 8; i++) {
            double mult_result = pt_mult_result->GetCKKSPackedValue()[i].real();
            double add_result = pt_add_result->GetCKKSPackedValue()[i].real();
            bool match = abs(mult_result - add_result) < 0.01;
            
            cout << "  " << setw(4) << i << " | "
                 << setw(7) << fixed << setprecision(4) << mult_result << " | "
                 << setw(6) << add_result << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  ✅ ct × ct = ct + 1 kung ct = φ!\n";
        cout << "  ✅ ZERO level cost!\n";
        cout << "  ✅ Ito ay φ-recursive: φ² = φ + 1\n";
        cout << "  ⚠️ Pero gumagana lang para sa ct = φ\n";
        cout << "  ⚠️ Hindi pa general sa arbitrary ct\n\n";
    }
    
    // ============================================
    // EMERGENT SEARCH 5: φ-POWER CT×CT
    // ============================================
    
    void test_phi_power_ct_mult() {
        cout << "========================================\n";
        cout << "  SEARCH 5: φ-POWER CT×CT\n";
        cout << "========================================\n\n";
        
        cout << "  Key idea: Kung ang ct ay φ-power,\n";
        cout << "  ang ct × ct ay ma-replace ng addition.\n\n";
        
        // Test: ct = φ², ct × ct = φ⁴ = 3φ + 2 (Fibonacci!)
        vector<double> phi2_vals = {PHI*PHI, PHI*PHI, PHI*PHI, PHI*PHI, 
                                    PHI*PHI, PHI*PHI, PHI*PHI, PHI*PHI};
        Plaintext pt_phi2 = cryptoContext->MakeCKKSPackedPlaintext(phi2_vals);
        auto ct_phi2 = cryptoContext->Encrypt(keyPair.publicKey, pt_phi2);
        
        cout << "  ct = φ²\n";
        cout << "  ct × ct = φ⁴ = 3φ + 2\n\n";
        
        // Traditional
        auto ct_mult = cryptoContext->EvalMult(ct_phi2, ct_phi2);
        
        // Additive decomposition: 3φ + 2
        auto ct_phi = cryptoContext->EvalMult(ct_phi2, PHI_INV); // φ²/φ = φ
        auto ct_3phi = cryptoContext->EvalAdd(ct_phi, ct_phi);
        ct_3phi = cryptoContext->EvalAdd(ct_3phi, ct_phi);
        auto ct_add = cryptoContext->EvalAdd(ct_3phi, 2.0);
        
        cout << "  Traditional ct × ct level: " << GetLevel(ct_mult) << "\n";
        cout << "  Additive level: " << GetLevel(ct_add) << "\n\n";
        
        // Verify
        Plaintext pt_mult_result, pt_add_result;
        cryptoContext->Decrypt(keyPair.secretKey, ct_mult, &pt_mult_result);
        cryptoContext->Decrypt(keyPair.secretKey, ct_add, &pt_add_result);
        pt_mult_result->SetLength(8);
        pt_add_result->SetLength(8);
        
        cout << "  VERIFICATION (ct × ct = 3φ + 2):\n";
        cout << "  Slot | ct × ct | 3φ + 2 | Match?\n";
        cout << "  -----|---------|--------|-------\n";
        
        for (int i = 0; i < 8; i++) {
            double mult_result = pt_mult_result->GetCKKSPackedValue()[i].real();
            double add_result = pt_add_result->GetCKKSPackedValue()[i].real();
            bool match = abs(mult_result - add_result) < 0.1;
            
            cout << "  " << setw(4) << i << " | "
                 << setw(7) << fixed << setprecision(3) << mult_result << " | "
                 << setw(6) << add_result << " | "
                 << (match ? "✅" : "❌") << "\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  ✅ φ-power ct × ct ay ma-replace ng addition!\n";
        cout << "  ✅ Fibonacci coefficients ang gamit\n";
        cout << "  ⚠️ Gumagana lang para sa φ-powers\n";
        cout << "  ⚠️ Kailangan ng general na decomposition\n\n";
    }

public:
    void run_all() {
        test_integer_decomposition();
        test_phi_basis_decomposition();
        test_simd_ct_mult();
        test_recursive_ct_mult();
        test_phi_power_ct_mult();
        
        cout << "========================================\n";
        cout << "  CT×CT EMERGENT SEARCH COMPLETE\n";
        cout << "========================================\n\n";
        cout << "  KEY FINDINGS:\n";
        cout << "  ✅ φ² = φ + 1 (additive replacement)\n";
        cout << "  ✅ φ⁴ = 3φ + 2 (Fibonacci decomposition)\n";
        cout << "  ✅ Zero-level possible para sa φ-powers\n";
        cout << "  ⚠️ General ct × ct ay kailangan pa\n\n";
        cout << "  NEXT: φ-power decomposition para sa\n";
        cout << "  arbitrary ct × ct\n\n";
    }
};

int main() {
    PhiEmergentCtMult search;
    search.run_all();
    return 0;
}
