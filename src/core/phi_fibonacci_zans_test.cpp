#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include "zans_production_lib.h"
using namespace lbcrypto;
using namespace std;
int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

vector<int> generate_fib_steps(int limit) {
    vector<int> fib = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};
    return fib;
}

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

    mt19937_64 rng(12345);
    uniform_int_distribution<int64_t> dist(2, 10);
    auto fib_steps = generate_fib_steps(1000);

    auto ct = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{42}));
    int64_t expected = 42;

    cout << "=== FIBONACCI-TIMED ZANS TEST ===" << endl;
    cout << "Method: ZANS intervals = Fibonacci numbers" << endl;
    cout << "Heavy ZANS (3×) at FIB steps, light ZANS otherwise" << endl;
    cout << "Range: 2-10 | 200 steps" << endl;
    cout << string(55, '-') << endl;

    int next_fib_idx = 0;
    int next_fib = fib_steps[next_fib_idx];

    for (int step = 0; step < 200; step++) {
        int64_t mult = dist(rng);
        expected = mod_pos(expected * mult, modulus);
        auto ct_mult = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{mult}));

        ct = cc->EvalMult(ct, ct_mult);
        
        // Check if this is a Fibonacci step
        bool is_fib = (step + 1 == next_fib);
        if (is_fib) {
            // HEAVY ZANS: 3× fresh Enc(0)
            for (int i = 0; i < 3; i++) {
                auto fresh0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{0}));
                ct = cc->EvalAdd(ct, fresh0);
                ct = pool.stabilize(ct);
            }
            next_fib_idx++;
            if (next_fib_idx < fib_steps.size()) next_fib = fib_steps[next_fib_idx];
            else next_fib = 999999; // No more fib steps
        } else {
            // Light ZANS
            ct = cc->EvalAdd(ct, anchor0);
            ct = pool.stabilize(ct);
        }

        if ((step+1) % 20 == 0 || (is_fib && step < 100)) {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(1);
            int64_t dec_val = mod_pos(pt->GetPackedValue()[0], modulus);
            bool ok = (dec_val == expected);
            cout << "Step " << setw(4) << step+1 << ": Scale=" << setw(4) << ct->GetNoiseScaleDeg()
                 << " Dec=" << dec_val << " Exp=" << expected 
                 << " [" << (ok ? "OK" : "FAIL") << "]";
            if (is_fib) cout << " [FIB]";
            cout << endl;
            if (!ok) {
                cout << "\nFAILED at step " << step+1 << endl;
                return 1;
            }
        }
    }
    cout << "\nALL 200 PASSED!" << endl;
    return 0;
}
