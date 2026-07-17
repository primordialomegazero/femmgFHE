#include <openfhe.h>
#include <iostream>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t mod) { return ((v % mod) + mod) % mod; }
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
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    int64_t expecteds[20] = {350895783,989818833,363909778,95891439,638568703,637470999,767435203,974191267,477918545,29435631,667600771,290736870,339251719,825573981,295570009,551007926,378089962,761643814,889916976,280492114};
    int64_t mults[20] = {752,12,841,847,824,307,877,379,956,200,278,581,448,167,384,129,897,445,269,577};
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{447342178}));
    for (int i = 0; i < 20; i++) {
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mults[i]}));
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        ct = cc->EvalMult(ct, ct_mult);
        ct = cc->EvalAdd(ct, cc->EvalMult(overflow, anchor0));
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
        cout << "Step " << i << ": dec=" << dec_val << " exp=" << expecteds[i] << " [" << (dec_val == expecteds[i] ? "OK" : "FAIL") << "]" << endl;
        if (dec_val != expecteds[i]) return 1;
    }
    cout << "ALL 20 PASSED" << endl;
    return 0;
}
