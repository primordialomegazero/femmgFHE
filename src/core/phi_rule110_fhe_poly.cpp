// ============================================
// φ-RULE 110 FHE POLY — Polynomial Transition
// P(val) = next gamit ang EvalPolyLinear
// Walang EvalMult, walang lookup table
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
    parameters.SetMultiplicativeDepth(2);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    cout << "========================================\n";
    cout << "  φ-RULE 110 FHE POLY — Polynomial\n";
    cout << "========================================\n\n";
    cout << "  P(val) = next gamit ang EvalPolyLinear\n\n";

    // ============================================
    // TEST 1: Polynomial approximation ng transition
    // ============================================
    cout << "  TEST 1: Polynomial approximation\n\n";

    // Ang transition: P(0)=0, P(1)=1, P(2)=1, P(3)=0,
    //                P(4)=1, P(5)=1, P(6)=1, P(7)=0
    // 
    // Hanapin ang degree-2 polynomial na nag-a-approximate:
    // P(val) ≈ a₀ + a₁×val + a₂×val²
    
    // Subukan: P(val) ≈ 0.2 + 0.5×val - 0.1×val² (approx)
    vector<double> coeffs = {0.2, 0.5, -0.1};  // a₀, a₁, a₂

    cout << "  Polynomial: P(val) = " << coeffs[0] << " + "
         << coeffs[1] << "×val + " << coeffs[2] << "×val²\n\n";

    // I-verify sa plaintext
    cout << "  val | P(val) approx | Next | Match?\n";
    cout << "  ----|---------------|------|-------\n";
    for (int val = 0; val < 8; val++) {
        double p_val = coeffs[0] + coeffs[1] * val + coeffs[2] * val * val;
        int next = (val == 0 || val == 3 || val == 7) ? 0 : 1;
        bool match = (p_val >= 0.5) == next;
        
        cout << "  " << setw(3) << val << " | "
             << setw(12) << p_val << " |  "
             << next << "   | "
             << (match ? "✅" : "❌") << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: EvalPolyLinear sa FHE
    // ============================================
    cout << "  TEST 2: EvalPolyLinear sa FHE\n\n";

    // Initial: val=3 (pattern 011) → next=0
    vector<double> val_3(8, 3.0);
    Plaintext pt_3 = cc->MakeCKKSPackedPlaintext(val_3);
    auto ct_3 = cc->Encrypt(keyPair.publicKey, pt_3);

    auto ct_poly = cc->EvalPolyLinear(ct_3, coeffs);
    
    auto v_poly = decrypt_state(ct_poly);
    cout << "  P(3) = " << v_poly[0] << " (expected: ~0 para sa next=0)\n";
    cout << "  Level: " << ct_poly->GetLevel() << "\n\n";

    // ============================================
    // TEST 3: Full transition sa FHE
    // ============================================
    cout << "  TEST 3: Full transition sa FHE\n\n";

    // Initial: 11010101 → pattern values
    vector<double> init = {6.0, 6.0, 3.0, 6.0, 3.0, 6.0, 3.0, 6.0};
    // 110 → 6, 101 → 5, 010 → 2, 101 → 5...
    // Sa totoo lang, ang 8 slots ay may iba't ibang patterns
    
    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "  Initial pattern values:\n  ";
    for (double v : init) cout << setw(4) << v;
    cout << "\n\n";

    // I-apply ang polynomial transition
    auto ct_next = cc->EvalPolyLinear(ct_state, coeffs);
    
    auto v_next = decrypt_state(ct_next);
    
    cout << "  After polynomial transition:\n  ";
    for (int i = 0; i < 8; i++) {
        cout << setw(8) << v_next[i];
    }
    cout << "\n";
    cout << "  Decoded bits: ";
    for (int i = 0; i < 8; i++) {
        cout << (v_next[i] >= 0.5 ? "1" : "0");
    }
    cout << "\n\n";
    cout << "  Level: " << ct_next->GetLevel() << "\n";

    return 0;
}
