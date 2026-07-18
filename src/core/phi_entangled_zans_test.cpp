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
    params.SetMultiplicativeDepth(60);
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

    cout << "=== ENTANGLED ZERO PAIRS TEST ===" << endl;
    cout << "Method: Every step = ct + Enc(0)_a + Enc(0)_b (fresh pair)" << endl;
    cout << "Each pair: independent Enc(0) with opposite noise?" << endl;
    cout << "Range: 2-10 | 100 steps" << endl;
    cout << string(55, '-') << endl;

    for (int step = 0; step < 100; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));

        ct = cc->EvalMult(ct, ct_mult);

        // ENTANGLED PAIR: Two independent Enc(0) — simulate "entanglement"
        auto zero_a = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        auto zero_b = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        
        ct = cc->EvalAdd(ct, zero_a);
        ct = cc->EvalAdd(ct, zero_b);
        ct = pool.stabilize(ct);

        if ((step+1) % 20 == 0 || step < 5) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            bool ok = (dec_val == expected);
            cout << "Step " << setw(3) << step << ": Scale=" << setw(4) << ct->GetNoiseScaleDeg()
                 << " Dec=" << dec_val << " Exp=" << expected 
                 << " [" << (ok ? "OK" : "FAIL") << "]" << endl;
            if (!ok) {
                cout << "\nFAILED at step " << step << endl;
                return 1;
            }
        }
    }
    cout << "\nALL 100 PASSED!" << endl;
    return 0;
}
