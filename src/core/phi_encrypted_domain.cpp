// ============================================
// φ-ENCRYPTED DOMAIN — Loob ng CKKS
// Tingnan ang structure ng ciphertext
// May φ-pattern ba sa encrypted domain?
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

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

    const double PHI = 1.6180339887498948482;

    cout << "========================================\n";
    cout << "  φ-ENCRYPTED DOMAIN — Loob ng CKKS\n";
    cout << "========================================\n\n";

    // ============================================
    // 1. Ang ring dimension at modulus
    // ============================================
    cout << "  --- 1. Ring parameters ---\n\n";
    cout << "  Ring dimension: " << cc->GetRingDimension() << "\n";
    cout << "  Modulus: " << cc->GetModulus() << "\n";
    cout << "  Batch size: " << cc->GetEncodingParams()->GetBatchSize() << "\n\n";

    // ============================================
    // 2. Ang ciphertext structure
    // ============================================
    cout << "  --- 2. Ciphertext structure ---\n\n";

    // I-encrypt ang φ-based na value
    vector<double> vals(8, PHI);
    Plaintext pt_phi = cc->MakeCKKSPackedPlaintext(vals);
    auto ct_phi = cc->Encrypt(keyPair.publicKey, pt_phi);

    // I-access ang ciphertext elements
    auto ct_elements = ct_phi->GetElements();
    
    cout << "  Ciphertext elements: " << ct_elements.size() << "\n";
    cout << "  Element 0 (c0): degree " << ct_elements[0].GetLength() << "\n";
    cout << "  Element 1 (c1): degree " << ct_elements[1].GetLength() << "\n\n";

    // ============================================
    // 3. Ang coefficients — may φ-pattern ba?
    // ============================================
    cout << "  --- 3. Coefficients analysis ---\n\n";
    cout << "  Sample ng c0 coefficients (first 10):\n  ";
    
    auto c0 = ct_elements[0];
    int sample_count = min(10, (int)c0.GetLength());
    
    for (int i = 0; i < sample_count; i++) {
        cout << c0[i] << " ";
    }
    cout << "\n\n";

    // ============================================
    // 4. Ang φ at ang ring dimension
    // ============================================
    cout << "  --- 4. φ at ring dimension ---\n\n";
    
    uint32_t ring_dim = cc->GetRingDimension();
    double phi_pow = pow(PHI, log2(ring_dim));
    
    cout << "  Ring dim = " << ring_dim << "\n";
    cout << "  φ^log2(ring_dim) = " << phi_pow << "\n";
    cout << "  φ ≈ ring_dim^(1/log2(ring_dim))? " 
         << (abs(PHI - pow(ring_dim, 1.0/log2(ring_dim))) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // 5. Ang scaling factor at φ
    // ============================================
    cout << "  --- 5. Scaling factor at φ ---\n\n";
    
    double scaling = pow(2.0, 59);  // 2^59
    double log_phi_scaling = log(scaling) / log(PHI);
    
    cout << "  Scaling = 2^59 = " << scaling << "\n";
    cout << "  log_φ(scaling) = " << log_phi_scaling << "\n";
    cout << "  φ^log_φ(scaling) ≈ scaling? " 
         << (abs(pow(PHI, log_phi_scaling) - scaling) / scaling < 0.01 ? "✅" : "❌") << "\n";

    return 0;
}
