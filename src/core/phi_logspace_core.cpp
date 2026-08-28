// ============================================
// φ-LOG SPACE FHE — CORE IMPLEMENTATION
//
// Encryption: ct = Encrypt(log_φ(value))
// Multiplication: ct_add(ct_a, ct_b) = ct_{a×b}
// Division: ct_sub(ct_a, ct_b) = ct_{a/b}
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

class PhiLogSpaceFHE {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    int GetLevel(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetLevel();
    }
    
    int GetTowers(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetElements()[0].GetNumOfElements();
    }
    
public:
    PhiLogSpaceFHE() {
        uint32_t multDepth = 50;
        uint32_t scaleModSize = 50;
        uint32_t batchSize = 1;
        
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(multDepth);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(1);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
    }
    
    // ============================================
    // ENCRYPT / DECRYPT SA LOG SPACE
    // ============================================
    
    Ciphertext<DCRTPoly> encrypt_log(double value) {
        double log_phi_value = log(value) / LN_PHI;
        vector<double> val(1, log_phi_value);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(val);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    double decrypt_to_value(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        double log_val = result_pt->GetCKKSPackedValue()[0].real();
        return pow(PHI, log_val);
    }
    
    // ============================================
    // ZERO-LEVEL OPERATIONS SA LOG SPACE
    // ============================================
    
    // Multiply: a × b = exp(log(a) + log(b))
    Ciphertext<DCRTPoly> multiply(
        const Ciphertext<DCRTPoly>& ct_a,
        const Ciphertext<DCRTPoly>& ct_b) {
        return cc->EvalAdd(ct_a, ct_b);  // ZERO-LEVEL!
    }
    
    // Divide: a / b = exp(log(a) - log(b))
    Ciphertext<DCRTPoly> divide(
        const Ciphertext<DCRTPoly>& ct_a,
        const Ciphertext<DCRTPoly>& ct_b) {
        auto neg_b = cc->EvalNegate(ct_b);
        return cc->EvalAdd(ct_a, neg_b);  // ZERO-LEVEL!
    }
    
    // ============================================
    // GETTERS
    // ============================================
    
    int get_level(const Ciphertext<DCRTPoly>& ct) {
        return GetLevel(ct);
    }
    
    int get_towers(const Ciphertext<DCRTPoly>& ct) {
        return GetTowers(ct);
    }
};

int main() {
    cout << "========================================\n";
    cout << "  φ-LOG SPACE FHE — CORE\n";
    cout << "========================================\n\n";
    
    PhiLogSpaceFHE fhe;
    
    cout << "  ✅ Core initialized\n";
    cout << "  φ = " << fixed << setprecision(15) << 1.6180339887498948482 << "\n";
    cout << "  ln(φ) = " << log(1.6180339887498948482) << "\n\n";
    
    cout << "  CORE READY\n";
    return 0;
}
