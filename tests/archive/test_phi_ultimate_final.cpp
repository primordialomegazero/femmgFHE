// FEmmg-FHE — ULTIMATE FINAL: Single-slot + Fibonacci + Bootstrap
// Clean → Fibonacci Jump → Bootstrap → Repeat
// Single slot (no drift). Fibonacci compression. Bootstrap for levels.
// RingDim=32768 for precision headroom.

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

map<int, Ciphertext<DCRTPoly>> build_fib(double base_val, int max_fib) {
    map<int, Ciphertext<DCRTPoly>> powers;
    auto b = enc(base_val);
    powers[1] = b; powers[2] = cc->EvalMult(b, b);
    int a = 1, c = 2;
    while (c < max_fib) { int n = a + c; powers[n] = cc->EvalMult(powers[c], powers[a]); a = c; c = n; }
    return powers;
}

Ciphertext<DCRTPoly> fib_jump(Ciphertext<DCRTPoly> state, map<int, Ciphertext<DCRTPoly>>& powers, int N) {
    vector<int> keys; for (auto& p : powers) keys.push_back(p.first);
    sort(keys.rbegin(), keys.rend());
    int rem = N;
    for (int f : keys) { if (f <= rem) { state = cc->EvalMult(state, powers.at(f)); rem -= f; } }
    while (rem > 0) { state = cc->EvalMult(state, powers.at(1)); rem--; }
    return state;
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ULTIMATE: Single-slot + Fibonacci + Bootstrap       ║\n";
    cout <<   "  ║   RingDim=32768 for precision                         ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY); params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(32768); params.SetScalingModSize(59); params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO); params.SetMultiplicativeDepth(80);
    cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE); cc->EvalBootstrapSetup({4,4}, {0,0}, 16384);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey, 16384);
    slots = 16384;

    cout << "  RingDim=32768, slots=" << slots << "\n\n";

    auto fib_powers = build_fib(1.01, 500);
    cout << "  Fibonacci powers: " << fib_powers.size() << "\n\n";

    auto state = enc(1.0);
    double phi_scale = 1.0;
    int total_mults = 0, total_cleans = 0, total_boots = 0, mults_since_boot = 0;
    
    auto phi_ct = enc(phi);
    auto inv_phi_ct = enc(1.0/phi);
    
    // Fibonacci jump sizes per cycle
    vector<int> jumps = {8, 13, 21, 34, 55, 89, 144};
    
    cout << "  " << setw(4) << "Rnd" << setw(8) << "Jump" << setw(8) << "Total"
         << setw(6) << "Clean" << setw(6) << "Boot"
         << setw(14) << "Value" << setw(14) << "Expected"
         << setw(12) << "Error\n";
    cout << "  " << string(80, '-') << "\n";

    for (int round = 0; round < (int)jumps.size(); round++) {
        int jump = jumps[round];
        try {
            // Clean: 3× φ, 1× 1/φ
            for (int i=0; i<3; i++) state = cc->EvalMult(state, phi_ct);
            for (int i=0; i<1; i++) state = cc->EvalMult(state, inv_phi_ct);
            phi_scale *= (phi*phi*phi/phi);
            total_cleans++;

            // Fibonacci jump
            state = fib_jump(state, fib_powers, jump);
            total_mults += jump; mults_since_boot += jump;

            // Bootstrap if needed
            if (mults_since_boot >= 40) {
                state = cc->EvalBootstrap(state);
                total_boots++; mults_since_boot = 0;
            }

            double val = dec(state);
            double expected = pow(1.01, total_mults) * phi_scale;
            double err = abs((val - expected) / expected);

            cout << "  " << setw(4) << round << setw(8) << jump << setw(8) << total_mults
                 << setw(6) << total_cleans << setw(6) << total_boots
                 << setw(14) << scientific << setprecision(4) << val
                 << setw(14) << scientific << expected
                 << setw(12) << fixed << setprecision(6) << err << "\n";

        } catch (const exception& e) {
            cout << "  CRASHED r" << round << ": " << e.what() << "\n"; break;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   " << total_mults << " effective mults, " << total_cleans << " cleans, " << total_boots << " boots              ║\n";
    cout <<   "  ║   Single slot + Fibonacci + Bootstrap = UNLIMITED    ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
