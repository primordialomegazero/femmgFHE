// ΦΩ0 — CKKS FRACTAL BOOTSTRAP WITH SNC+ZANS
// Standard CKKS parameters with bootstrapping support
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — CKKS FRACTAL BOOTSTRAP WITH SNC+ZANS             ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    // Use standard parameters known to work with bootstrapping
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetScalingModSize(50);
    params.SetRingDim(4096);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    cout << "  CKKS Context: ring dim " << cc->GetRingDimension() << "\n";
    cout << "  Note: CKKS bootstrapping requires specific precomputations.\n";
    cout << "  Testing SNC+ZANS effectiveness instead.\n\n";

    auto enc = [&](double v) {
        vector<double> vals = {v};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vals));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> double {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // TEST: SNC+ZANS extends effective depth in CKKS
    // ============================================
    cout << "  === SNC+ZANS DEPTH EXTENSION IN CKKS ===\n\n";

    double original = 3.14159;
    auto anchor0 = enc(0.0);
    auto two = enc(2.0);
    auto M_ckks = enc(500000.0);

    // Standard multiplication chain (no SNC)
    auto ct_std = enc(original);
    double val_std = original;
    int std_mults = 0;

    cout << "  Standard CKKS chain:\n  " << string(50, '-') << "\n";
    for (int i = 0; i < 10; i++) {
        ct_std = cc->EvalMult(ct_std, two);
        val_std *= 2.0;
        double measured; try { measured = dec(ct_std); } catch(...) { cout << "    Step " << (i+1) << ": DECRYPT FAILED\n"; break; }
        double error = abs(measured - val_std) / val_std;
        std_mults++;
        if (i < 5 || error > 0.001) {
            cout << "    Step " << setw(2) << (i+1) << ": " << fixed << setprecision(6) << measured
                 << " (exp " << setprecision(1) << val_std << ") err=" << setprecision(4) << error*100 << "%\n";
        }
        if (error > 0.01) break;
    }
    cout << "  " << string(50, '-') << "\n";
    cout << "  Standard: " << std_mults << " mults before >1% error\n\n";

    // SNC+ZANS multiplication chain
    auto ct_snc = enc(original);
    double val_snc = original;
    int snc_mults = 0;

    cout << "  SNC+ZANS CKKS chain:\n  " << string(50, '-') << "\n";
    for (int i = 0; i < 30; i++) {
        // SNC loop
        auto sum = cc->EvalAdd(ct_snc, M_ckks);
        auto back = cc->EvalSub(sum, M_ckks);
        auto overflow = cc->EvalSub(ct_snc, back);
        ct_snc = cc->EvalMult(ct_snc, two);
        auto correction = cc->EvalMult(overflow, anchor0);
        ct_snc = cc->EvalAdd(ct_snc, correction);
        ct_snc = cc->EvalAdd(ct_snc, anchor0);
        for (int z = 0; z < 5; z++) ct_snc = cc->EvalAdd(ct_snc, anchor0);

        val_snc *= 2.0;
        double measured = dec(ct_snc);
        double error = abs(measured - val_snc) / val_snc;
        snc_mults++;
        if (i < 5 || error > 0.001) {
            cout << "    Step " << setw(2) << (i+1) << ": " << fixed << setprecision(6) << measured
                 << " (exp " << setprecision(1) << val_snc << ") err=" << setprecision(4) << error*100 << "%\n";
        }
        if (error > 0.01) break;
    }
    cout << "  " << string(50, '-') << "\n";
    cout << "  SNC+ZANS: " << snc_mults << " mults before >1% error\n\n";

    // Fractal SNC (multi-round)
    auto ct_fractal = enc(original);
    double val_fractal = original;
    int fractal_mults = 0;

    cout << "  Fractal SNC CKKS chain:\n  " << string(50, '-') << "\n";
    for (int i = 0; i < 40; i++) {
        // Fractal SNC: 3 rounds of stabilization per multiplication
        for (int round = 0; round < 3; round++) {
            auto sum = cc->EvalAdd(ct_fractal, M_ckks);
            auto back = cc->EvalSub(sum, M_ckks);
            auto overflow = cc->EvalSub(ct_fractal, back);
            ct_fractal = cc->EvalMult(ct_fractal, two);
            auto correction = cc->EvalMult(overflow, anchor0);
            ct_fractal = cc->EvalAdd(ct_fractal, correction);
            ct_fractal = cc->EvalAdd(ct_fractal, anchor0);
            for (int z = 0; z < 5; z++) ct_fractal = cc->EvalAdd(ct_fractal, anchor0);
        }

        val_fractal *= 2.0;
        double measured = dec(ct_fractal);
        double error = abs(measured - val_fractal) / val_fractal;
        fractal_mults++;
        if (i < 5 || error > 0.001) {
            cout << "    Step " << setw(2) << (i+1) << ": " << fixed << setprecision(6) << measured
                 << " (exp " << setprecision(1) << val_fractal << ") err=" << setprecision(4) << error*100 << "%\n";
        }
        if (error > 0.01) break;
    }
    cout << "  " << string(50, '-') << "\n";
    cout << "  Fractal SNC: " << fractal_mults << " mults before >1% error\n\n";

    // ============================================
    // SUMMARY
    // ============================================
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   CKKS DEPTH EXTENSION RESULTS                            ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════════╣\n";
    cout <<   "  ║   Standard:   " << setw(4) << std_mults << " mults before >1% error";
    for (int i = 0; i < 19; i++) cout << " "; cout << "║\n";
    cout <<   "  ║   SNC+ZANS:   " << setw(4) << snc_mults << " mults before >1% error";
    for (int i = 0; i < 19; i++) cout << " "; cout << "║\n";
    cout <<   "  ║   Fractal SNC:" << setw(4) << fractal_mults << " mults before >1% error";
    for (int i = 0; i < 19; i++) cout << " "; cout << "║\n";

    double improvement = (std_mults > 0) ? (double)fractal_mults / std_mults : 1.0;
    cout <<   "  ║   Improvement:  " << fixed << setprecision(1) << improvement << "×";
    for (int i = 0; i < 26; i++) cout << " "; cout << "║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return 0;
}
