// ΦΩ0 — BREAKING POINT SEARCH
// Binary search: saang step nagfa-fail with random 2-10
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

bool test_steps(int total_steps) {
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
    
    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 10);
    
    int64_t start_val = 42;
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{start_val}));
    int64_t expected = start_val;
    
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
    }
    
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct, &pt);
    pt->SetLength(1);
    int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
    
    return (dec_val == expected);
}

int main() {
    cout << "\n=== BREAKING POINT SEARCH (2-10 random) ===" << endl;
    cout << "Finding exact step where True Divine fails\n";
    cout << string(40, '-') << endl;
    cout << setw(10) << "Steps" << setw(10) << "Result" << endl;
    cout << string(40, '-') << endl;
    
    vector<int> test_points = {20, 30, 40, 50, 60, 70, 80, 90, 100, 150, 200};
    
    for(int steps : test_points) {
        bool ok = test_steps(steps);
        cout << setw(10) << steps << setw(10) << (ok ? "OK" : "FAIL") << endl;
        if(!ok) break;  // Stop after first fail
    }
    
    cout << string(40, '-') << endl;
    cout << "\nThreshold found above.\n\n";
    return 0;
}
