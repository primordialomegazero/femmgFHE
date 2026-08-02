// FEmmg-FHE — LEVEL TRACE: Where do levels go?
// Step by step: encrypt, clean, jump, check level after each

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   LEVEL TRACE: Where do CKKS levels go?               ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY); params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096); params.SetScalingModSize(59); params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO); params.SetMultiplicativeDepth(30);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    int slots = 2048;

    auto enc = [&](double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    auto level = [&](const Ciphertext<DCRTPoly>& ct) {
        return ct->GetLevel();
    };

    cout << "  Initial depth budget: 30\n\n";

    // STEP 1: Encrypt
    auto ct = enc(1.0);
    cout << "  [1] Encrypt:          level=" << level(ct) << " (consumed 0)\n";

    // STEP 2: One EvalMult (multiply by φ)
    auto phi_ct = enc(phi);
    ct = cc->EvalMult(ct, phi_ct);
    cout << "  [2] EvalMult(×φ):     level=" << level(ct) << " (consumed 1) ← EACH MULT COSTS 1\n";

    // STEP 3: Another EvalMult
    ct = cc->EvalMult(ct, phi_ct);
    cout << "  [3] EvalMult(×φ):     level=" << level(ct) << " (consumed 1)\n";

    // STEP 4: EvalAdd (should be FREE)
    auto ct2 = enc(2.0);
    ct = cc->EvalAdd(ct, ct2);
    cout << "  [4] EvalAdd:          level=" << level(ct) << " (consumed 0) ← ADD IS FREE ✓\n";

    // STEP 5: Try mul_X as EvalAdd implementation
    // mul_X(x) = (x.b, x.a + x.b) — this uses EvalAdd, NOT EvalMult
    auto state_a = enc(3.0);
    auto state_b = enc(4.0);
    cout << "\n  ── mul_X implementation test ──\n";
    cout << "  state_a level=" << level(state_a) << " state_b level=" << level(state_b) << "\n";
    
    // mul_X: new_a = state_b (copy), new_b = state_a + state_b (add)
    auto new_a = state_b;  // Just reference copy — zero cost
    auto new_b = cc->EvalAdd(state_a, state_b);  // Addition — zero levels
    cout << "  mul_X: new_a level=" << level(new_a) << " (copy) new_b level=" << level(new_b) << " (add)\n";
    cout << "  ✓ mul_X costs ZERO levels when done as copy+add\n\n";

    // STEP 6: Full clean cycle (3× mul_X + 1× div_X) using EvalMult for φ
    cout << "  ── Clean via EvalMult (×φ, ×φ, ×φ, ×1/φ) ──\n";
    ct = enc(1.0);
    cout << "  Start: level=" << level(ct) << "\n";
    ct = cc->EvalMult(ct, phi_ct);  // ×φ
    cout << "  ×φ [1]: level=" << level(ct) << "\n";
    ct = cc->EvalMult(ct, phi_ct);  // ×φ
    cout << "  ×φ [2]: level=" << level(ct) << "\n";
    ct = cc->EvalMult(ct, phi_ct);  // ×φ
    cout << "  ×φ [3]: level=" << level(ct) << "\n";
    auto inv_phi_ct = enc(1.0/phi);
    ct = cc->EvalMult(ct, inv_phi_ct);  // ×1/φ
    cout << "  ×1/φ [1]: level=" << level(ct) << "\n";
    cout << "  ❌ Clean via EvalMult costs 4 LEVELS per cycle!\n\n";

    // STEP 7: The real question — can we do mul_X WITHOUT EvalMult?
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   CONCLUSION:                                          ║\n";
    cout <<   "  ║   mul_X as copy+add = ZERO levels (theory)            ║\n";
    cout <<   "  ║   mul_X as EvalMult(×φ) = 1 level per call (practice) ║\n";
    cout <<   "  ║   Current clean uses EvalMult → 4 levels per cycle   ║\n";
    cout <<   "  ║   For true zero-depth: need φ-element representation  ║\n";
    cout <<   "  ║   with a,b as separate CKKS ciphertexts               ║\n";
    cout <<   "  ║   Then mul_X = copy+add, not EvalMult                ║\n";
    cout <<   "  ║   This is why Simple Fib (dual-ciphertext) survived   ║\n";
    cout <<   "  ║   228 mults — mul_X there was TRUE copy+add          ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
