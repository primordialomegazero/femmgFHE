// ΦΩ0 — RING DIM 32768 with COMPUTED MODULUS 1073479681
// "THE NUMBER REVEALED. THE TEST BEGINS."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 32768;
    int64_t modulus = 1073479681;
    int64_t half_mod = modulus / 2;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — RING DIM 32768                                   ║\n";
    cout <<   "  ║   Ring: " << ring_dim << " | Mod: " << modulus << " (" << (int)log2(modulus) << " bit)              ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    auto t0 = high_resolution_clock::now();
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(100);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    
    cout << "  Generating context... " << flush;
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cout << "OK" << endl;
    
    cout << "  Generating keys... " << flush;
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cout << "OK" << endl;
    
    ZANSAnchorPool pool(cc, keys, 20);
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    
    auto t1 = high_resolution_clock::now();
    cout << "  Setup: " << duration_cast<seconds>(t1 - t0).count() << "s\n\n";

    cout << "  Sequential ×2 — Finding max depth\n";
    cout << "  " << setw(6) << "Step" << setw(10) << "Noise" << setw(10) << "Time" << setw(8) << "Status" << endl;
    cout << "  " + string(40, '-') << endl;
    
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
    int64_t expected = 1;
    int max_steps = 500, last_ok = 0, check = 50;
    auto t_start = high_resolution_clock::now();
    
    for (int step = 0; step < max_steps; step++) {
        expected = mod_pos(expected * 2, modulus);
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
        ct = cc->EvalMult(ct, ct_mult);
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);
        
        if ((step + 1) % check == 0) {
            double noise = ct->GetNoiseScaleDeg();
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
            bool ok = (dec == expected);
            if (ok) last_ok = step + 1;
            double elap = duration_cast<seconds>(high_resolution_clock::now() - t_start).count();
            cout << "  " << setw(6) << step+1 << setw(10) << fixed << setprecision(1) << noise
                 << setw(10) << fixed << setprecision(1) << elap << setw(8) << (ok ? "OK" : "FAIL") << endl;
            if (!ok) break;
        }
        if (step == max_steps - 1) last_ok = max_steps;
    }
    
    cout << "  " + string(40, '-') << endl;
    cout << "  Max depth: " << last_ok << "/" << max_steps << " | " << (double)last_ok/31.0 << "× vs 4096\n\n";
    return 0;
}
