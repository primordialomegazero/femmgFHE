// ΦΩ0 — GOAL 2: SNC VERIFICATION v3
// BARE vs SNC — with try-catch survival counting
// "SAFE DECRYPTION. COUNT SURVIVAL."

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

double safe_decrypt(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keys, 
                    const Ciphertext<DCRTPoly>& ct, int numSlots, bool& ok) {
    try {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(numSlots);
        ok = true;
        return pt->GetRealPackedValue()[0];
    } catch (...) {
        ok = false;
        return -1;
    }
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — GOAL 2: SNC VERIFICATION v3                   ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096);
    params.SetScalingModSize(59);
    params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetMultiplicativeDepth(100);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    int numSlots = 2048, maxSteps = 100;

    auto enc = [&](double v) {
        vector<double> vec(numSlots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto enc_zero = enc(0.0);
    auto enc_mul = enc(2.0);

    // BARE
    cout << "  ── BARE ──\n";
    auto ct = enc(1.0);
    int bare_steps = 0;
    double expected = 1.0;
    for (int i = 0; i < maxSteps; i++) {
        ct = cc->EvalMult(ct, enc_mul);
        expected *= 2.0;
        bool ok; double dec = safe_decrypt(cc, keys, ct, numSlots, ok);
        if (!ok) break;
        if (abs((dec - expected) / expected) < 0.01) bare_steps = i + 1;
        else break;
    }
    cout << "    Survived: " << bare_steps << " mults\n";

    // SNC
    cout << "  ── SNC ──\n";
    ct = enc(1.0);
    int snc_steps = 0;
    expected = 1.0;
    for (int i = 0; i < maxSteps; i++) {
        ct = cc->EvalMult(ct, enc_mul);
        ct = cc->EvalAdd(ct, enc_zero);
        expected *= 2.0;
        bool ok; double dec = safe_decrypt(cc, keys, ct, numSlots, ok);
        if (!ok) break;
        if (abs((dec - expected) / expected) < 0.01) snc_steps = i + 1;
        else break;
    }
    cout << "    Survived: " << snc_steps << " mults\n";

    // SNCx5
    cout << "  ── SNCx5 ──\n";
    ct = enc(1.0);
    int snc5_steps = 0;
    expected = 1.0;
    for (int i = 0; i < maxSteps; i++) {
        ct = cc->EvalMult(ct, enc_mul);
        for (int j = 0; j < 5; j++) ct = cc->EvalAdd(ct, enc_zero);
        expected *= 2.0;
        bool ok; double dec = safe_decrypt(cc, keys, ct, numSlots, ok);
        if (!ok) break;
        if (abs((dec - expected) / expected) < 0.01) snc5_steps = i + 1;
        else break;
    }
    cout << "    Survived: " << snc5_steps << " mults\n";

    // VERDICT
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   SURVIVAL: BARE=" << bare_steps << " | SNC=" << snc_steps << " | SNCx5=" << snc5_steps;
    for (int i = 0; i < (27 - to_string(bare_steps).length() - to_string(snc_steps).length() - to_string(snc5_steps).length()); i++) cout << " ";
    cout << "║\n";
    if (snc_steps > bare_steps + 1)
        cout << "  ║   *** SNC WORKS: +" << (snc_steps - bare_steps) << " steps ***                           ║\n";
    else if (snc_steps < bare_steps - 1)
        cout << "  ║   *** SNC MAKES IT WORSE ***                            ║\n";
    else
        cout << "  ║   *** SNC HAS NO EFFECT ***                             ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
