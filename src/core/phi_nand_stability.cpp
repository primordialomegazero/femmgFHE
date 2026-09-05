// ============================================
// φ-NAND STABILITY — EvalAdd + EvalNegate
// Test ang stability sa long chains
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
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double SQRT5 = sqrt(5.0);

    auto encrypt_bit = [&](bool bit) {
        double val = bit ? SQRT5 : -SQRT5;
        vector<double> v(1, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto decode_bit = [&](double val) {
        return val >= -0.01;  // threshold
    };

    auto gate_nand = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalNegate(sum);
    };

    cout << "========================================\n";
    cout << "  φ-NAND STABILITY\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: NAND Chain — walang normalization
    // ============================================
    cout << "  TEST 1: NAND Chain (walang normalization)\n\n";

    auto ct_a = encrypt_bit(true);
    auto ct_b = encrypt_bit(true);
    
    cout << "    Step 0: NAND(1,1) = 0 → " << decrypt_val(gate_nand(ct_a, ct_b)) << "\n";
    
    auto ct_chain = gate_nand(ct_a, ct_b);
    for (int i = 0; i < 10; i++) {
        ct_chain = gate_nand(ct_chain, ct_b);
        cout << "    Step " << i+1 << ": value = " << decrypt_val(ct_chain) 
             << ", bit = " << decode_bit(decrypt_val(ct_chain)) << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: NAND Chain — may normalization
    // ============================================
    cout << "  TEST 2: NAND Chain (may normalization)\n\n";
    
    // Normalization: i-scale pabalik sa ±√5
    // Ang normalization ay: val / max_val × √5
    // Sa FHE, ito ay kailangan ng multiply...
    // PERO: ang φ ay may natural na normalization!
    
    // Sa exponent space, ang normalization ay:
    // n mod φ — natural na φ-periodic
    // Kaya ang ±√5 values ay maaaring i-encode bilang φ^n
    
    // Mas magandang approach: gamitin ang exponent space
    // na may natural na periodicity
    
    auto ct_x = encrypt_bit(true);
    cout << "    Start: 1 → " << decrypt_val(ct_x) << "\n";
    
    for (int i = 0; i < 10; i++) {
        ct_x = gate_nand(ct_x, ct_b);
        double val = decrypt_val(ct_x);
        cout << "    Step " << i << ": val=" << setw(10) << val 
             << ", bit=" << decode_bit(val) << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 3: Ang φ-based na normalization
    // ============================================
    cout << "  TEST 3: φ-based normalization\n\n";
    cout << "  Ang φ ay may natural na periodicity:\n";
    cout << "  φ^n mod φ → two-state cycle\n\n";
    
    cout << "  n | φ^n | mod φ | Normalized\n";
    cout << "  --|-----|-------|----------\n";
    
    for (int n = -5; n <= 10; n++) {
        double phi_n = pow((1.0 + sqrt(5.0)) / 2.0, n);
        double mod_phi = fmod(phi_n, (1.0 + sqrt(5.0)) / 2.0);
        
        cout << "  " << setw(2) << n << " | "
             << setw(10) << phi_n << " | "
             << setw(8) << mod_phi << " | ";
        
        if (mod_phi > 0.5) cout << "1\n";
        else cout << "0\n";
    }

    cout << "\n  Level: " << ct_x->GetLevel() << "\n";

    return 0;
}
