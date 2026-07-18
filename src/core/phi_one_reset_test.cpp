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
    params.SetMultiplicativeDepth(60);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool(cc, keys, 20);
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));

    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 10);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "=== ENC(1) RESET TEST ===" << endl;
    cout << "Method: Every 10 steps: ct = ct × Enc(1) + heavy ZANS" << endl;
    cout << "Theory: ×1 preserves value, +ZANS refreshes noise structure" << endl;
    cout << "Range: 2-10 | 200 steps | Reset every 10" << endl;
    cout << string(55, '-') << endl;

    for (int step = 0; step < 200; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));

        // Full True Divine
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        ct = cc->EvalMult(ct, ct_mult);
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);

        // ENC(1) RESET every 10 steps
        if ((step+1) % 10 == 0) {
            ct = cc->EvalMult(ct, ct_one);  // ×1 — preserve value
            // Heavy ZANS after ×1
            for (int i = 0; i < 3; i++) {
                auto fresh0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
                ct = cc->EvalAdd(ct, fresh0);
                ct = pool.stabilize(ct);
            }
        }

        if ((step+1) % 20 == 0 || step < 5) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            cout << "Step " << setw(4) << step+1 << ": " << (dec_val == expected ? "OK" : "FAIL") << endl;
            if (dec_val != expected) return 1;
        }
    }
    cout << "\nALL 200 PASSED WITH ENC(1) RESET!" << endl;
    return 0;
}
