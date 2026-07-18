#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int test(bool prestabilize_mult) {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
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
    ZANSAnchorPool pool(cc, keys, 20);
    ZANSAnchorPool mult_pool(cc, keys, 20);
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));

    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 1000);

    // Pre-compute 50 multipliers
    vector<int64_t> mult_vals;
    vector<Ciphertext<DCRTPoly>> mult_cts;
    for (int i = 0; i < 50; i++) {
        int64_t m = dist(rng);
        mult_vals.push_back(m);
        auto ct_m = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{m}));
        if (prestabilize_mult) {
            // Pre-stabilize the multiplier with ZANS
            ct_m = cc->EvalAdd(ct_m, anchor0);
            ct_m = mult_pool.stabilize(ct_m);
        }
        mult_cts.push_back(ct_m);
    }
    uniform_int_distribution<int> pick(0, 49);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    for (int step = 0; step < 200; step++) {
        int idx = pick(rng);
        expected = mod_pos(expected * mult_vals[idx], modulus);
        ct = cc->EvalMult(ct, mult_cts[idx]);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);

        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        if (mod_pos(pt->GetPackedValue()[0], modulus) != expected) return step;
    }
    return 200;
}

int main() {
    cout << "\n=== PRE-STABILIZED MULTIPLIERS TEST ===" << endl;
    cout << "Pool: 50 unique | Range: 2-1000 | Max 200 steps" << endl;
    cout << string(50, '-') << endl;

    int normal = test(false);
    cout << "Normal multipliers:     " << setw(4) << normal << " steps" << endl;

    int prestab = test(true);
    cout << "Pre-stabilized mults:   " << setw(4) << prestab << " steps" << endl;

    cout << string(50, '-') << endl;
    if (prestab > normal) {
        cout << "IMPROVEMENT: +" << (prestab - normal) << " steps!" << endl;
    } else {
        cout << "No significant difference" << endl;
    }
    return 0;
}
