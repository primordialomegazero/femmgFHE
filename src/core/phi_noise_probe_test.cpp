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
    ZANSAnchorPool pool(cc, keys, 10);

    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 10);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "=== NOISE PROBE: (ct + Enc(0)) - Enc(0) - ct ===" << endl;
    cout << "This measures the noise artifact of Enc(0)!" << endl;
    cout << setw(5) << "Step" << setw(6) << "Mult" << setw(12) << "NoiseProbe" 
         << setw(12) << "ScaleDeg" << setw(12) << "Decrypted" << setw(12) << "Expected" << endl;
    cout << string(65, '-') << endl;

    for (int step = 0; step < 40; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));

        // NOISE PROBE: Use Enc(0) instead of half_mod
        auto Z = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
        auto sum = cc->EvalAdd(ct, Z);
        auto back = cc->EvalSub(sum, Z);
        auto probe = cc->EvalSub(ct, back);  // This is the NOISE FINGERPRINT!

        Plaintext pt_probe;
        cc->Decrypt(keys.secretKey, probe, &pt_probe); pt_probe->SetLength(1);
        int64_t noise_fingerprint = mod_pos(pt_probe->GetPackedValue()[0], modulus);

        ct = cc->EvalMult(ct, ct_mult);
        ct = cc->EvalAdd(ct, Z);  // Use same Z as anchor
        ct = pool.stabilize(ct);

        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
        int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
        double scale = ct->GetNoiseScaleDeg();

        cout << setw(5) << step 
             << setw(6) << mult 
             << setw(12) << noise_fingerprint
             << setw(12) << scale
             << setw(12) << dec_val 
             << setw(12) << expected
             << (dec_val == expected ? " OK" : " FAIL") << endl;
    }
    return 0;
}
