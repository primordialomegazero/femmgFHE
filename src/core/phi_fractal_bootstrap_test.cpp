// ΦΩ0 — FRACTAL BOOTSTRAP v2.0 — MULTI-PARTY TEST
// Pure Enc(0) cascading for noise reset without decryption
// "THE FRACTAL HEALS. NO KEY REQUIRED."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include "zans_production_lib.h"
#include "phi_fractal_bootstrap_v2.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — FRACTAL BOOTSTRAP v2.0 — MULTI-PARTY              ║\n";
    cout <<   "  ║   Pure Enc(0) cascading — no decryption, no key           ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    FractalBootstrapEngine fbe(cc, keys, modulus, 5, 100);

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> int64_t {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return mod_pos((int64_t)pt->GetPackedValue()[0], modulus);
    };

    // ============================================
    // TEST: Corrupt ciphertext, then fractal bootstrap
    // ============================================
    cout << "  === FRACTAL BOOTSTRAP TEST ===\n\n";

    auto ct = enc(42);
    auto two = enc(2);
    auto M = enc(modulus / 2);
    auto anchor0 = enc(0);

    cout << "  Phase 1: Corrupt ciphertext with multiplications\n";
    double noise_before = fbe.measure_noise(ct);
    cout << "  Initial noise: " << noise_before << "\n";

    // SNC-stabilized multiplications to add noise
    for (int i = 0; i < 20; i++) {
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        ct = cc->EvalMult(ct, two);
        auto correction = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, correction);
        ct = cc->EvalAdd(ct, anchor0);
    }

    double noise_after_mults = fbe.measure_noise(ct);
    int64_t val_before = dec(ct);
    cout << "  Noise after 20 mults: " << noise_after_mults << "\n";
    cout << "  Value: " << val_before << " (expected " << mod_pos(42LL * (1LL << 20), modulus) << ")\n\n";

    // Phase 2: Fractal Bootstrap (no decryption)
    cout << "  Phase 2: Fractal Bootstrap (pure Enc(0) cascading)\n";
    auto ct_healed = fbe.fractal_bootstrap(ct, 3);

    double noise_after = fbe.measure_noise(ct_healed);
    int64_t val_after = dec(ct_healed);

    cout << "\n  === RESULTS ===\n\n";
    cout << "  Noise before:  " << fixed << setprecision(2) << noise_after_mults << "\n";
    cout << "  Noise after:   " << fixed << setprecision(2) << noise_after << "\n";
    cout << "  Noise change:  " << fixed << setprecision(1) << (noise_after_mults - noise_after) 
         << " (" << ((noise_after_mults - noise_after) / noise_after_mults * 100) << "%)\n";
    cout << "  Value before:  " << val_before << "\n";
    cout << "  Value after:   " << val_after << "\n";
    cout << "  Value preserved: " << (val_before == val_after ? "YES ✓" : "NO ✗") << "\n";
    cout << "  Total Enc(0) additions: " << fbe.get_total_enc0_additions() << "\n\n";

    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FRACTAL BOOTSTRAP — ";
    if (val_before == val_after && noise_after < noise_after_mults) {
        cout << "NOISE REDUCED, VALUE PRESERVED ✓   ║\n";
    } else {
        cout << "CHECK RESULTS                        ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return 0;
}
