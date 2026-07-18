#include <openfhe.h>
#include <iostream>
#include <random>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int test_pool(int pool_size) {
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
    ZANSAnchorPool pool(cc, keys, 10);
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));

    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 1000);
    
    // Pre-compute pool_size unique multipliers
    vector<int64_t> mult_vals;
    vector<Ciphertext<DCRTPoly>> mult_cts;
    for (int i = 0; i < pool_size; i++) {
        int64_t m = dist(rng);
        mult_vals.push_back(m);
        mult_cts.push_back(cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{m})));
    }
    uniform_int_distribution<int> pick(0, pool_size-1);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    for (int step = 0; step < 500; step++) {
        int idx = pick(rng);
        expected = mod_pos(expected * mult_vals[idx], modulus);
        ct = cc->EvalMult(ct, mult_cts[idx]);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);

        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        if (mod_pos(pt->GetPackedValue()[0], modulus) != expected) return step;
    }
    return 500;
}

int main() {
    cout << "\n=== CIPHERTEXT POOL SIZE SWEEP ===" << endl;
    cout << setw(12) << "Pool Size" << setw(15) << "Steps Passed" << endl;
    cout << string(30, '-') << endl;
    for (int size : {1, 2, 3, 5, 10, 20, 50, 100}) {
        int steps = test_pool(size);
        cout << setw(12) << size << setw(15) << steps << endl;
    }
}
