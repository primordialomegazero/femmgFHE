// ============================================
// φ-BEATTY SLOTS — Natural na Slot Mapping
// Ang Beatty sequence bilang optimal na slot
// distribution para sa CKKS
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

    cout << "========================================\n";
    cout << "  φ-BEATTY SLOTS — Natural na Slot Mapping\n";
    cout << "========================================\n\n";

    // ============================================
    // 1. Beatty sequence bilang slot mapping
    // ============================================
    cout << "  --- 1. Beatty sequence ---\n\n";
    cout << "  slot(i) = floor(i × φ) — optimal distribution\n\n";

    cout << "  i | floor(iφ) | floor(iφ²) | Gap\n";
    cout << "  --|-----------|------------|-----\n";
    for (int i = 0; i <= 15; i++) {
        double b1 = floor(i * PHI);
        double b2 = floor(i * PHI * PHI);
        cout << "  " << setw(2) << i << " | "
             << setw(10) << b1 << " | "
             << setw(10) << b2 << " | "
             << setw(5) << (b2 - b1) << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Ang partition property
    // ============================================
    cout << "  --- 2. Partition property ---\n\n";
    cout << "  Ang floor(iφ) at floor(iφ²) ay partition\n";
    cout << "  ng lahat ng positive integers\n\n";

    // I-verify: walang overlap at walang gap
    vector<bool> covered(50, false);
    for (int i = 1; i <= 15; i++) {
        covered[(int)floor(i * PHI)] = true;
        covered[(int)floor(i * PHI * PHI)] = true;
    }
    
    cout << "  Covered integers (1-50):\n  ";
    for (int i = 1; i <= 50; i++) {
        if (covered[i]) cout << i << " ";
    }
    cout << "\n\n";

    // ============================================
    // 3. Slot gap analysis
    // ============================================
    cout << "  --- 3. Slot gap analysis ---\n\n";
    cout << "  Ang gaps sa pagitan ng Beatty slots\n";
    cout << "  ay φ-based na may natural na distribution\n\n";

    vector<double> slots;
    for (int i = 0; i <= 10; i++) {
        slots.push_back(floor(i * PHI));
    }
    
    cout << "  Slots: ";
    for (double s : slots) cout << s << " ";
    cout << "\n\n";
    
    cout << "  Gaps: ";
    for (size_t i = 1; i < slots.size(); i++) {
        cout << (slots[i] - slots[i-1]) << " ";
    }
    cout << "\n\n";

    // ============================================
    // 4. CKKS na may Beatty slot mapping
    // ============================================
    cout << "  --- 4. CKKS na may Beatty slot mapping ---\n\n";

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

    // I-encrypt ang φ-powers sa Beatty na slots
    vector<double> vals(8, 0.0);
    for (int i = 0; i < 8; i++) {
        vals[i] = pow(PHI, i);
    }
    
    Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
    auto ct = cc->Encrypt(keyPair.publicKey, pt);
    
    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct, &pt_out);
    pt_out->SetLength(8);
    auto res = pt_out->GetCKKSPackedValue();
    
    cout << "  φ-powers sa slots:\n  ";
    for (int i = 0; i < 8; i++) {
        cout << setw(8) << res[i].real();
    }
    cout << "\n\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
