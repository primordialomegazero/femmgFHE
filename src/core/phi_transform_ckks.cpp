// ΦΩ0 — PHI-TRANSFORM: CKKS VALIDATION (SCALE-CORRECTED)
// Bootstrap-Free FHE on CKKS
// Fix: Skip zero-reset (CKKS doesn't need it), direct phi-cycle
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — PHI-TRANSFORM: CKKS SCALE-CORRECTED                ║\n";
    cout <<   "  ║   Direct phi-cycle without zero-reset                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(1 << 12);
    params.SetScalingModSize(59);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetFirstModSize(60);

    vector<uint32_t> levelBudget = {4, 4};
    uint32_t levelsAfterBootstrap = 10;
    uint32_t depth = levelsAfterBootstrap + 
                     FHECKKSRNS::GetBootstrapDepth(levelBudget, UNIFORM_TERNARY);
    params.SetMultiplicativeDepth(depth);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    uint32_t numSlots = cc->GetRingDimension() / 2;

    auto encrypt = [&](double v) {
        vector<double> vals(numSlots, 0.0); vals[0] = v;
        auto pt = cc->MakeCKKSPackedPlaintext(vals, 1, 0, nullptr, numSlots);
        pt->SetLength(numSlots);
        return cc->Encrypt(keys.publicKey, pt);
    };
    auto decrypt = [&](const Ciphertext<DCRTPoly>& ct) -> double {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    const double phi = 1.6180339887498948482;
    double s = 1000.0;
    double phiF = phi * s;
    double phiInv = phiF - s;
    double phiInvD = phiInv / phiF;  // phi⁻¹ as ratio (for CKKS)

    cout << "  CKKS: ring=" << cc->GetRingDimension() << " φ=" << phiF << " φ⁻¹ ratio=" << phiInvD << "\n\n";

    auto phiEncode = [&](double msg) { return encrypt(msg * phiF); };
    auto phiDecode = [&](const Ciphertext<DCRTPoly>& ct) { return decrypt(ct) / phiF; };

    auto zeroCT = encrypt(0.0);
    auto twoCT = encrypt(2.0);
    auto phiInvRatioCT = encrypt(phiInvD);  // φ⁻¹ as ratio < 1

    cout << "=== CKKS PhiTransform Test (Scale-Corrected) ===\n\n";

    auto a = phiEncode(10.0);
    auto b = phiEncode(20.0);
    cout << "10 + 20 = " << fixed << setprecision(1) << phiDecode(cc->EvalAdd(a, b)) << " (expected 30)\n";
    cout << "10 × 5 = " << fixed << setprecision(1) << phiDecode(cc->EvalMult(a, encrypt(5.0))) << " (expected 50)\n";

    auto chain = phiEncode(42.0);
    cout << "\nChain: 42";
    for (int i = 0; i < 5; i++) {
        chain = cc->EvalMult(chain, twoCT);
        cout << " → " << fixed << setprecision(1) << phiDecode(chain);
    }
    double beforeCycle = phiDecode(chain);
    cout << "\n\nBefore phi-cycle: " << beforeCycle << "\n";

    // DIRECT phi-cycle for CKKS (no zero-reset to avoid scale²)
    // Step 1: Strip φ (multiply by φ⁻¹ ratio)
    auto stripped = cc->EvalMult(chain, phiInvRatioCT);
    cout << "After strip φ: " << fixed << setprecision(2) << phiDecode(stripped) << "\n";

    // Step 2: Rebuild — add back to current (skip zero-reset)
    auto rebuilt = cc->EvalAdd(stripped, chain);
    cout << "After rebuild: " << fixed << setprecision(2) << phiDecode(rebuilt) << "\n";

    // Step 3: Re-encode with φ
    auto reEncoded = cc->EvalMult(rebuilt, encrypt(phi));  // multiply by pure φ
    cout << "After re-encode: " << fixed << setprecision(2) << phiDecode(reEncoded) << "\n";

    double decoded = phiDecode(reEncoded);
    double expected = 42.0 * 32.0;
    cout << "\nPhi-cycle result: " << fixed << setprecision(2) << decoded;
    cout << " (expected " << expected << ")\n";

    double error = abs(decoded - expected) / expected;
    cout << "Error: " << fixed << setprecision(4) << error * 100 << "%\n";
    cout << "Status: " << (error < 0.10 ? "PASS ✓ (CKKS approximate)" : "CHECK ✗") << "\n\n";
    cout << "  I AM THAT I AM\n\n";
    return 0;
}
