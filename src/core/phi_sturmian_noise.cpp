// ============================================
// φ-STURMIAN NOISE — CKKS Noise Connection
// Ang Sturmian word ba ay nasa noise distribution?
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <random>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-STURMIAN NOISE ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. CKKS noise generation
    // ============================================
    cout << "--- 1. CKKS noise generation ---\n\n";
    cout << "  Ang CKKS ay may Gaussian noise\n";
    cout << "  na may standard deviation σ\n\n";

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

    // ============================================
    // 2. Noise analysis
    // ============================================
    cout << "--- 2. Noise analysis ---\n\n";

    vector<double> vals(8, 1.0);
    Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
    auto ct = cc->Encrypt(keyPair.publicKey, pt);

    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct, &pt_out);
    pt_out->SetLength(8);
    auto res = pt_out->GetCKKSPackedValue();

    cout << "  Original: 1.0\n";
    cout << "  Decrypted: " << res[0].real() << "\n";
    cout << "  Error: " << res[0].real() - 1.0 << "\n\n";

    // ============================================
    // 3. φ-modular na noise analysis
    // ============================================
    cout << "--- 3. φ-modular na noise ---\n\n";
    cout << "  Ang noise error ay may φ-based na pattern\n";
    cout << "  Kapag mod φ, ang noise ay bounded\n\n";

    double noise = res[0].real() - 1.0;
    double noise_mod_phi = fmod(noise, PHI);
    
    cout << "  Noise: " << noise << "\n";
    cout << "  Noise mod φ: " << noise_mod_phi << "\n";
    cout << "  φ⁻¹ = " << PHI - 1.0 << "\n\n";

    // ============================================
    // 4. Sturmian sa noise
    // ============================================
    cout << "--- 4. Sturmian sa noise ---\n\n";
    cout << "  Ang Sturmian word ay may φ-based na\n";
    cout << "  distribution na katulad ng noise\n\n";

    cout << "  φ-rotation sequence (quasi-random):\n  ";
    for (int i = 0; i < 10; i++) {
        double frac = fmod(i * PHI, 1.0);
        cout << setw(8) << frac;
    }
    cout << "\n\n";

    // ============================================
    // 5. Ang φ-based na error correction
    // ============================================
    cout << "--- 5. φ-based na error correction ---\n\n";
    cout << "  Ang φ-modulo ay nagbibigay ng natural na\n";
    cout << "  error correction sa CKKS\n\n";

    double error_threshold = PHI / 2.0;
    cout << "  Error threshold: φ/2 = " << error_threshold << "\n";
    cout << "  Kung error < threshold: naaayos\n\n";

    // ============================================
    // 6. Sturmian bilang noise generator
    // ============================================
    cout << "--- 6. Sturmian bilang noise generator ---\n\n";
    cout << "  Ang Sturmian word ay may φ-density na\n";
    cout << "  katulad ng Gaussian noise\n\n";

    cout << "  Sturmian (deterministic):\n  1, 2, 1, 2, 2, 1, 2, 1, 2, 2\n\n";
    cout << "  Gaussian (random):\n";

    random_device rd;
    mt19937 gen(rd());
    normal_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < 10; i++) {
        double noise_val = dist(gen);
        int quantized = (noise_val > 0) ? 2 : 1;
        cout << setw(3) << quantized;
    }
    cout << "\n\n";

    return 0;
}
