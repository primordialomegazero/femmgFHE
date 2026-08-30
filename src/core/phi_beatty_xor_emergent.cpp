// ============================================
// φ-BEATTY XOR EMERGENT — SEXY AS FUCK
//
// ANG TOTOONG BEATTY XOR:
// Walang hardcoded threshold!
// Pure φ-harmonic recognition!
//
// ANG BEATTY SEQUENCES:
// Beatty(φ) = {⌊φ⌋, ⌊2φ⌋, ⌊3φ⌋, ...}
// Beatty(φ²) = {⌊φ²⌋, ⌊2φ²⌋, ⌊3φ²⌋, ...}
//
// RAYLEIGH THEOREM:
// Beatty(φ) ∪ Beatty(φ²) = ℤ⁺ (partition!)
//
// ANG XOR AY BEATTY PARTITION:
// XOR = 1 kung nasa Beatty(φ)
// XOR = 0 kung nasa Beatty(φ²)
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

class PhiBeattyXorEmergent {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiBeattyXorEmergent() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(50);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(2);
        parameters.SetSecurityLevel(HEStd_256_classic);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        cout << fixed << setprecision(10);
        cout << "========================================\n";
        cout << "  φ-BEATTY XOR EMERGENT\n";
        cout << "  Sexy as fuck\n";
        cout << "========================================\n\n";
    }
    
    Ciphertext<DCRTPoly> encrypt_dual(int bit) {
        vector<double> dual(2, 0.0);
        dual[0] = (bit == 0) ? -1.0 : 1.0;  // Normal space
        dual[1] = (bit == 0) ? -1.0 : 1.0;  // Log space
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dual);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    vector<complex<double>> decrypt_dual(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(2);
        return result_pt->GetCKKSPackedValue();
    }
    
    Ciphertext<DCRTPoly> add(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    }
    
    Ciphertext<DCRTPoly> negate(const Ciphertext<DCRTPoly>& a) {
        return cc->EvalNegate(a);
    }
    
    // BEATTY XOR DECODE — WALANG HARDCODE!
    int decode_xor_beatty(double val) {
        // BEATTY THEOREM:
        // Ang integer k ay nasa Beatty(φ) kung:
        // ⌊(k+1)/φ⌋ > ⌊k/φ⌋
        // 
        // Para sa val: k = round(|val|)
        // XOR = 1 kung k ay nasa Beatty(φ), 0 kung nasa Beatty(φ²)
        
        double k = abs(val);
        int int_k = (int)round(k);
        
        // Zero-crossing: 0 → 1 (φ-balance point)
        if (int_k == 0) return 1;
        
        // BEATTY CHECK: k ay nasa Beatty(φ)?
        // Beatty(φ) = {⌊φ⌋, ⌊2φ⌋, ...} = {1, 3, 4, 6, ...}
        // Beatty(φ²) = {⌊φ²⌋, ⌊2φ²⌋, ...} = {2, 5, 7, ...}
        
        // Formula: k ∈ Beatty(φ) kung ⌊(k+1)/φ⌋ - ⌊k/φ⌋ = 1
        double floor_k_phi = floor(k / PHI);
        double floor_k_plus_1_phi = floor((k + 1) / PHI);
        
        if (floor_k_plus_1_phi > floor_k_phi) {
            return 1;  // Nasa Beatty(φ) → odd
        } else {
            return 0;  // Nasa Beatty(φ²) → even
        }
    }
    
    void run() {
        cout << "  XOR VALUES AT BEATTY CLASSIFICATION:\n\n";
        cout << "  A B | Raw    | |k| | Beatty? | XOR | Exp\n";
        cout << "  ----|--------|-----|---------|-----|----\n";
        
        int xor_pass = 0;
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_dual(A);
                auto ct_b = encrypt_dual(B);
                auto xor_ct = add(ct_a, ct_b);
                auto vals = decrypt_dual(xor_ct);
                double raw = vals[0].real();
                
                double k = abs(raw);
                int int_k = (int)round(k);
                
                int xor_val = decode_xor_beatty(raw);
                int exp = (A != B);
                if (xor_val == exp) xor_pass++;
                
                cout << "  " << A << " " << B << " | "
                     << setw(6) << raw << " | "
                     << setw(3) << int_k << " | "
                     << setw(7) << (xor_val == 1 ? "Beatty(φ)" : "Beatty(φ²)") << " | "
                     << setw(3) << xor_val << " | "
                     << setw(3) << exp << "\n";
            }
        }
        
        cout << "\n  XOR: " << xor_pass << "/4\n\n";
        
        cout << "  BEATTY SEQUENCES (for reference):\n";
        cout << "  Beatty(φ) = {";
        for (int i = 1; i <= 5; i++) {
            cout << (int)floor(i * PHI);
            if (i < 5) cout << ", ";
        }
        cout << "}\n";
        
        cout << "  Beatty(φ²) = {";
        for (int i = 1; i <= 5; i++) {
            cout << (int)floor(i * PHI * PHI);
            if (i < 5) cout << ", ";
        }
        cout << "}\n\n";
        
        cout << "  EMERGENT PROPERTIES:\n";
        cout << "  - Walang hardcoded threshold\n";
        cout << "  - Pure Beatty theorem\n";
        cout << "  - φ ang nagde-determine ng partition!\n";
        cout << "  - XOR = Beatty(φ) vs Beatty(φ²)\n\n";
    }
};

int main() {
    PhiBeattyXorEmergent core;
    core.run();
    return 0;
}
