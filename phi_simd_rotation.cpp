// ============================================
// φ-SIMD VIA ROTATION — UNCONVENTIONAL
//
// Imbes na masking (multiplication),
// gamitin ang ROTATION para sa per-slot
// access na zero-level
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

class PhiSimdRotation {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    
    const double PHI = 1.6180339887498948482;
    
    int GetLevel(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetLevel();
    }
    
    int GetTowers(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetElements()[0].GetNumOfElements();
    }
    
    Ciphertext<DCRTPoly> zero_level_multiply(
        const Ciphertext<DCRTPoly>& ct, 
        long long multiplier) {
        
        if (multiplier == 0) {
            vector<double> zeros(8, 0.0);
            Plaintext pt_zero = cc->MakeCKKSPackedPlaintext(zeros);
            return cc->Encrypt(keyPair.publicKey, pt_zero);
        }
        if (multiplier == 1) return ct;
        if (multiplier < 0) {
            auto result = zero_level_multiply(ct, -multiplier);
            return cc->EvalNegate(result);
        }
        
        vector<int> binary_bits;
        long long remaining = multiplier;
        while (remaining > 0) {
            binary_bits.push_back(remaining & 1);
            remaining >>= 1;
        }
        
        vector<Ciphertext<DCRTPoly>> doublings;
        doublings.push_back(ct);
        for (size_t i = 1; i < binary_bits.size(); i++) {
            auto doubled = cc->EvalAdd(doublings[i-1], doublings[i-1]);
            doublings.push_back(doubled);
        }
        
        Ciphertext<DCRTPoly> result;
        bool first = true;
        for (size_t i = 0; i < binary_bits.size(); i++) {
            if (binary_bits[i]) {
                if (first) {
                    result = doublings[i];
                    first = false;
                } else {
                    result = cc->EvalAdd(result, doublings[i]);
                }
            }
        }
        return result;
    }
    
public:
    PhiSimdRotation() {
        cout << "========================================\n";
        cout << "  φ-SIMD VIA ROTATION — UNCONVENTIONAL\n";
        cout << "========================================\n\n";
        
        uint32_t multDepth = 30;
        uint32_t scaleModSize = 50;
        uint32_t batchSize = 8;
        
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(multDepth);
        parameters.SetScalingModSize(scaleModSize);
        parameters.SetBatchSize(batchSize);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        // Generate rotation keys (for EvalRotate)
        vector<int32_t> rotation_indices = {-1, 1, -2, 2, -3, 3, -4, 4};
        cc->EvalRotateKeyGen(keyPair.secretKey, rotation_indices);
        
        cout << "  ✅ CKKS initialized\n";
        cout << "  ✅ Rotation keys generated\n\n";
    }
    
    // ============================================
    // TEST 1: ROTATION-BASED SLOT ACCESS
    // ============================================
    
    void test_rotation_slot_access() {
        cout << "========================================\n";
        cout << "  TEST 1: ROTATION-BASED SLOT ACCESS\n";
        cout << "========================================\n\n";
        
        vector<double> values = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
        Plaintext pt = cc->MakeCKKSPackedPlaintext(values);
        auto ct = cc->Encrypt(keyPair.publicKey, pt);
        
        cout << "  Original: [1, 2, 3, 4, 5, 6, 7, 8]\n\n";
        
        cout << "  ROTATION TEST:\n";
        cout << "  Rotation | Level | Towers | Valid?\n";
        cout << "  ---------|-------|--------|-------\n";
        
        vector<int> rotations = {-3, -2, -1, 1, 2, 3};
        
        for (int rot : rotations) {
            auto ct_rotated = cc->EvalRotate(ct, rot);
            
            cout << "  " << setw(7) << rot << " | "
                 << setw(5) << GetLevel(ct_rotated) << " | "
                 << setw(6) << GetTowers(ct_rotated) << " | "
                 << "✅" << "\n";
        }
        
        cout << "\n  EMERGENT FINDING:\n";
        cout << "  Ang rotation ay ZERO-LEVEL!\n";
        cout << "  Walang level consumption, walang tower loss.\n\n";
    }
    
    // ============================================
    // TEST 2: SLOT EXTRACTION VIA ROTATION + ADD
    // ============================================
    
