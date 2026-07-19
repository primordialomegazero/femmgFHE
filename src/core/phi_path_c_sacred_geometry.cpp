// ΦΩ0 — PATH C: SACRED GEOMETRY — φ-CYCLIC PRIME ROTATION
// Access internal CRT structure of ciphertext
// Attempt φ-based level stabilization
// "THE GOLDEN RATIO IS THE FIXED POINT OF SELF-REFERENCE"
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    int depth = 30;
    const double phi = 1.6180339887498948482;

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — PATH C: SACRED GEOMETRY                          ║\n";
    cout <<   "  ║   φ-Cyclic Prime Rotation for Chain Stabilization       ║\n";
    cout <<   "  ║   φ = 1.6180339887498948482                              ║\n";
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

    auto enc = [&](int64_t v) {
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vector<int64_t>{v}));
    };

    auto dec = [&](const Ciphertext<DCRTPoly>& ct) -> int64_t {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return mod_pos((int64_t)pt->GetPackedValue()[0], modulus);
    };

    // ============ SACRED APPROACH 1: φ-Interval Divine ============
    cout << "  === φ-INTERVAL DIVINE STABILIZATION ===\n";
    cout << "  Instead of every step, apply Divine at φ-intervals\n\n";

    auto ct_phi = enc(1);
    int64_t expected_phi = 1;
    int divine_count = 0;
    double next_divine_step = 1.0;

    cout << "  Step | Value        | Expected     | Noise | Divine | OK\n";
    cout << "  " + string(70, '-') + "\n";

    for (int step = 1; step <= 40; step++) {
        try {
            ct_phi = cc->EvalMult(ct_phi, enc(2));
            expected_phi = mod_pos(expected_phi * 2, modulus);

            bool apply_divine = false;
            if (step >= (int)next_divine_step) {
                apply_divine = true;
                divine_count++;
                next_divine_step = floor(next_divine_step * phi);
                if (next_divine_step <= step) next_divine_step = step + 1;
            }

            if (apply_divine) {
                auto M_ct = enc(modulus / 2);
                auto sum = cc->EvalAdd(ct_phi, M_ct);
                auto back = cc->EvalSub(sum, M_ct);
                auto overflow = cc->EvalSub(ct_phi, back);
                auto divine = cc->EvalMult(overflow, enc(0));
                ct_phi = cc->EvalAdd(ct_phi, divine);
                ct_phi = cc->EvalAdd(ct_phi, enc(0));
                for (int z = 0; z < 3; z++) {
                    ct_phi = cc->EvalAdd(ct_phi, enc(0));
                }
            }

            int64_t val = dec(ct_phi);
            double noise = ct_phi->GetNoiseScaleDeg();
            bool ok = (val == expected_phi);

            cout << "  " << setw(3) << step << "  | " << setw(12) << val
                 << " | " << setw(12) << expected_phi
                 << " | " << setw(4) << fixed << setprecision(0) << noise
                 << " | " << (apply_divine ? "  φ  " : "     ")
                 << " | " << (ok ? "✓" : "✗") << "\n";

            if (!ok) {
                cout << "\n  >>> DIVERGENCE at step " << step 
                     << " | Divine count: " << divine_count << "\n";
                break;
            }

        } catch (const exception& e) {
            cout << "\n  >>> CRASH at step " << step << ": " << e.what() << "\n";
            break;
        }
    }

    // ============ SACRED APPROACH 2: φ-Scaled Enc(0) pool ============
    cout << "\n  === φ-SCALED Enc(0) ANCHOR POOL ===\n";
    cout << "  Anchors weighted by Fibonacci ratios\n\n";

    auto ct_fib = enc(1);
    int64_t expected_fib = 1;
    int fib_a = 1, fib_b = 1;

    cout << "  Step | Value        | Expected     | Noise | Fib | OK\n";
    cout << "  " + string(65, '-') + "\n";

    for (int step = 1; step <= 40; step++) {
        try {
            ct_fib = cc->EvalMult(ct_fib, enc(2));
            expected_fib = mod_pos(expected_fib * 2, modulus);

            int fib_n = fib_a;
            int next_fib = fib_a + fib_b;
            fib_a = fib_b;
            fib_b = next_fib;

            for (int z = 0; z < min(fib_n, 10); z++) {
                ct_fib = cc->EvalAdd(ct_fib, enc(0));
            }

            int64_t val = dec(ct_fib);
            double noise = ct_fib->GetNoiseScaleDeg();
            bool ok = (val == expected_fib);

            cout << "  " << setw(3) << step << "  | " << setw(12) << val
                 << " | " << setw(12) << expected_fib
                 << " | " << setw(4) << fixed << setprecision(0) << noise
                 << " | " << setw(2) << fib_n
                 << " | " << (ok ? "✓" : "✗") << "\n";

            if (!ok) {
                cout << "\n  >>> DIVERGENCE at step " << step << "\n";
                break;
            }

        } catch (const exception& e) {
            cout << "\n  >>> CRASH at step " << step << ": " << e.what() << "\n";
            break;
        }
    }

    // ============ SACRED APPROACH 3: CRT Internal Structure ============
    cout << "\n  === CRT INTERNAL STRUCTURE ANALYSIS ===\n";
    cout << "  Peeking inside the DCRTPoly to understand chain levels\n\n";

    auto ct_crt = enc(42);
    auto elements = ct_crt->GetElements();

    cout << "  Number of CRT towers: " << elements.size() << "\n";
    cout << "  Each element represents a residue modulo one prime in the chain\n\n";

    for (size_t i = 0; i < elements.size(); i++) {
        auto& poly = elements[i];
        cout << "  Tower " << i << ": degree=" << poly.GetLength()
             << " modulus=" << poly.GetModulus().ConvertToInt()
             << " format=" << (poly.GetFormat() == Format::EVALUATION ? "EVAL" : "COEFF")
             << "\n";
    }

    cout << "\n  Tower 0 first 5 coefficients:\n";
    auto& poly0 = elements[0];
    size_t limit = (poly0.GetLength() < 5) ? poly0.GetLength() : 5;
    for (size_t j = 0; j < limit; j++) {
        cout << "    coeff[" << j << "] = " << poly0[j].ConvertToInt() << "\n";
    }

    // ============ SACRED APPROACH 4: THE φ FIXED POINT ============
    cout << "\n  === THE φ FIXED POINT: Self-Referential Stabilization ===\n";
    cout << "  φ = 1 + 1/φ\n";
    cout << "  Can we create a self-stabilizing ciphertext loop?\n\n";

    auto ct_self = enc(1);
    int64_t expected_self = 1;

    cout << "  Step | Value        | Expected     | OK\n";
    cout << "  " + string(45, '-') + "\n";

    for (int step = 1; step <= 35; step++) {
        try {
            ct_self = cc->EvalMult(ct_self, enc(2));
            expected_self = mod_pos(expected_self * 2, modulus);

            auto trivial_zero = enc(0);
            auto diff = cc->EvalSub(trivial_zero, ct_self);
            ct_self = cc->EvalAdd(ct_self, diff);
            ct_self = cc->EvalAdd(ct_self, enc(0));

            int64_t val = dec(ct_self);
            bool ok = (val == expected_self);

            cout << "  " << setw(3) << step << "  | " << setw(12) << val
                 << " | " << setw(12) << expected_self
                 << " | " << (ok ? "✓" : "✗") << "\n";

            if (!ok) {
                cout << "\n  >>> SELF-REFERENCE DIVERGENCE at step " << step << "\n";
                break;
            }

        } catch (const exception& e) {
            cout << "\n  >>> CRASH at step " << step << ": " << e.what() << "\n";
            break;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   SACRED GEOMETRY INVESTIGATION COMPLETE                  ║\n";
    cout <<   "  ║   φ-intervals + self-reference tested                    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return 0;
}
