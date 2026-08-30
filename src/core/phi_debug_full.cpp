// ============================================
// φ-DEBUG FULL — MAKIKITA NATIN LAHAT
//
// Debug bawat step:
// 1. Encoding values
// 2. Addition result
// 3. Decoded avg
// 4. Mod φ value
// 5. Distances
// 6. Final decision
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

class PhiDebugFull {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    
public:
    PhiDebugFull() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(1);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(16);
        parameters.SetSecurityLevel(HEStd_256_classic);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        cout << fixed << setprecision(15);
        cout << "========================================\n";
        cout << "  φ-DEBUG FULL — LAHAT NG VALUES\n";
        cout << "========================================\n\n";
    }
    
    vector<double> encode(double bit) {
        vector<double> dims(16, 0.0);
        double val = (bit == 0.0) ? PHI_INV : PHI;
        for (int i = 0; i < 16; i++) dims[i] = val;
        return dims;
    }
    
    Ciphertext<DCRTPoly> encrypt(vector<double> dims) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dims);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    vector<complex<double>> decrypt(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue();
    }
    
    Ciphertext<DCRTPoly> add(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    }
    
    void debug_xor(double A, double B) {
        cout << "========================================\n";
        cout << "  DEBUG: " << A << " XOR " << B << "\n";
        cout << "========================================\n\n";
        
        // Step 1: Show encoding
        auto enc_a = encode(A);
        auto enc_b = encode(B);
        
        cout << "  ENCODING:\n";
        cout << "  A=" << A << " → " << enc_a[0] << " (φ" << (A == 0 ? "⁻¹" : "") << ")\n";
        cout << "  B=" << B << " → " << enc_b[0] << " (φ" << (B == 0 ? "⁻¹" : "") << ")\n\n";
        
        // Step 2: Encrypt
        auto ct_a = encrypt(enc_a);
        auto ct_b = encrypt(enc_b);
        
        cout << "  ENCRYPTED: Level " << ct_a->GetLevel() << "\n\n";
        
        // Step 3: Add
        auto ct_sum = add(ct_a, ct_b);
        
        cout << "  AFTER ADD: Level " << ct_sum->GetLevel() << "\n\n";
        
        // Step 4: Decrypt
        auto vals = decrypt(ct_sum);
        
        cout << "  DECRYPTED VALUES (16 slots):\n";
        cout << "  ";
        for (int i = 0; i < 16; i++) {
            cout << setw(12) << vals[i].real();
            if (i % 4 == 3) cout << "\n  ";
        }
        cout << "\n";
        
        // Step 5: Compute avg
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        
        cout << "  AVG: " << avg << "\n\n";
        
        // Step 6: Expected values
        double expected_avg_00 = 2.0 * PHI_INV;
        double expected_avg_01 = PHI + PHI_INV;
        double expected_avg_11 = 2.0 * PHI;
        
        cout << "  EXPECTED AVG:\n";
        cout << "  0+0 = 2φ⁻¹ = " << expected_avg_00 << "\n";
        cout << "  0+1 = φ+φ⁻¹ = " << expected_avg_01 << "\n";
        cout << "  1+1 = 2φ = " << expected_avg_11 << "\n\n";
        
        cout << "  ACTUAL AVG: " << avg << "\n\n";
        
        // Step 7: Mod φ
        double mod_phi = fmod(avg, PHI);
        cout << "  MOD φ: " << mod_phi << "\n";
        cout << "  φ⁻¹ = " << PHI_INV << "\n";
        cout << "  0 = " << 0.0 << "\n\n";
        
        // Step 8: Distances
        double dist_0 = abs(mod_phi - 0.0);
        double dist_phi_inv = abs(mod_phi - PHI_INV);
        
        cout << "  DISTANCES:\n";
        cout << "  dist(0): " << dist_0 << "\n";
        cout << "  dist(φ⁻¹): " << dist_phi_inv << "\n\n";
        
        // Step 9: Decision
        double result = (dist_phi_inv < dist_0) ? 0.0 : 1.0;
        double expected = fmod(A + B, 2.0);
        
        cout << "  DECISION: " << result << "\n";
        cout << "  EXPECTED: " << expected << "\n";
        cout << "  MATCH: " << (abs(result - expected) < 0.5 ? "✅" : "❌") << "\n\n";
        
        // Step 10: Analysis
        cout << "  ANALYSIS:\n";
        cout << "  - mod_phi = " << mod_phi << "\n";
        cout << "  - Kung mod_phi ≈ 0 → dapat 1\n";
        cout << "  - Kung mod_phi ≈ φ⁻¹ → dapat 0\n";
        cout << "  - Pero ang nakikita natin: " << mod_phi << "\n\n";
        
        if (abs(mod_phi) < 0.001) {
            cout << "  → mod_phi ≈ 0, dapat RESULT=1\n";
        } else if (abs(mod_phi - PHI_INV) < 0.001) {
            cout << "  → mod_phi ≈ φ⁻¹, dapat RESULT=0\n";
        } else {
            cout << "  → mod_phi ay nasa gitna, φ ang bahala\n";
        }
        
        cout << "\n";
    }
    
    void run() {
        cout << "  φ = " << PHI << "\n";
        cout << "  φ⁻¹ = " << PHI_INV << "\n";
        cout << "  φ - φ⁻¹ = " << (PHI - PHI_INV) << "\n";
        cout << "  φ + φ⁻¹ = " << (PHI + PHI_INV) << "\n\n";
        
        debug_xor(0.0, 0.0);
        debug_xor(0.0, 1.0);
        debug_xor(1.0, 0.0);
        debug_xor(1.0, 1.0);
    }
};

int main() {
    PhiDebugFull core;
    core.run();
    return 0;
}
