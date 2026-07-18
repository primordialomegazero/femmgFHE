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
    ZANSAnchorPool pool(cc, keys, 20);

    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 10);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "=== TRIPLE ZERO ZANS TEST ===" << endl;
    cout << "Method: ct += Enc(0)_a + Enc(0)_b + Enc(0)_c (fresh each)" << endl;
    cout << "Range: 2-10 | 200 steps" << endl;
    cout << string(55, '-') << endl;

    for (int step = 0; step < 200; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));

        ct = cc->EvalMult(ct, ct_mult);

        // TRIPLE ZERO: Three independent Enc(0)
        auto z1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        auto z2 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        auto z3 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        
        ct = cc->EvalAdd(ct, z1);
        ct = cc->EvalAdd(ct, z2);
        ct = cc->EvalAdd(ct, z3);
        ct = pool.stabilize(ct);

        if ((step+1) % 25 == 0 || step < 5) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            bool ok = (dec_val == expected);
            cout << "Step " << setw(4) << step+1 << ": Scale=" << setw(4) << ct->GetNoiseScaleDeg()
                 << " Dec=" << dec_val << " Exp=" << expected 
                 << " [" << (ok ? "OK" : "FAIL") << "]" << endl;
            if (!ok) {
                cout << "\nFAILED at step " << step+1 << endl;
                return 1;
            }
        }
    }
    cout << "\nALL 200 PASSED WITH TRIPLE ZERO!" << endl;
    return 0;
}
