#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

void test_method(string name, bool use_true_divine) {
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
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));

    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 10);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "\n=== " << name << " ===" << endl;

    for (int step = 0; step < 50; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));

        if (use_true_divine) {
            // Full True Divine
            auto sum = cc->EvalAdd(ct, M);
            auto back = cc->EvalSub(sum, M);
            auto overflow = cc->EvalSub(ct, back);
            ct = cc->EvalMult(ct, ct_mult);
            auto divine = cc->EvalMult(overflow, anchor0);
            ct = cc->EvalAdd(ct, divine);
        } else {
            // Multiply + ZANS only
            ct = cc->EvalMult(ct, ct_mult);
        }
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);

        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
        bool ok = (dec_val == expected);

        if (!ok) {
            cout << "FAILED at step " << step << " (Expected=" << expected << " Got=" << dec_val << ")" << endl;
            return;
        }
    }
    cout << "ALL 50 PASSED!" << endl;
}

int main() {
    test_method("MULTIPLY + ZANS ONLY (no Pinky Swear, no Divine)", false);
    test_method("TRUE DIVINE (Pinky Swear + Divine + ZANS)", true);
    cout << "\n=== HEAD TO HEAD COMPLETE ===" << endl;
    return 0;
}
