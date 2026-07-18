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

    // Pre-compute 5 different multipliers
    vector<int64_t> mults = {2, 3, 5, 7, 11};
    vector<Ciphertext<DCRTPoly>> ct_mults;
    for (auto m : mults) {
        ct_mults.push_back(cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{m})));
    }

    mt19937_64 rng(42);
    uniform_int_distribution<int> pick(0, 4);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "=== LIMITED POOL TEST (5 multipliers, repeated) ===" << endl;
    cout << "Pool: {2, 3, 5, 7, 11} | Random selection | 1000 steps" << endl;

    for (int i = 0; i < 1000; i++) {
        int idx = pick(rng);
        expected = mod_pos(expected * mults[idx], modulus);
        ct = cc->EvalMult(ct, ct_mults[idx]);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);

        if ((i+1) % 200 == 0) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
            cout << "Step " << (i+1) << ": dec=" << dec << " exp=" << expected 
                 << " [" << (dec == expected ? "OK" : "FAIL") << "]" << endl;
            if (dec != expected) return 1;
        }
    }
    cout << "ALL 1000 PASSED (limited pool)" << endl;
    return 0;
}
