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
    params.SetMultiplicativeDepth(200);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool1(cc, keys, 20);
    ZANSAnchorPool pool2(cc, keys, 20);
    ZANSAnchorPool pool3(cc, keys, 20);
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));

    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 1000);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "=== MAX EFFORT TEST ===" << endl;
    cout << "Depth: 200 | Triple ZANS | Fresh Enc(0) | Enc(1) reset every 10" << endl;
    cout << "Range: 2-1000 | 500 steps" << endl;
    cout << string(65, '-') << endl;

    for (int step = 0; step < 500; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));

        // Enc(1) reset every 10 steps
        if (step > 0 && step % 10 == 0) {
            ct = cc->EvalMult(ct, ct_one);
            // Heavy ZANS after reset
            auto fresh0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
            ct = cc->EvalAdd(ct, fresh0);
            ct = pool1.stabilize(ct);
            ct = pool2.stabilize(ct);
            ct = pool3.stabilize(ct);
        }

        // TRIPLE ZANS: Before
        auto fresh0a = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        ct = cc->EvalAdd(ct, fresh0a);
        ct = pool1.stabilize(ct);

        // Multiply
        ct = cc->EvalMult(ct, ct_mult);

        // TRIPLE ZANS: After
        auto fresh0b = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        ct = cc->EvalAdd(ct, fresh0b);
        ct = pool2.stabilize(ct);
        ct = pool3.stabilize(ct);

        if ((step+1) % 50 == 0 || step < 5) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            double scale_deg = ct->GetNoiseScaleDeg();
            bool ok = (dec_val == expected);
            cout << "Step " << setw(4) << step << ": Scale=" << setw(5) << scale_deg
                 << " Dec=" << dec_val << " Exp=" << expected 
                 << " [" << (ok ? "OK" : "FAIL") << "]" << endl;
            if (!ok) {
                cout << "\nFIRST FAILURE at step " << step << endl;
                return 1;
            }
        }
    }
    cout << "\nALL 500 PASSED — MAX EFFORT WORKS!" << endl;
    return 0;
}
