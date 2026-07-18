// ΦΩ0 — DIVINE OVERFLOW ANALYSIS
// Maps relationship: multiplier value → overflow magnitude → noise growth
// Goal: Find the normalization factor for arbitrary multipliers
// "KNOW THY ENEMY. THE OVERFLOW REVEALS ITS SECRETS."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <cmath>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int64_t half_mod = modulus / 2;

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    ZANSAnchorPool pool(cc, keys, 10);
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   DIVINE OVERFLOW ANALYSIS — Arbitrary Multiplier Map     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    // Test multipliers: small (2-10), medium (11-100), large (101-1000), huge (1001+)
    vector<int64_t> test_multipliers = {2, 3, 5, 10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000};
    
    cout << "  " << setw(10) << "Multiplier" 
         << setw(12) << "Pre-Noise" << setw(12) << "Post-Mul"
         << setw(12) << "Post-Divine" << setw(10) << "Reduction"
         << setw(8) << "Status" << endl;
    cout << "  " + string(70, '-') << endl;

    for (auto mult : test_multipliers) {
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
        double noise_start = ct->GetNoiseScaleDeg();
        
        // Pinky Swear
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        
        // Multiply
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));
        ct = cc->EvalMult(ct, ct_mult);
        double noise_post_mul = ct->GetNoiseScaleDeg();
        
        // Divine
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);
        double noise_post_divine = ct->GetNoiseScaleDeg();
        
        // Verify
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
        int64_t expected = mod_pos(42 * mult, modulus);
        bool ok = (dec == expected);
        
        double reduction = noise_post_mul - noise_post_divine;
        
        cout << "  " << setw(10) << mult
             << setw(12) << fixed << setprecision(2) << noise_start
             << setw(12) << noise_post_mul
             << setw(12) << noise_post_divine
             << setw(10) << reduction
             << setw(8) << (ok ? "OK" : "FAIL") << endl;
    }

    // Test sequential chain with different multipliers — find failure point
    cout << "\n\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   SEQUENTIAL CHAIN FAILURE POINT BY MULTIPLIER TYPE      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    vector<pair<string,int64_t>> chain_tests = {
        {"×2 (small)", 2},
        {"×3 (small)", 3},
        {"×10 (medium)", 10},
        {"×100 (large)", 100},
        {"×1000 (huge)", 1000}
    };

    for (auto& [label, mult] : chain_tests) {
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
        int max_steps = 500;
        int last_ok = 0;
        int64_t expected = 1;
        
        for (int step = 0; step < max_steps; step++) {
            expected = mod_pos(expected * mult, modulus);
            
            auto sum = cc->EvalAdd(ct, M);
            auto back = cc->EvalSub(sum, M);
            auto overflow = cc->EvalSub(ct, back);
            
            auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));
            ct = cc->EvalMult(ct, ct_mult);
            
            auto divine = cc->EvalMult(overflow, anchor0);
            ct = cc->EvalAdd(ct, divine);
            ct = cc->EvalAdd(ct, anchor0);
            ct = pool.stabilize(ct);
            
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
            
            if (dec == expected) {
                last_ok = step + 1;
            } else {
                break;
            }
        }
        
        cout << "  " << setw(20) << label 
             << " | Max steps: " << setw(4) << last_ok
             << " | Noise@fail: " << ct->GetNoiseScaleDeg() << endl;
    }

    cout << "\n  Analysis complete.\n\n";
    return 0;
}
