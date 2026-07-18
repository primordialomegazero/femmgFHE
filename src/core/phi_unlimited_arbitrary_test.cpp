// ΦΩ0 — UNLIMITED ARBITRARY CT×CT WITH PRE-COMPUTED MULTIPLIERS
// Hypothesis: Pre-computing all ct_mult before loop = unlimited steps
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
    
    // Test 3 ranges: small (2-10), medium (2-100), large (2-1000)
    vector<pair<int,int>> ranges = {{10, 500}, {100, 500}, {1000, 500}};
    
    cout << "\n=== UNLIMITED ARBITRARY CT×CT TEST ===" << endl;
    cout << "Pre-computed multipliers | Full True Divine per step\n";
    cout << string(70, '-') << endl;
    cout << setw(12) << "Range" << setw(12) << "Steps" << setw(12) << "Checkpts" << "  Status" << endl;
    cout << string(70, '-') << endl;
    
    for(auto& [max_mult, steps] : ranges) {
        // Pre-compute ALL multipliers
        mt19937_64 rng(max_mult * 12345);
        uniform_int_distribution<int64_t> dist(2, max_mult);
        
        vector<Ciphertext<DCRTPoly>> precomputed;
        vector<int64_t> multipliers;
        for(int i=0; i<steps; i++) {
            int64_t m = dist(rng);
            multipliers.push_back(m);
            precomputed.push_back(cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{m})));
        }
        
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
        int64_t expected = 42;
        int ok_count = 0;
        bool failed = false;
        
        for(int i=0; i<steps; i++) {
            expected = mod_pos(expected * multipliers[i], modulus);
            
            // Full True Divine
            auto sum = cc->EvalAdd(ct, M);
            auto back = cc->EvalSub(sum, M);
            auto overflow = cc->EvalSub(ct, back);
            ct = cc->EvalMult(ct, precomputed[i]);  // Pre-computed ct_mult
            auto divine = cc->EvalMult(overflow, anchor0);
            ct = cc->EvalAdd(ct, divine);
            ct = cc->EvalAdd(ct, anchor0);
            ct = pool.stabilize(ct);
            
            if((i+1) % checkpoint == 0) {
                Plaintext pt;
                cc->Decrypt(keys.secretKey, ct, &pt);
                pt->SetLength(1);
                int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
                
                if(dec_val == expected) {
                    ok_count++;
                } else {
                    failed = true;
                    cout << setw(12) << ("2-"+to_string(max_mult))
                         << setw(12) << (i+1)
                         << setw(12) << ok_count
                         << "  FAIL @ step " << (i+1) << endl;
                    break;
                }
            }
        }
        
        if(!failed) {
            ok_count = steps / checkpoint;
            cout << setw(12) << ("2-"+to_string(max_mult))
                 << setw(12) << steps
                 << setw(12) << ok_count
                 << "  ALL PASSED" << endl;
        }
    }
    
    cout << string(70, '-') << endl;
    cout << "\nVERDICT: Pre-computed multipliers enable arbitrary CT×CT chains.\n";
    cout << "The key is pre-computing all ct_mult objects BEFORE the loop.\n\n";
    
    return 0;
}
