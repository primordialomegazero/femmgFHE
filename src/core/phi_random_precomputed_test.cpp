// ΦΩ0 — RANDOM CT×CT WITH PRE-COMPUTED MULTIPLIERS
// All ct_mult encrypted BEFORE the loop
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
    mt19937_64 rng(42);
    uniform_int_distribution<int64_t> dist(2, 1000);
    int total = 1000;
    vector<Ciphertext<DCRTPoly>> precomputed;
    vector<int64_t> multipliers;
    for (int i = 0; i < total; i++) {
        int64_t m = dist(rng);
        multipliers.push_back(m);
        precomputed.push_back(cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{m})));
    }
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;
    cout << "=== RANDOM CT×CT WITH PRE-COMPUTED MULTIPLIERS ===" << endl;
    cout << "Steps: " << total << " | Range: 2-1000" << endl;
    for (int i = 0; i < total; i++) {
        expected = mod_pos(expected * multipliers[i], modulus);
        ct = cc->EvalMult(ct, precomputed[i]);
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
    cout << "ALL PASSED" << endl;
    return 0;
}
