#include <openfhe.h>
#include <iostream>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t mod) {
    return ((v % mod) + mod) % mod;
}

int main() {
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
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));

    int64_t expecteds[] = {193578710, 932051086, 686152586, 925348058, 207066559};
    int64_t mults[] = {116, 27, 761, 283, 252};

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{686579304}));

    cout << "=== TRUE DIVINE RANDOM CT×CT (with sign fix) ===" << endl;
    for (int i = 0; i < 5; i++) {
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mults[i]}));
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        ct = cc->EvalMult(ct, ct_mult);
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);

        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        int64_t raw = pt->GetPackedValue()[0];
        int64_t dec_val = mod_pos(raw, modulus);
        bool ok = (dec_val == expecteds[i]);
        cout << "Step " << i << ": raw=" << raw << " normalized=" << dec_val
             << " exp=" << expecteds[i] << " [" << (ok ? "OK" : "FAIL") << "]" << endl;
    }
    return 0;
}