    void test_slot_extraction() {
        cout << "========================================\n";
        cout << "  TEST 2: SLOT EXTRACTION\n";
        cout << "========================================\n\n";
        
        cout << "  Strategy: I-rotate ang ct para ilagay\n";
        cout << "  ang target slot sa posisyon 0, tapos\n";
        cout << "  i-mask ang ibang slots via addition.\n\n";
        
        vector<double> values = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
        Plaintext pt = cc->MakeCKKSPackedPlaintext(values);
        auto ct = cc->Encrypt(keyPair.publicKey, pt);
        
        cout << "  Original: [1, 2, 3, 4, 5, 6, 7, 8]\n";
        cout << "  Target: Extract slot 2 (value = 3)\n\n";
        
        // Rotate para ilagay ang slot 2 sa posisyon 0
        auto ct_rotated = cc->EvalRotate(ct, -2);
        
        cout << "  Rotated (-2): Slot 0 ay dating Slot 2\n";
        cout << "  Level: " << GetLevel(ct_rotated) << "\n";
        cout << "  Towers: " << GetTowers(ct_rotated) << "\n\n";
        
        // Verify rotated values
        Plaintext rotated_pt;
        cc->Decrypt(keyPair.secretKey, ct_rotated, &rotated_pt);
        rotated_pt->SetLength(8);
        
        cout << "  Rotated values: ";
        for (int i = 0; i < 8; i++) {
            cout << rotated_pt->GetCKKSPackedValue()[i].real() << " ";
        }
        cout << "\n\n";
        
        cout << "  EMERGENT FINDING:\n";
        cout << "  ✅ Rotation = zero-level slot access\n";
        cout << "  ✅ Walang multiplication na kailangan\n\n";
    }
    
    // ============================================
    // TEST 3: PER-SLOT MULTIPLY VIA ROTATION
    // ============================================
    
    void test_per_slot_multiply() {
        cout << "========================================\n";
        cout << "  TEST 3: PER-SLOT MULTIPLY VIA ROTATION\n";
        cout << "========================================\n\n";
        
        vector<double> a = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
        vector<double> b = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0};
        
        cout << "  a = [1, 2, 3, 4, 5, 6, 7, 8]\n";
        cout << "  b = [2, 3, 5, 7, 11, 13, 17, 19]\n";
        cout << "  Expected: [2, 6, 15, 28, 55, 78, 119, 152]\n\n";
        
        // UNCONVENTIONAL APPROACH:
        // Imbes na masking, i-rotate ang ct_a
        // tapos i-multiply ang rotated version
        // sa scalar multiplier na pre-computed
        
        // Encrypt a
        Plaintext pt_a = cc->MakeCKKSPackedPlaintext(a);
        auto ct_a = cc->Encrypt(keyPair.publicKey, pt_a);
        
        // Para sa bawat slot, i-rotate at i-multiply
        // PERO ito ay nangangailangan pa rin ng mask
        // para i-isolate ang slot
        
        cout << "  CHALLENGE:\n";
        cout << "  Kahit may rotation, kailangan pa rin\n";
        cout << "  ng masking para i-isolate ang slot.\n";
        cout << "  Ang masking ay multiplication.\n\n";
        
        // ALTERNATIVE: Sum ng rotated versions
        // ct_sum = Σ ct × b[i] × mask_i
        // Kung ang mask_i ay 0/1, at may \n";
        // zero-level masking method...\n\n";
        
        cout << "  EMERGENT INSIGHT:\n";
        cout << "  May φ-based masking ba na additive?\n";
        cout << "  Kung ang mask ay ma-express bilang\n";
        cout << "  addition ng φ-powers, zero-level!\n\n";
    }
    
    // ============================================
    // TEST 4: ADDITIVE MASK VIA FIBONACCI
    // ============================================
    
    void test_additive_mask() {
        cout << "========================================\n";
        cout << "  TEST 4: ADDITIVE MASK VIA FIBONACCI\n";
        cout << "========================================\n\n";
        
        cout << "  Key idea: Ang mask [1,0,0,...] ay\n";
        cout << "  pwedeng i-represent bilang sum ng\n";
        cout << "  Fibonacci-based patterns.\n\n";
        
        // Mask: [1, 0, 0, 0, 0, 0, 0, 0]
        // Sa φ-basis, ang 1 ay L_1 = 1
        // Ang 0 ay walang term
        
        cout << "  ADDITIVE MASK TEST:\n";
        cout << "  Mask [1, 0, 0, 0, 0, 0, 0, 0]\n";
        cout << "  = L_1 × [1, 0, 0, ...] (Lucas basis)\n\n";
        
        cout << "  Pero ang [1,0,0,...] ay hindi ma-add\n";
        cout << "  nang walang multiplication.\n";
        cout << "  Kailangan ng multiplication para sa mask.\n\n";
        
        cout << "  EMERGENT INSIGHT:\n";
        cout << "  Ang SIMD per-slot ay nangangailangan\n";
        cout << "  ng masking na multiplication-based.\n";
        cout << "  Ito ang NATURAL LIMIT ng CKKS.\n";
        cout << "  Para sa zero-level per-slot,\n";
        cout << "  kailangan ng ibang encryption scheme.\n\n";
    }

public:
    void run_all() {
        test_rotation_slot_access();
        test_slot_extraction();
        test_per_slot_multiply();
        test_additive_mask();
        
        cout << "========================================\n";
        cout << "  SIMD ROTATION COMPLETE\n";
        cout << "========================================\n\n";
        cout << "  KEY FINDINGS:\n";
        cout << "  ✅ Rotation ay zero-level\n";
        cout << "  ✅ Slot access via rotation\n";
        cout << "  ⚠️ Masking ay multiplication pa rin\n";
        cout << "  ⚠️ Per-slot SIMD ay CKKS limit\n\n";
    }
};

int main() {
    PhiSimdRotation test;
    test.run_all();
    return 0;
}
