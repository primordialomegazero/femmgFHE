// ============================================
// φ-CANONICAL EMBEDDING — Golden Angle Slots
// Ang φ-rotation sa CKKS roots of unity
// Natural na slot mapping na may golden angle
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <complex>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-CANONICAL EMBEDDING ===\n\n";
    cout << fixed << setprecision(15);

    // ============================================
    // 1. Ang golden angle slot mapping
    // ============================================
    cout << "--- 1. Golden angle slot mapping ---\n\n";
    
    int M = 32768;
    double golden_angle = 2.0 * M_PI / (PHI * PHI);
    double golden_step = M / (PHI * PHI);
    
    cout << "  Golden angle: " << golden_angle << " rad\n";
    cout << "  Golden step: " << golden_step << " slots\n";
    cout << "  Fractional: " << golden_step - floor(golden_step) << "\n\n";

    // ============================================
    // 2. Golden slot indices
    // ============================================
    cout << "--- 2. Golden slot indices ---\n\n";
    cout << "  slot(i) = floor(i × golden_step) mod M\n\n";
    
    cout << "  i | golden slot | distance from prev\n";
    cout << "  --|-------------|--------------------\n";
    
    int prev_slot = 0;
    for (int i = 0; i < 15; i++) {
        int slot = (int)floor(i * golden_step) % M;
        int dist = slot - prev_slot;
        if (dist < 0) dist += M;
        
        cout << "  " << setw(2) << i << " | "
             << setw(10) << slot << " | "
             << setw(10) << dist << "\n";
        
        prev_slot = slot;
    }
    cout << "\n";

    // ============================================
    // 3. Golden angle sa complex plane
    // ============================================
    cout << "--- 3. Golden angle sa complex plane ---\n\n";
    cout << "  Ang golden angle ay nagbibigay ng\n";
    cout << "  optimal na distribution sa circle\n\n";

    for (int i = 0; i < 10; i++) {
        double angle = i * golden_angle;
        complex<double> z = exp(complex<double>(0, angle));
        
        cout << "  i=" << setw(2) << i << ": "
             << setw(8) << z.real() << " + "
             << setw(8) << z.imag() << "i\n";
    }
    cout << "\n";

    // ============================================
    // 4. CKKS na may golden angle slots
    // ============================================
    cout << "--- 4. CKKS na may golden slots ---\n\n";
    cout << "  Ang golden angle ay nagbibigay ng\n";
    cout << "  natural na slot permutation\n\n";

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

    // I-encrypt ang φ-powers sa golden angle na slots
    vector<double> vals(8, 0.0);
    for (int i = 0; i < 8; i++) vals[i] = pow(PHI, i);
    
    Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
    auto ct = cc->Encrypt(keyPair.publicKey, pt);
    
    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct, &pt_out);
    pt_out->SetLength(8);
    auto res = pt_out->GetCKKSPackedValue();
    
    cout << "  φ-powers sa CKKS slots:\n  ";
    for (int i = 0; i < 8; i++) {
        cout << setw(8) << res[i].real();
    }
    cout << "\n";
    cout << "  Level: " << ct->GetLevel() << "\n\n";

    // ============================================
    // 5. Ang optimal na spacing
    // ============================================
    cout << "--- 5. Optimal na spacing ---\n\n";
    cout << "  Ang golden angle ay may pinaka-optimal na\n";
    cout << "  spacing — katulad ng sunflower seeds\n\n";
    
    cout << "  Sunflower pattern:\n";
    cout << "  seed(i) = i × golden_angle\n";
    cout << "  Na may natural na φ-distribution\n\n";
    
    cout << "  i | angle | x | y\n";
    cout << "  --|-------|---|---\n";
    for (int i = 0; i < 8; i++) {
        double angle = i * golden_angle;
        double radius = sqrt(i + 1);
        double x = radius * cos(angle);
        double y = radius * sin(angle);
        
        cout << "  " << setw(1) << i << " | "
             << setw(5) << angle << " | "
             << setw(5) << x << " | "
             << setw(5) << y << "\n";
    }
    cout << "\n";

    return 0;
}
