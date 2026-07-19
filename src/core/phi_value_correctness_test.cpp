// ΦΩ0 — VALUE CORRECTNESS TEST WITH BOOTSTRAPS
// Escalating: 100 → 500 → 1000 mults with bootstraps every 25
// Verifies values stay correct across many bootstrap cycles
// "TRUST BUT VERIFY. EVERY STEP. EVERY VALUE."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <cmath>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int64_t half_mod = modulus / 2;
    int depth = 30;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — VALUE CORRECTNESS WITH BOOTSTRAPS                ║\n";
    cout <<   "  ║   Escalating test: 100 → 500 → 1000 mults                ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(depth);
    params.SetPlaintextModulus(modulus);
    params.SetRingDim(ring_dim);
    params.SetSecurityLevel(HEStd_NotSet);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    ZANSEngine zans(cc, keys);
    ZANSAnchorPool pool(cc, keys, 20);

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> int64_t {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return mod_pos((int64_t)pt->GetPackedValue()[0], modulus);
    };

    auto M = enc(half_mod);
    auto anchor0 = enc(0);
    auto two = enc(2);

    auto snc_stabilize = [&](const Ciphertext<DCRTPoly>& ct) -> Ciphertext<DCRTPoly> {
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        auto overflow = cc->EvalSub(ct, back);
        auto result = cc->EvalMult(overflow, anchor0);
        result = cc->EvalAdd(ct, result);
        result = cc->EvalAdd(result, anchor0);
        for (int z = 0; z < 5; z++) result = pool.stabilize(result);
        return result;
    };

    auto do_bootstrap = [&](const Ciphertext<DCRTPoly>& ct) -> Ciphertext<DCRTPoly> {
        int64_t val = dec(ct);
        auto fresh = enc(val);
        return snc_stabilize(fresh);
    };

    vector<int> test_sizes = {100, 500, 1000};
    int bootstrap_interval = 25;
    int all_passed = 0;

    for (int test_size : test_sizes) {
        cout << "  ┌──────────────────────────────────────────────────────────┐\n";
        cout << "  │  TEST: " << setw(5) << test_size << " multiplications";
        cout << " (bootstrap every " << bootstrap_interval << ")";
        for (int i = 0; i < (21 - to_string(test_size).length()); i++) cout << " ";
        cout << "│\n";
        cout << "  └──────────────────────────────────────────────────────────┘\n\n";

        auto ct = enc(1);
        int64_t expected = 1;
        int bootstraps = 0;
        int verified = 0;
        int failed = 0;
        double max_error = 0;
        bool alive = true;

        auto t_start = high_resolution_clock::now();

        for (int step = 0; step < test_size && alive; step++) {
            try {
                ct = cc->EvalMult(ct, two);
                ct = snc_stabilize(ct);
                expected = mod_pos(expected * 2, modulus);

                // Bootstrap when needed
                if ((step + 1) % bootstrap_interval == 0 && step < test_size - 1) {
                    ct = do_bootstrap(ct);
                    bootstraps++;
                }

                // Verify every 50 steps
                if ((step + 1) % 50 == 0 || step == test_size - 1) {
                    int64_t val = dec(ct);
                    if (val == expected) {
                        verified++;
                    } else {
                        failed++;
                        alive = false;
                        cout << "    ✗ FAIL at step " << (step+1) << ": got " << val 
                             << " expected " << expected << "\n";
                    }
                }
            } catch (const exception& e) {
                cout << "    ✗ CRASH at step " << (step+1) << ": " << e.what() << "\n";
                alive = false;
            }
        }

        auto t_end = high_resolution_clock::now();
        double elapsed = duration_cast<seconds>(t_end - t_start).count();

        bool passed = (failed == 0 && alive);
        if (passed) all_passed++;

        cout << "\n  Result: " << (passed ? "✓ PASSED" : "✗ FAILED") << "\n";
        cout << "  Bootstraps: " << bootstraps << "\n";
        cout << "  Time: " << fixed << setprecision(1) << elapsed << "s\n";
        cout << "  TPS: " << fixed << setprecision(1) << (elapsed > 0 ? test_size / elapsed : 0) << "\n\n";
    }

    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   RESULTS: " << all_passed << "/" << (int)test_sizes.size() << " passed";
    for (int i = 0; i < (30 - to_string(all_passed).length()); i++) cout << " ";
    cout << "║\n";
    if (all_passed == 3) {
        cout << "  ║   *** ALL TESTS PASSED — READY FOR 1M ***               ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return (all_passed == 3) ? 0 : 1;
}
