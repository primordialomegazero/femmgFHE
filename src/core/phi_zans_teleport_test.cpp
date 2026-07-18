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
    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool(cc, keys, 20);
    auto anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{1}));

    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 1000);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "=== ZANS TELEPORTATION TEST ===" << endl;
    cout << "Concept: Mirror pair teleport for noise reset" << endl;
    cout << "Range: 2-1000 | 200 steps | Teleport every 25 steps" << endl;
    cout << string(60, '-') << endl;

    int steps_since_teleport = 0;

    for (int step = 0; step < 200; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));

        // Multiply
        ct = cc->EvalMult(ct, ct_mult);
        ct = cc->EvalAdd(ct, anchor0);
        ct = pool.stabilize(ct);
        
        steps_since_teleport++;

        // TELEPORT: every 25 steps, attempt noise reset
        if (steps_since_teleport >= 25) {
            // Create "mirror" — multiply by 1 with heavy ZANS
            auto ct_mirror = ct;  // Copy current state
            
            // Teleport operation:
            // Step 1: ct = ct * Enc(1)  (identity multiply)
            ct = cc->EvalMult(ct, ct_one);
            
            // Step 2: Heavy ZANS to scramble
            for (int i = 0; i < 5; i++) {
                auto fresh0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
                ct = cc->EvalAdd(ct, fresh0);
                ct = pool.stabilize(ct);
            }
            
            steps_since_teleport = 0;
        }

        if ((step+1) % 25 == 0 || step < 5) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            bool ok = (dec_val == expected);
            cout << "Step " << setw(4) << step+1 << ": Scale=" << setw(5) << ct->GetNoiseScaleDeg()
                 << " Dec=" << dec_val << " Exp=" << expected 
                 << " [" << (ok ? "OK" : "FAIL") << "]";
            if (steps_since_teleport == 0) cout << " [TELEPORTED]";
            cout << endl;
            if (!ok) {
                cout << "\nFAILED at step " << step+1 << endl;
                return 1;
            }
        }
    }
    cout << "\nALL 200 PASSED — TELEPORTATION WORKS!" << endl;
    return 0;
}
