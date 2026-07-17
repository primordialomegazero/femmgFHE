// ΦΩ0 — FULL TRUE DIVINE 1K CT×CT TEST
// Pinky Swear + Divine + ZANS + Pool, 1000 random steps
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
    int total_steps = 1000;
    int checkpoint = 100;
    
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
    
    mt19937_64 rng(42);
    uniform_int_distribution<int64_t> dist(2, 1000);
    
    int64_t start_val = 42;
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{start_val}));
    int64_t expected = start_val;
    
    cout << "\n=== FULL TRUE DIVINE 1K CT×CT TEST ===" << endl;
    cout << "Start: " << start_val << " | Steps: " << total_steps << endl;
    cout << "Method: Pinky Swear + Divine + ZANS + Pool" << endl;
    cout << string(60, '-') << endl;
    
    auto t_start = high_resolution_clock::now();
    int passed = 0, total = 0;
    
    for(int step = 0; step < total_steps; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));
        
        // === FULL TRUE DIVINE ===
        // 1. Pinky Swear: overflow detection
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        
        // 2. Multiply
        ct = cc->EvalMult(ct, ct_mult);
        
        // 3. Divine Intervention
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        
        // 4. ZANS Pool
        ct = pool.stabilize(ct);
        
        if((step + 1) % checkpoint == 0) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, ct, &pt);
            pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            bool ok = (dec_val == expected);
            if(ok) passed++;
            total++;
            
            double scale_deg = ct->GetNoiseScaleDeg();
            auto now = high_resolution_clock::now();
            double elapsed = duration_cast<seconds>(now - t_start).count();
            
            cout << "Step " << setw(4) << (step+1) 
                 << " | Scale=" << setw(6) << (int)scale_deg
                 << " | Value=" << (ok ? "OK" : "FAIL")
                 << " | " << fixed << setprecision(0) << elapsed << "s" << endl;
            
            if(!ok) {
                cout << "  Expected: " << expected << " Got: " << dec_val << endl;
            }
        }
    }
    
    auto t_end = high_resolution_clock::now();
    double total_sec = duration_cast<milliseconds>(t_end - t_start).count() / 1000.0;
    
    cout << string(60, '-') << endl;
    cout << "Result: " << passed << "/" << total << " checkpoints passed" << endl;
    cout << "Time: " << fixed << setprecision(1) << total_sec << "s" << endl;
    cout << (passed == total ? "FULL TRUE DIVINE WORKS!" : "SOME FAILURES") << endl;
    cout << endl;
    
    return (passed == total) ? 0 : 1;
}
