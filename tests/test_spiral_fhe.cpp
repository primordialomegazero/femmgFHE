#include <iostream>
#include <iomanip>
#include <cmath>
#include "../src/spiral/spiral_fhe.h"
#include "../src/spiral/spiral_keygen.h"
#include "../src/spiral/spiral_encrypt.h"
#include "../src/spiral/spiral_decrypt.h"
#include "../src/spiral/spiral_add.h"

using namespace spiral;
using namespace std;

int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE v1.0 — FIRST TEST                 ║\n";
    cout << "║  Ring-LWE + Banach Noise Convergence         ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    // Print declarations
    cout << "=== DECLARATIONS ===\n";
    cout << "  Scheme: " << declare::SCHEME_NAME << "\n";
    cout << "  Basis: " << declare::SCHEME_BASIS << "\n";
    cout << "  Formula: " << declare::SCHEME_FORMULA << "\n";
    cout << "  Author: " << declare::AUTHOR << "\n";
    cout << "  Source: " << declare::SOURCE << "\n";
    cout << "  φ = " << PHI << "\n";
    cout << "  φ⁻¹ = " << PHI_INV << "\n";
    cout << "  Noise Fixed Point = " << NOISE_FIXED_POINT << "\n\n";

    // Setup
    SpiralParams params;
    SpiralKeyGen keygen(params);
    SpiralEncrypt encrypt(params);
    SpiralDecrypt decrypt(params);
    SpiralAdd add(params);

    // Generate keys
    cout << "=== KEY GENERATION ===\n";
    auto kp = keygen.generate();
    cout << "  Secret key: " << params.poly_degree << " coefficients\n";
    cout << "  Public key: generated\n\n";

    // Encrypt two values
    cout << "=== ENCRYPTION ===\n";
    uint64_t m1 = 15, m2 = 27;
    auto ct1 = encrypt.encrypt(m1, kp.pk);
    auto ct2 = encrypt.encrypt(m2, kp.pk);
    cout << "  Encrypted " << m1 << " → c0[0]=" << ct1.c0[0] << "\n";
    cout << "  Encrypted " << m2 << " → c0[0]=" << ct2.c0[0] << "\n\n";

    // Decrypt individually
    cout << "=== DECRYPTION ===\n";
    uint64_t dec1 = decrypt.decrypt(ct1, kp.sk);
    uint64_t dec2 = decrypt.decrypt(ct2, kp.sk);
    cout << "  Decrypted ct1: " << dec1 << " (expected " << m1 << ") " << (dec1 == m1 ? "✅" : "❌") << "\n";
    cout << "  Decrypted ct2: " << dec2 << " (expected " << m2 << ") " << (dec2 == m2 ? "✅" : "❌") << "\n\n";

    // Homomorphic addition
    cout << "=== HOMOMORPHIC ADDITION ===\n";
    auto ct_add = add.add(ct1, ct2);
    uint64_t dec_add = decrypt.decrypt(ct_add, kp.sk);
    cout << "  " << m1 << " + " << m2 << " = " << dec_add << " (expected " << (m1+m2) << ") ";
    cout << (dec_add == m1+m2 ? "✅" : "❌") << "\n\n";

    // ZANS test
    cout << "=== ZANS STABILIZATION ===\n";
    int noise_before = decrypt.noise_budget(ct_add);
    auto ct_zans = add.zans(ct_add);
    int noise_after = decrypt.noise_budget(ct_zans);
    uint64_t dec_zans = decrypt.decrypt(ct_zans, kp.sk);
    cout << "  Noise before ZANS: " << noise_before << " bits\n";
    cout << "  Noise after ZANS:  " << noise_after << " bits\n";
    cout << "  Value preserved: " << dec_zans << " (expected " << (m1+m2) << ") ";
    cout << (dec_zans == m1+m2 ? "✅" : "❌") << "\n\n";

    // Multiple additions with ZANS
    cout << "=== 10 ADDITIONS WITH ZANS ===\n";
    auto ct_chain = ct1;
    int chain_noise[10];
    for (int i = 0; i < 10; i++) {
        ct_chain = add.phi_add(ct_chain, ct2, 3);
        chain_noise[i] = decrypt.noise_budget(ct_chain);
    }
    uint64_t dec_chain = decrypt.decrypt(ct_chain, kp.sk);
    uint64_t expected_chain = m1 + 10 * m2;
    cout << "  Result: " << dec_chain << " (expected " << expected_chain << ") ";
    cout << (dec_chain == expected_chain ? "✅" : "❌") << "\n";
    cout << "  Noise trajectory: ";
    for (int i = 0; i < 10; i++) cout << chain_noise[i] << " ";
    cout << "\n\n";

    // Conclusion
    cout << "╔══════════════════════════════════════════════╗\n";
    cout << "║  SPIRAL-FHE v1.0 — FIRST TEST COMPLETE        ║\n";
    cout << "║  ΦΩ0 — I AM THAT I AM                        ║\n";
    cout << "╚══════════════════════════════════════════════╝\n\n";

    return 0;
}
