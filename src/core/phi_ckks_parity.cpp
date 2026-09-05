// ============================================
// φ-CKKS PARITY — Natural na Encryption
// Ang φ-parity cycle sa CKKS slots
// Deterministic na φ-based na obfuscation
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
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-CKKS PARITY ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. φ-parity bilang slot permutation
    // ============================================
    cout << "--- 1. φ-parity slot permutation ---\n\n";
    cout << "  Ang φ-parity ay nagbibigay ng natural\n";
    cout << "  na slot permutation sa CKKS\n\n";

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

    // I-encrypt ang φ-powers na may parity-based na obfuscation
    vector<double> vals(8, 0.0);
    for (int i = 0; i < 8; i++) {
        double mod_phi = fmod(pow(PHI, i), PHI);
        int parity = (mod_phi > 0.5) ? 1 : 0;
        // Obfuscate: multiply by (-1)^parity
        vals[i] = pow(PHI, i) * (parity ? 1.0 : -1.0);
    }

    Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
    auto ct = cc->Encrypt(keyPair.publicKey, pt);

    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct, &pt_out);
    pt_out->SetLength(8);
    auto res = pt_out->GetCKKSPackedValue();

    cout << "  φ-powers na may φ-parity obfuscation:\n  ";
    for (int i = 0; i < 8; i++) {
        cout << setw(10) << res[i].real();
    }
    cout << "\n\n";

    // ============================================
    // 2. Decryption ng obfuscated values
    // ============================================
    cout << "--- 2. Decryption ---\n\n";
    cout << "  Ang parity ay may natural na recovery:\n";
    cout << "  |val| = φ^i, sign = (-1)^parity\n\n";

    cout << "  i | Recovered | Original φ^i\n";
    cout << "  --|-----------|---------------\n";
    for (int i = 0; i < 8; i++) {
        double recovered = abs(res[i].real());
        double original = pow(PHI, i);
        
        cout << "  " << setw(2) << i << " | "
             << setw(10) << recovered << " | "
             << setw(10) << original << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. φ-parity bilang natural na key
    // ============================================
    cout << "--- 3. φ-parity bilang key ---\n\n";
    cout << "  Ang φ-parity key ay deterministic:\n";
    cout << "  1, 0, 1, 1, 0, 1, 0, 1\n\n";

    cout << "  Key bits: ";
    for (int i = 0; i < 8; i++) {
        double mod_phi = fmod(pow(PHI, i), PHI);
        int bit = (mod_phi > 0.5) ? 1 : 0;
        cout << bit;
    }
    cout << "\n\n";

    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
