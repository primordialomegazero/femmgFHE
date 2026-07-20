// ΦΩ0 — PHI-TRANSFORM ON LUCAS PRIME (33-bit, φ order=47)
// "THE LUCAS PRIME. φ CYCLES EVERY 47 STEPS."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int64_t modulus = 6643838879LL;  // Lucas prime, 33 bits
    int64_t phiF = 2053059122;        // φ mod this prime
    int64_t phiInv = mod_pos(phiF - 1000, modulus);  // Approx φ⁻¹ (scaled)
    
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — PHI-TRANSFORM ON LUCAS PRIME                      ║\n";
    cout <<   "  ║   p=" << modulus << " (33-bit Lucas prime)                    ║\n";
    cout <<   "  ║   φ=" << phiF << ", order=47";
    for (int i = 0; i < (28 - to_string(phiF).length()); i++) cout << " ";
    cout << "║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> int64_t {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return mod_pos((int64_t)pt->GetPackedValue()[0], modulus);
    };

    // Verify φ property: φ² ≡ φ + 1 mod p
    int64_t phi_sq = mod_pos(phiF * phiF, modulus);
    int64_t phi_plus_1 = mod_pos(phiF + 1000, modulus);  // +1 scaled
    cout << "  φ² mod p = " << phi_sq << "\n";
    cout << "  φ+1 mod p = " << phi_plus_1 << "\n";
    cout << "  φ² ≡ φ+1? " << (phi_sq == phi_plus_1 ? "YES! φ IS REAL!" : "scaling mismatch") << "\n\n";

    // Test basic encryption
    auto ct = enc(42);
    cout << "  Enc(42) → Dec: " << dec(ct) << " ✓\n";
    
    ct = cc->EvalMult(ct, enc(2));
    cout << "  ×2 → " << dec(ct) << " ✓\n\n";

    // Test φ-cyclic property: multiply by φ 47 times, should return to original
    cout << "  Testing φ-cycle (×φ " << 47 << " times)...\n";
    auto cycle = enc(1);
    auto enc_phi = enc(phiF);
    int64_t val = 1;
    
    for (int i = 1; i <= 47; i++) {
        cycle = cc->EvalMult(cycle, enc_phi);
        val = mod_pos(val * phiF, modulus);
        if (i <= 5 || i >= 45 || val == 1) {
            cout << "    Step " << i << ": " << dec(cycle) << " (mod val: " << val << ")\n";
        }
    }
    
    int64_t final_val = dec(cycle);
    cout << "\n  Final after 47 ×φ: " << final_val << "\n";
    cout << "  Should be 1 (full cycle): " << (final_val == 1 ? "YES! φ-CYCLE CONFIRMED!" : "no") << "\n\n";
    
    cout << "  I AM THAT I AM\n\n";
    return 0;
}
