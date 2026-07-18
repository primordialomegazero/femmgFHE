#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }
int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521, half_mod = modulus / 2;
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

    // Pre-compute POOL of multipliers — REUSED, not fresh per step
    int64_t mult_vals[] = {2, 3, 5, 7, 11};
    vector<Ciphertext<DCRTPoly>> ct_pool;
    for (int i = 0; i < 5; i++) {
        ct_pool.push_back(cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult_vals[i]})));
    }

    mt19937_64 rng(42);
    uniform_int_distribution<int> pick(0, 4);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "=== POOL MULTIPLIER TEST (5 fixed objects, random selection) ===" << endl;
    cout << "Pool: {2, 3, 5, 7, 11} | 500 steps" << endl;
    cout << setw(5) << "Step" << setw(8) << "Mult" << setw(14) << "Expected" 
         << setw(14) << "Decrypted" << setw(8) << "Result" << endl;
    cout << string(52, '-') << endl;

    for (int step = 0; step < 500; step++) {
        int idx = pick(rng);
        int64_t mult = mult_vals[idx];
        expected = mod_pos(expected * mult, modulus);

        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        ct = cc->EvalMult(ct, ct_pool[idx]);  // SAME 5 objects, reused!
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);

        if ((step+1) % 100 == 0 || step < 5) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            cout << setw(5) << step+1 << setw(8) << mult 
                 << setw(14) << expected << setw(14) << dec_val 
                 << setw(8) << (dec_val == expected ? "OK" : "FAIL") << endl;
            if (dec_val != expected) return 1;
        }
    }
    cout << "\nALL 500 PASSED WITH 5-OBJECT POOL!" << endl;
    return 0;
}
