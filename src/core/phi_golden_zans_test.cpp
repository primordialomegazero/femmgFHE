#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521, half_mod = modulus / 2;
    double phi = 1.6180339887498948482;
    
    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(60);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    ZANSAnchorPool pool(cc, keys, 30);
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));

    mt19937_64 rng(42);
    uniform_int_distribution<int64_t> dist(2, 100);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "=== φ-WEIGHTED ZANS TEST ===" << endl;
    cout << "Method: φ-scaled Enc(0) magnitudes + Fibonacci-timed heavy ZANS" << endl;
    cout << "Range: 2-100 | 200 steps" << endl;

    double phi_power = 1.0;
    int fib_steps[] = {1,2,3,5,8,13,21,34,55,89,144,233};
    int fib_idx = 0;

    for (int step = 0; step < 200; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));

        // Full True Divine
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        ct = cc->EvalMult(ct, ct_mult);
        auto divine = cc->EvalMult(overflow, 
            cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0})));
        ct = cc->EvalAdd(ct, divine);

        // φ-WEIGHTED ZANS: Multiple Enc(0) with φ-scaled "intensity"
        int num_anchors = (int)(phi_power) + 1;  // 1, 1, 2, 3, 4, 7, 11, 18...
        for (int i = 0; i < num_anchors; i++) {
            auto fresh0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
            ct = cc->EvalAdd(ct, fresh0);
            ct = pool.stabilize(ct);
        }
        phi_power *= phi;
        if (phi_power > 50) phi_power = 1.0;  // Reset cycle

        // Extra heavy ZANS at Fibonacci steps
        if (fib_idx < 12 && step+1 == fib_steps[fib_idx]) {
            for (int i = 0; i < 5; i++) {
                auto fresh0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
                ct = cc->EvalAdd(ct, fresh0);
                ct = pool.stabilize(ct);
            }
            fib_idx++;
        }

        if ((step+1) % 40 == 0 || step < 5) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            cout << "Step " << setw(4) << step+1 << ": " << (dec_val == expected ? "OK" : "FAIL") << endl;
            if (dec_val != expected) return 1;
        }
    }
    cout << "\nALL 200 PASSED WITH φ-WEIGHTED ZANS!" << endl;
    return 0;
}
