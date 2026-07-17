#include <openfhe.h>
#include <iostream>
#include <iomanip>
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

    // Test A: Same object ×2
    auto ct_same = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
    auto mult_same = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
    int64_t exp_same = 1;

    cout << "=== SAME vs DIFFERENT OBJECT TEST ===" << endl;
    
    // Test A: Same object
    for (int i = 0; i < 100; i++) {
        exp_same = mod_pos(exp_same * 2, modulus);
        ct_same = cc->EvalMult(ct_same, mult_same);
        ct_same = cc->EvalAdd(ct_same, anchor0);
        ct_same = pool.stabilize(ct_same);
    }
    Plaintext pt;
    cc->Decrypt(keys.secretKey, ct_same, &pt); pt->SetLength(1);
    cout << "Same object ×2 (100 steps): " << (mod_pos(pt->GetPackedValue()[0], modulus) == exp_same ? "OK" : "FAIL") << endl;

    // Test B: Different objects ×2 (but all enc(2))
    auto ct_diff = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));
    int64_t exp_diff = 1;
    for (int i = 0; i < 100; i++) {
        exp_diff = mod_pos(exp_diff * 2, modulus);
        auto mult_new = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2})); // FRESH enc(2)!
        ct_diff = cc->EvalMult(ct_diff, mult_new);
        ct_diff = cc->EvalAdd(ct_diff, anchor0);
        ct_diff = pool.stabilize(ct_diff);
    }
    cc->Decrypt(keys.secretKey, ct_diff, &pt); pt->SetLength(1);
    cout << "Different enc(2) objects (100 steps): " << (mod_pos(pt->GetPackedValue()[0], modulus) == exp_diff ? "OK" : "FAIL") << endl;

    return 0;
}
