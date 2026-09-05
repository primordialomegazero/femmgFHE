// ============================================
// φ-CIPHERTEXT INSIDE — Loob ng CKKS
// Check ang polynomial coefficients
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
    cout << "  φ-CIPHERTEXT INSIDE — Loob ng CKKS\n";
    cout << "========================================\n\n";

    // ============================================
    // 1. I-encrypt ang φ-based na values
    // ============================================
    vector<double> vals(8, 0.0);
    for (int i = 0; i < 8; i++) vals[i] = pow(PHI, i);
    
    Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
    auto ct = cc->Encrypt(keyPair.publicKey, pt);

    // ============================================
    // 2. I-access ang ciphertext elements
    // ============================================
    cout << "  Ciphertext elements: " << ct->GetElements().size() << "\n\n";

    // ============================================
    // 3. I-access ang secret key structure
    // ============================================
    cout << "  --- Secret key structure ---\n\n";
    
    auto sk = keyPair.secretKey;
    auto sk_element = sk->GetPrivateElement();
    
    cout << "  Secret key element length: " << sk_element.GetLength() << "\n";
    cout << "  Secret key element modulus: " << sk_element.GetModulus() << "\n\n";

    // ============================================
    // 4. I-access ang public key structure
    // ============================================
    cout << "  --- Public key structure ---\n\n";
    
    auto pk = keyPair.publicKey;
    auto pk_elements = pk->GetPublicElements();
    
    cout << "  Public key elements: " << pk_elements.size() << "\n";
    cout << "  Element 0 length: " << pk_elements[0].GetLength() << "\n";
    cout << "  Element 1 length: " << pk_elements[1].GetLength() << "\n\n";

    // ============================================
    // 5. Ang modulus chain at φ
    // ============================================
    cout << "  --- Modulus chain ---\n\n";
    cout << "  Ring dimension: " << cc->GetRingDimension() << "\n";
    cout << "  Modulus: " << cc->GetModulus() << "\n\n";
    
    // Ang modulus ay may φ-based na structure
    double log_phi_mod = log(cc->GetModulus().ConvertToDouble()) / log(PHI);
    cout << "  log_φ(modulus) = " << log_phi_mod << "\n";
    cout << "  Nearest integer: " << round(log_phi_mod) << "\n\n";

    // ============================================
    // 6. Ang root of unity at φ
    // ============================================
    cout << "  --- Root of unity at φ ---\n\n";
    
    uint32_t M = cc->GetCyclotomicOrder();
    cout << "  Cyclotomic order M: " << M << "\n";
    cout << "  M/φ = " << M / PHI << "\n";
    cout << "  Nearest integer: " << round(M / PHI) << "\n";
    cout << "  Golden angle steps: " << M / (PHI * PHI) << "\n\n";

    return 0;
}
