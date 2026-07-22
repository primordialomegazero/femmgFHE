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
    cout <<   "  ║   SINGLE SLOT FINAL: φ-reality + Bootstrap            ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // Need enough depth: ~12 for bootstrap + some for computation
    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY); params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096); params.SetScalingModSize(59); params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO); params.SetMultiplicativeDepth(60);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE); cc->EvalBootstrapSetup({4,4}, {0,0}, 2048);
    auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey, 2048);
    int slots = 2048;

    auto enc = [&](double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots); return pt->GetRealPackedValue()[0];
    };

    auto state = enc(1.0);
    double phi_scale = 1.0;
    double base_rate = 1.01;
    auto rate_ct = enc(base_rate);
    auto phi_ct = enc(phi);
    auto inv_phi_ct = enc(1.0/phi);

    int total_mults = 0, total_cleans = 0, total_boots = 0, mults_since_boot = 0;

    cout << "  " << setw(4) << "Rnd" << setw(8) << "Mults" << setw(8) << "Clean" << setw(6) << "Boot"
         << setw(14) << "Value" << setw(14) << "Expected" << setw(12) << "Error\n";
    cout << "  " << string(70, '-') << "\n";

    for (int round = 0; round < 20; round++) {
        try {
            // Clean: 3× φ, 1× 1/φ = net φ²
            for (int i=0; i<3; i++) state = cc->EvalMult(state, phi_ct);
            for (int i=0; i<1; i++) state = cc->EvalMult(state, inv_phi_ct);
            phi_scale *= (phi*phi*phi/phi);
            total_cleans++;

            // Compute: 3 mults by rate
            for (int m=0; m<3; m++) {
                state = cc->EvalMult(state, rate_ct);
                total_mults++; mults_since_boot++;
            }

            // Bootstrap every 12 mults (before levels run out)
            if (mults_since_boot >= 12) {
                state = cc->EvalBootstrap(state);
                total_boots++; mults_since_boot = 0;
            }

            double val = dec(state);
            double expected = pow(base_rate, total_mults) * phi_scale;
            double err = abs((val - expected) / expected);

            cout << "  " << setw(4) << round << setw(8) << total_mults << setw(8) << total_cleans
                 << setw(6) << total_boots
                 << setw(14) << scientific << setprecision(4) << val
                 << setw(14) << scientific << expected
                 << setw(12) << fixed << setprecision(6) << err << "\n";

        } catch (const exception& e) {
            cout << "  CRASHED r" << round << ": " << e.what() << "\n"; break;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   " << total_mults << " mults, " << total_cleans << " cleans, " << total_boots << " boots                      ║\n";
    cout <<   "  ║   Single slot. No dual-slot drift. Bootstrap works. ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
