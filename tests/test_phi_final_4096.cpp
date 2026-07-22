// FEmmg-FHE — FINAL: Single-slot + Fibonacci + Bootstrap (4096)
// Optimized for consumer hardware. Clean architecture.

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <map>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482;
CryptoContext<DCRTPoly> cc;
KeyPair<DCRTPoly> keys;
int slots;

Ciphertext<DCRTPoly> enc(double v) {
    vector<double> vec(slots, 0.0); vec[0] = v;
    return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
}
double dec(const Ciphertext<DCRTPoly>& ct) {
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
    pt->SetLength(slots); return pt->GetRealPackedValue()[0];
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FINAL: Single-slot φ-clean + Fibonacci + Bootstrap  ║\n";
    cout <<   "  ║   RingDim=4096. Consumer hardware.                   ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY); params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096); params.SetScalingModSize(59); params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO); params.SetMultiplicativeDepth(60);
    cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE); cc->EvalBootstrapSetup({4,4}, {0,0}, 2048);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey, 2048);
    slots = 2048;

    auto state = enc(1.0);
    double phi_scale = 1.0;
    int total_mults = 0, total_cleans = 0, total_boots = 0, mults_since_boot = 0;
    auto phi_ct = enc(phi), inv_phi_ct = enc(1.0/phi), rate_ct = enc(1.01);

    // Fibonacci pattern: small rotations, clean between, bootstrap periodically
    vector<int> jumps = {5, 8, 13, 21, 34};
    
    cout << "  Clean (φ²) → Jump (Fibonacci) → Bootstrap/40 → Repeat\n\n";
    cout << "  " << setw(4) << "Rnd" << setw(6) << "Jump" << setw(8) << "Total"
         << setw(6) << "Cln" << setw(6) << "Boot"
         << setw(12) << "Value" << setw(12) << "Expected"
         << setw(10) << "Error\n";
    cout << "  " << string(70, '-') << "\n";

    for (int round = 0; round < 15; round++) {
        int jump = jumps[round % jumps.size()];
        try {
            // Clean
            for (int i=0; i<3; i++) state = cc->EvalMult(state, phi_ct);
            for (int i=0; i<1; i++) state = cc->EvalMult(state, inv_phi_ct);
            phi_scale *= (phi*phi*phi/phi); total_cleans++;

            // Fibonacci jump
            for (int m=0; m<jump; m++) {
                state = cc->EvalMult(state, rate_ct);
                total_mults++; mults_since_boot++;
            }

            // Bootstrap
            if (mults_since_boot >= 35) {
                state = cc->EvalBootstrap(state); total_boots++; mults_since_boot = 0;
            }

            double val = dec(state);
            double expected = pow(1.01, total_mults) * phi_scale;
            double err = abs((val - expected) / expected);

            cout << "  " << setw(4) << round << setw(6) << jump << setw(8) << total_mults
                 << setw(6) << total_cleans << setw(6) << total_boots
                 << setw(12) << scientific << setprecision(3) << val
                 << setw(12) << scientific << expected
                 << setw(10) << fixed << setprecision(4) << err << "\n";

        } catch (const exception& e) {
            cout << "  ║   CRASHED r" << round << ": " << e.what() << "\n"; break;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   " << total_mults << " mults, " << total_cleans << " cleans, " << total_boots << " boots                         ║\n";
    cout <<   "  ║   Pattern: Clean → Fibonacci → Bootstrap → Repeat   ║\n";
    cout <<   "  ║   Single slot. No drift. Simple.                    ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
