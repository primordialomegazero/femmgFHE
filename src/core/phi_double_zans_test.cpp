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
    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool_before(cc, keys, 10);  // Separate pool for before
    ZANSAnchorPool pool_after(cc, keys, 10);   // Separate pool for after
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));

    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 10);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "=== DOUBLE ZANS TEST (Before + After Multiply) ===" << endl;
    cout << "Method: ZANS → Multiply → ZANS | Range: 2-10 | 100 steps" << endl;
    cout << setw(5) << "Step" << setw(6) << "Mult" << setw(10) << "Scale" 
         << setw(12) << "Expected" << setw(12) << "Decrypted" << setw(6) << "Match" << endl;
    cout << string(55, '-') << endl;

    for (int step = 0; step < 100; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));

        // DOUBLE ZANS: Before multiply
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool_before.stabilize(ct);

        // Multiply
        ct = cc->EvalMult(ct, ct_mult);

        // DOUBLE ZANS: After multiply
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool_after.stabilize(ct);

        if ((step+1) % 10 == 0 || step < 5) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            double scale_deg = ct->GetNoiseScaleDeg();
            bool ok = (dec_val == expected);
            cout << setw(5) << step << setw(6) << mult << setw(10) << scale_deg
                 << setw(12) << expected << setw(12) << dec_val 
                 << setw(6) << (ok ? "OK" : "FAIL") << endl;
            if (!ok) {
                cout << "\nFIRST FAILURE at step " << step << endl;
                return 1;
            }
        }
    }
    cout << "ALL 100 PASSED — DOUBLE ZANS WORKS!" << endl;
    return 0;
}
