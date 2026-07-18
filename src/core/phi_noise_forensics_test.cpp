#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>
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

    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 10);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "=== NOISE FORENSICS: Tracking all noise metrics ===" << endl;
    cout << setw(5) << "Step" << setw(6) << "Mult" << setw(10) << "ScaleDeg"
         << setw(12) << "LogNoise" << setw(12) << "PTxtVal" 
         << setw(14) << "Decrypted" << setw(6) << "Match" << endl;
    cout << string(70, '-') << endl;

    for (int step = 0; step < 40; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));

        // Measure noise BEFORE
        double scale_before = ct->GetNoiseScaleDeg();
        double log_noise_before = log2(ct->GetNoiseScaleDeg() + 1);

        ct = cc->EvalMult(ct, ct_mult);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);

        // Measure noise AFTER
        double scale_after = ct->GetNoiseScaleDeg();
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
        bool match = (dec_val == expected);

        cout << setw(5) << step 
             << setw(6) << mult 
             << setw(10) << scale_after
             << setw(12) << fixed << setprecision(2) << log2(scale_after)
             << setw(12) << expected
             << setw(14) << dec_val 
             << setw(6) << (match ? "OK" : "FAIL") << endl;

        if (!match) {
            cout << "\n=== FAILURE ANALYSIS ===" << endl;
            cout << "Failed at step " << step << endl;
            cout << "Scale before fail: " << scale_before << " -> Scale after: " << scale_after << endl;
            cout << "Expected: " << expected << " Got: " << dec_val << endl;
            cout << "Difference: " << mod_pos(dec_val - expected, modulus) << " (mod " << modulus << ")" << endl;
            return 0;
        }
    }
    return 0;
}
