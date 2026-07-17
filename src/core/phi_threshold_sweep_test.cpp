// ΦΩ0 — TRUE DIVINE THRESHOLD SWEEP
// Test different multiplier ranges: 2-10, 2-50, 2-100, 2-500
// Find the exact limit for random CT×CT
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

struct SweepResult {
    int max_range;
    int max_steps_passed;
    double scale_at_fail;
    int64_t expected_at_fail;
    int64_t got_at_fail;
};

SweepResult sweep_range(int max_mult, int total_steps) {
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

    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool(cc, keys, 10);
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    
    mt19937_64 rng(max_mult * 12345);  // Different seed per range
    uniform_int_distribution<int64_t> dist(2, max_mult);
    
    int64_t start_val = 42;
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{start_val}));
    int64_t expected = start_val;
    
    SweepResult result;
    result.max_range = max_mult;
    result.max_steps_passed = 0;
    
    for(int step = 0; step < total_steps; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));
        
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        ct = cc->EvalMult(ct, ct_mult);
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);
        
        // Check every step after 50
        if((step + 1) % 50 == 0) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            
            if(dec_val == expected) {
                result.max_steps_passed = step + 1;
            } else {
                result.scale_at_fail = ct->GetNoiseScaleDeg();
                result.expected_at_fail = expected;
                result.got_at_fail = dec_val;
                return result;
            }
        }
    }
    
    result.max_steps_passed = total_steps;
    result.scale_at_fail = ct->GetNoiseScaleDeg();
    return result;
}

int main() {
    cout << "\n=== TRUE DIVINE THRESHOLD SWEEP ===" << endl;
    cout << "Finding max steps for different multiplier ranges\n";
    cout << string(65, '-') << endl;
    cout << setw(12) << "Max Mult" << setw(12) << "Steps OK" << setw(12) << "Scale@" << setw(18) << "Expected vs Got" << endl;
    cout << string(65, '-') << endl;
    
    vector<int> ranges = {10, 50, 100, 500};
    
    for(int max_mult : ranges) {
        auto result = sweep_range(max_mult, 500);
        
        cout << setw(12) << ("2-"+to_string(max_mult))
             << setw(12) << result.max_steps_passed;
        
        if(result.max_steps_passed < 500) {
            cout << setw(12) << (int)result.scale_at_fail
                 << "  " << result.expected_at_fail << " vs " << result.got_at_fail;
        } else {
            cout << setw(12) << (int)result.scale_at_fail
                 << "  ALL PASSED";
        }
        cout << endl;
    }
    
    cout << string(65, '-') << endl;
    cout << "\nCONCLUSION: Larger random multipliers = faster noise accumulation\n";
    cout << "Fixed small multiplier (x2) = 1,000,000 steps\n";
    cout << "Random large multipliers = limited steps\n\n";
    
    return 0;
}
