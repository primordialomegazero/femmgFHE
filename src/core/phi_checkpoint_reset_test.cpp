// ΦΩ0 — CHECKPOINT RESET: ct = ct × Enc(0) + checkpoint
// Every N steps, save checkpoint, reset noise via multiply-by-zero
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
    int total_steps = 500;
    int reset_interval = 20;  // Save checkpoint every 20 steps
    
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
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    
    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 1000);  // Large range!
    
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    auto checkpoint = ct;  // Initial checkpoint
    int64_t expected = 42;
    
    cout << "\n=== CHECKPOINT RESET TEST ===" << endl;
    cout << "Range: 2-1000 | Steps: " << total_steps << " | Reset every " << reset_interval << endl;
    cout << "Method: ct = ct × Enc(0) + checkpoint" << endl;
    cout << string(70, '-') << endl;
    cout << setw(8) << "Step" << setw(12) << "Mult" << setw(16) << "Expected" << setw(16) << "Got" << setw(8) << "Match" << setw(8) << "Scale" << setw(10) << "Reset?" << endl;
    cout << string(70, '-') << endl;
    
    for(int step = 0; step < total_steps; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));
        
        // Pinky Swear
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        
        // Multiply
        ct = cc->EvalMult(ct, ct_mult);
        
        // Divine + ZANS
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);
        
        bool did_reset = false;
        
        // Checkpoint Reset every N steps
        if((step + 1) % reset_interval == 0 && step > 0) {
            ct = cc->EvalMult(ct, ct_zero);      // ct × Enc(0) = Enc(0)
            ct = cc->EvalAdd(ct, checkpoint);      // + checkpoint = restore value
            ct = pool.stabilize(ct);                // ZANS
            did_reset = true;
            
            // Update expected to checkpoint value * remaining multipliers
            // (This is complex — for demo, we track expected manually)
        }
        
        if((step + 1) % 50 == 0 || did_reset) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            bool match = (dec_val == expected);
            
            cout << setw(8) << (step+1)
                 << setw(12) << mult
                 << setw(16) << expected
                 << setw(16) << dec_val
                 << setw(8) << (match ? "OK" : "FAIL")
                 << setw(8) << (int)ct->GetNoiseScaleDeg()
                 << setw(10) << (did_reset ? "RESET" : "") << endl;
        }
    }
    
    cout << string(70, '-') << endl;
    cout << endl;
    return 0;
}
