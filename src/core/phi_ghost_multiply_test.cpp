// ΦΩ0 — GHOST MULTIPLY TEST (Pre-Multiply Noise Reset)
// ct = ct × Enc(1) + heavy ZANS → resets noise floor
// Then ct = ct × actual_mult + ZANS
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int64_t half_mod = modulus / 2;
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(60);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool(cc, keys, 10);
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
    
    mt19937_64 rng(12345);
    
    cout << "\n=== GHOST MULTIPLY TEST (Pre-Multiply Reset) ===" << endl;
    cout << string(60, '-') << endl;
    
    // Test different ranges
    vector<int> ranges = {10, 50, 100, 500};
    
    for(int max_mult : ranges) {
        uniform_int_distribution<int64_t> dist(2, max_mult);
        int64_t start_val = 42;
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{start_val}));
        int64_t expected = start_val;
        
        int max_ok = 0;
        int total_steps = 200;
        
        for(int step = 0; step < total_steps; step++) {
            int64_t mult = dist(rng);
            expected = mod_pos(expected * mult, modulus);
            
            auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));
            
            // === GHOST MULTIPLY: Reset noise floor before actual multiply ===
            ct = cc->EvalMult(ct, ct_one);          // Multiply by 1 (no value change)
            for(int i=0; i<5; i++) ct = cc->EvalAdd(ct, anchor0);  // Heavy ZANS
            ct = pool.stabilize(ct);                  // Pool reset
            
            // Pinky Swear
            auto sum = cc->EvalAdd(ct, M);
            auto back = cc->EvalSub(sum, M);
            auto overflow = cc->EvalSub(ct, back);
            
            // Actual multiply
            ct = cc->EvalMult(ct, ct_mult);
            
            // Divine + ZANS
            auto divine = cc->EvalMult(overflow, anchor0);
            ct = cc->EvalAdd(ct, divine);
            ct = cc->EvalAdd(ct, anchor0);
            ct = pool.stabilize(ct);
            
            if((step + 1) % 50 == 0) {
                Plaintext pt;
                cc->Decrypt(keys.secretKey, ct, &pt);
                pt->SetLength(1);
                int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
                
                if(dec_val == expected) {
                    max_ok = step + 1;
                } else {
                    break;
                }
            }
        }
        
        cout << "Range 2-" << setw(4) << max_mult << ": " << setw(4) << max_ok << "/" << total_steps;
        if(max_ok < total_steps) cout << " (FAIL @ " << max_ok+50 << ")";
        else cout << " (ALL PASSED)";
        cout << endl;
    }
    
    cout << string(60, '-') << endl;
    cout << endl;
    return 0;
}
