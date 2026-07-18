// ΦΩ0 — ×3 to 100 STEPS — GROUND TRUTH VERIFICATION
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
    int64_t half_mod = modulus / 2;
    int steps = 100;

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    ZANSAnchorPool pool(cc, keys, 10);

    auto M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{half_mod}));
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{2}));
    auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{3}));
    int64_t expected = 2;

    cout << "=== ×3 SEQUENTIAL — 100 STEPS (FULL DIVINE LOOP) ===" << endl;
    cout << "Start: 2 | Steps: " << steps << " | Modulus: " << modulus << endl;
    cout << string(55, '-') << endl;

    int last_ok = 0;
    for (int i = 0; i < steps; i++) {
        expected = mod_pos(expected * 3, modulus);

        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        ct = cc->EvalMult(ct, ct_mult);
        auto divine = cc->EvalMult(overflow, anchor0);
        ct = cc->EvalAdd(ct, divine);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);

        if ((i+1) % 10 == 0 || i == steps-1) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec = mod_pos(pt->GetPackedValue()[0], modulus);
            bool ok = (dec == expected);
            if (ok) last_ok = i+1;
            cout << "Step " << setw(3) << i+1 << " | Expected: " << setw(12) << expected
                 << " | Got: " << setw(12) << dec << " | " << (ok ? "OK" : "FAIL") << endl;
            if (!ok) break;
        }
    }
    cout << string(55, '-') << endl;
    cout << "Max steps: " << last_ok << "/" << steps << endl;
    return (last_ok == steps) ? 0 : 1;
}
