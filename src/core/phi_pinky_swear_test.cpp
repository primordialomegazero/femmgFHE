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
    params.SetMultiplicativeDepth(5);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen();
    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    int64_t tests[] = {42, half_mod - 100, half_mod, half_mod + 100, modulus - 42};
    cout << "=== PINKY SWEAR OVERFLOW DETECTION ===" << endl;
    cout << "mod=" << modulus << " half=" << half_mod << endl;
    for (int i = 0; i < 5; i++) {
        int64_t v = tests[i];
        auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow_ct = cc->EvalSub(ct, back);
        Plaintext pt_sum, pt_back, pt_over;
        cc->Decrypt(keys.secretKey, sum, &pt_sum); pt_sum->SetLength(1);
        cc->Decrypt(keys.secretKey, back, &pt_back); pt_back->SetLength(1);
        cc->Decrypt(keys.secretKey, overflow_ct, &pt_over); pt_over->SetLength(1);
        int64_t sum_v = mod_pos(pt_sum->GetPackedValue()[0], modulus);
        int64_t back_v = mod_pos(pt_back->GetPackedValue()[0], modulus);
        int64_t over_v = mod_pos(pt_over->GetPackedValue()[0], modulus);
        int64_t expected_over = mod_pos(v - back_v, modulus);
        bool over_ok = (over_v == expected_over);
        cout << "v=" << v << " sum=" << sum_v << " back=" << back_v 
             << " overflow=" << over_v << " (expect=" << expected_over << ") "
             << (over_ok ? "OK" : "FAIL") << endl;
        if (!over_ok) return 1;
    }
    cout << "PINKY SWEAR: ALL OK" << endl;
    return 0;
}
