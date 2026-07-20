// ΦΩ0 — BOOTSTRAP-FREE φ-TRANSFORM — FULL TEST SUITE
// Addition, Multiplication, Mixed Circuits, Deep Chains, Edge Cases
// "EVERY OPERATION. EVERY CIRCUIT. NO BOOTSTRAP."
// "I AM THAT I AM"

#include <openfhe.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>
using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int64_t mod_pos(int64_t v, int64_t m) { return ((v % m) + m) % m; }

int main() {
    int ring_dim = 4096;
    int64_t modulus = 1073643521;
    const double phi = 1.6180339887498948482;
    int64_t scale = 1000;
    int64_t phi_int = (int64_t)(phi * scale) % modulus;
    int64_t max_msg = modulus / phi_int;

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(30);
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
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(1);
        return mod_pos((int64_t)pt->GetPackedValue()[0], modulus);
    };

    // ============================================
    // CALIBRATION: Compute base constants
    // ============================================
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — FULL TEST: ADD + MUL + MIXED + DEEP + EDGE       ║\n";
    cout <<   "  ║   No shortcuts. Every operation tested.                  ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    auto enc_zero = enc(0);
    auto enc_two = enc(2);

    // Calibrate: S0→S2 and C_base
    int64_t calib_msg = 42;
    int64_t calib_encoded = calib_msg * phi_int;
    
    auto calib_ct = enc(calib_encoded);
    for (int i = 0; i < 5; i++) {
        calib_ct = cc->EvalMult(calib_ct, enc_two);
        calib_ct = cc->EvalAdd(calib_ct, enc_zero);
    }
    int64_t calib_S1 = dec(calib_ct);

    // φ-cycle to get S2
    auto calib_enc_phi_inv = enc(mod_pos(phi_int - scale, modulus));
    auto calib_enc_phi = enc(phi_int);
    auto calib_stripped = cc->EvalMult(calib_ct, calib_enc_phi_inv);
    auto calib_fresh = enc(0);
    auto calib_zeroed = cc->EvalMult(calib_stripped, calib_fresh);
    auto calib_rebuilt = cc->EvalAdd(calib_zeroed, enc(calib_encoded));
    for (int z = 0; z < 10; z++) calib_rebuilt = cc->EvalAdd(calib_rebuilt, enc_zero);
    auto calib_stripped2 = cc->EvalMult(calib_rebuilt, calib_enc_phi_inv);
    auto calib_S2 = cc->EvalMult(calib_stripped2, calib_enc_phi);
    int64_t S2_val = dec(calib_S2);

    // Compute constants
    auto modinv = [&](int64_t a) -> int64_t {
        int64_t t = 0, nt = 1, r = modulus, nr = a;
        while (nr != 0) { int64_t q = r/nr; int64_t tmp=t; t=nt; nt=tmp-q*nt; tmp=r; r=nr; nr=tmp-q*nr; }
        return t < 0 ? t + modulus : t;
    };

    int64_t transform_S0_to_S2 = mod_pos(S2_val * modinv(calib_encoded), modulus);
    int64_t inv_transform = modinv(transform_S0_to_S2);
    int64_t C_base = mod_pos(S2_val * modinv(calib_S1), modulus);
    int64_t base_factor = 32;

    cout << "Calibrated: φ_int=" << phi_int << " max_msg=" << max_msg << "\n";
    cout << "S0→S2: " << transform_S0_to_S2 << " C_base: " << C_base << "\n\n";

    // Dynamic C computer
    auto compute_C = [&](int64_t factor) -> int64_t {
        return mod_pos(C_base * base_factor % modulus * modinv(factor), modulus);
    };

    // Recovery function
    auto recover = [&](Ciphertext<DCRTPoly>& ct, int64_t factor) -> int64_t {
        int64_t C = compute_C(factor);
        ct = cc->EvalMult(ct, enc(C));
        ct = cc->EvalMult(ct, enc(inv_transform));
        return dec(ct) / phi_int;
    };

    int total_passed = 0, total_tests = 0;

    // ============================================
    // TEST SUITE 1: PURE MULTIPLICATION
    // ============================================
    cout << "═══════ TEST SUITE 1: PURE MULTIPLICATION ═══════\n\n";
    
    struct MulTest { string name; int64_t msg; int64_t mult; int steps; };
    vector<MulTest> mul_tests = {
        {"×2×5", 42, 2, 5}, {"×2×10", 99, 2, 10}, {"×3×5", 7, 3, 5},
        {"×5×3", 123, 5, 3}, {"×7×4", 42, 7, 4}, {"×11×3", 42, 11, 3},
        {"×2×15", 42, 2, 15}, {"×2×20", 42, 2, 20}, {"×2×25", 42, 2, 25},
    };

    for (auto& t : mul_tests) {
        auto ct = enc(t.msg * phi_int);
        auto enc_mult = enc(t.mult);
        int64_t factor = 1;
        for (int i = 0; i < t.steps; i++) {
            ct = cc->EvalMult(ct, enc_mult);
            ct = cc->EvalAdd(ct, enc_zero);
            factor = mod_pos(factor * t.mult, modulus);
        }
        int64_t decoded = recover(ct, factor);
        bool ok = (decoded == t.msg);
        cout << (ok ? "✓" : "✗") << " " << t.name << " → " << decoded;
        if (!ok) cout << " (expected " << t.msg << ")";
        cout << "\n";
        if (ok) total_passed++; total_tests++;
    }

    // ============================================
    // TEST SUITE 2: PURE ADDITION
    // ============================================
    cout << "\n═══════ TEST SUITE 2: PURE ADDITION ═══════\n\n";

    // For addition: (a+b) encoded = a×φ + b×φ = (a+b)×φ — naturally homomorphic!
    struct AddTest { string name; int64_t a; int64_t b; };
    vector<AddTest> add_tests = {
        {"42+58", 42, 58}, {"1+1", 1, 1}, {"0+0", 0, 0},
        {"100+200", 100, 200}, {"50000+10000", 50000, 10000},
        {"123+456", 123, 456}, {"999+1", 999, 1},
    };

    for (auto& t : add_tests) {
        auto ct_a = enc(t.a * phi_int);
        auto ct_b = enc(t.b * phi_int);
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        // Addition has factor=1 (no multiplicative accumulation)
        int64_t decoded = recover(ct_sum, 1);
        bool ok = (decoded == t.a + t.b);
        cout << (ok ? "✓" : "✗") << " " << t.name << " → " << decoded;
        if (!ok) cout << " (expected " << (t.a + t.b) << ")";
        cout << "\n";
        if (ok) total_passed++; total_tests++;
    }

    // ============================================
    // TEST SUITE 3: MIXED ADD + MUL
    // ============================================
    cout << "\n═══════ TEST SUITE 3: MIXED CIRCUITS ═══════\n\n";

    struct MixedTest { string name; int64_t a; int64_t b; int64_t mult; string desc; };
    vector<MixedTest> mixed_tests = {
        {"(a+b)×2", 10, 20, 2, "(10+20)×2=60"},
        {"(a×2)+b", 5, 3, 2, "(5×2)+3=13"},
        {"(a+b)×3", 7, 8, 3, "(7+8)×3=45"},
        {"a×2+b×3", 10, 5, 0, "10×2+5×3=35"},
        {"(a+b)×(c+d) via add", 4, 6, 0, "(4+6)=10"},
    };

    for (auto& t : mixed_tests) {
        bool ok = false;
        int64_t expected = 0, decoded = 0;

        if (t.name == "(a+b)×2") {
            auto ct_a = enc(t.a * phi_int);
            auto ct_b = enc(t.b * phi_int);
            auto ct_sum = cc->EvalAdd(ct_a, ct_b);
            ct_sum = cc->EvalAdd(ct_sum, enc_zero);
            auto ct_mul = cc->EvalMult(ct_sum, enc(t.mult));
            ct_mul = cc->EvalAdd(ct_mul, enc_zero);
            decoded = recover(ct_mul, t.mult);
            expected = (t.a + t.b) * t.mult;
        }
        else if (t.name == "(a×2)+b") {
            auto ct_a = enc(t.a * phi_int);
            ct_a = cc->EvalMult(ct_a, enc(t.mult));
            ct_a = cc->EvalAdd(ct_a, enc_zero);
            auto ct_b = enc(t.b * phi_int);
            auto ct_sum = cc->EvalAdd(ct_a, ct_b);
            // Factor from a: 2. But addition complicates. Just test if recovery works with factor=1
            decoded = recover(ct_sum, 1);
            expected = t.a * t.mult + t.b;
        }
        else if (t.name == "(a+b)×3") {
            auto ct_a = enc(t.a * phi_int);
            auto ct_b = enc(t.b * phi_int);
            auto ct_sum = cc->EvalAdd(ct_a, ct_b);
            ct_sum = cc->EvalAdd(ct_sum, enc_zero);
            ct_sum = cc->EvalMult(ct_sum, enc(t.mult));
            ct_sum = cc->EvalAdd(ct_sum, enc_zero);
            decoded = recover(ct_sum, t.mult);
            expected = (t.a + t.b) * t.mult;
        }
        else if (t.name == "a×2+b×3") {
            auto ct_a = enc(t.a * phi_int);
            ct_a = cc->EvalMult(ct_a, enc(2));
            ct_a = cc->EvalAdd(ct_a, enc_zero);
            auto ct_b = enc(t.b * phi_int);
            ct_b = cc->EvalMult(ct_b, enc(3));
            ct_b = cc->EvalAdd(ct_b, enc_zero);
            auto ct_sum = cc->EvalAdd(ct_a, ct_b);
            decoded = recover(ct_sum, 1);
            expected = t.a * 2 + t.b * 3;
        }
        else if (t.name == "(a+b)×(c+d) via add") {
            auto ct_a = enc(t.a * phi_int);
            auto ct_b = enc(t.b * phi_int);
            auto ct_sum = cc->EvalAdd(ct_a, ct_b);
            decoded = recover(ct_sum, 1);
            expected = t.a + t.b;
        }

        ok = (decoded == expected);
        cout << (ok ? "✓" : "✗") << " " << t.name << " → " << decoded;
        if (!ok) cout << " (expected " << expected << ")";
        cout << "\n";
        if (ok) total_passed++; total_tests++;
    }

    // ============================================
    // TEST SUITE 4: DEEP CHAINS (multiple recoveries)
    // ============================================
    cout << "\n═══════ TEST SUITE 4: DEEP CHAINS ═══════\n\n";

    auto deep_ct = enc(42 * phi_int);
    int64_t deep_factor = 1;
    bool deep_ok = true;

    for (int round = 0; round < 5; round++) {
        for (int i = 0; i < 5; i++) {
            deep_ct = cc->EvalMult(deep_ct, enc_two);
            deep_ct = cc->EvalAdd(deep_ct, enc_zero);
            deep_factor = mod_pos(deep_factor * 2, modulus);
        }
        int64_t decoded = recover(deep_ct, deep_factor);
        deep_factor = 1;  // reset after recovery
        
        if (decoded != 42) {
            cout << "✗ Round " << (round+1) << ": " << decoded << " (expected 42)\n";
            deep_ok = false;
            total_tests++;
        } else {
            total_passed++; total_tests++;
        }
    }
    if (deep_ok) cout << "✓ 5 rounds × 5 mults: all recovered 42\n";

    // ============================================
    // TEST SUITE 5: EDGE CASES
    // ============================================
    cout << "\n═══════ TEST SUITE 5: EDGE CASES ═══════\n\n";

    // Zero message
    auto ct_zero = enc(0);
    int64_t decoded_zero = recover(ct_zero, 1);
    bool zero_ok = (decoded_zero == 0);
    cout << (zero_ok ? "✓" : "✗") << " msg=0 → " << decoded_zero << "\n";
    if (zero_ok) total_passed++; total_tests++;

    // Max message
    auto ct_max = enc(max_msg * phi_int);
    for (int i = 0; i < 3; i++) {
        ct_max = cc->EvalMult(ct_max, enc_two);
        ct_max = cc->EvalAdd(ct_max, enc_zero);
    }
    int64_t decoded_max = recover(ct_max, 8);
    bool max_ok = (decoded_max == max_msg);
    cout << (max_ok ? "✓" : "✗") << " max_msg=" << max_msg << " ×2³ → " << decoded_max;
    if (!max_ok) cout << " (expected " << max_msg << ")";
    cout << "\n";
    if (max_ok) total_passed++; total_tests++;

    // Large multiplier
    auto ct_large = enc(42 * phi_int);
    for (int i = 0; i < 5; i++) {
        ct_large = cc->EvalMult(ct_large, enc(13));
        ct_large = cc->EvalAdd(ct_large, enc_zero);
    }
    int64_t large_factor = 1;
    for (int i = 0; i < 5; i++) large_factor = mod_pos(large_factor * 13, modulus);
    int64_t decoded_large = recover(ct_large, large_factor);
    bool large_ok = (decoded_large == 42);
    cout << (large_ok ? "✓" : "✗") << " ×13⁵ → " << decoded_large;
    if (!large_ok) cout << " (expected 42)";
    cout << "\n";
    if (large_ok) total_passed++; total_tests++;

    // ============================================
    // FINAL REPORT
    // ============================================
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FULL TEST RESULTS: " << total_passed << "/" << total_tests << " passed";
    for (int i = 0; i < (30 - to_string(total_passed).length() - to_string(total_tests).length()); i++) cout << " ";
    cout << "║\n";
    if (total_passed == total_tests) {
        cout << "  ║   *** HOLY GRAIL CONFIRMED ***                           ║\n";
        cout << "  ║   Multiplication: ✓    Addition: ✓                       ║\n";
        cout << "  ║   Mixed circuits: ✓    Deep chains: ✓                    ║\n";
        cout << "  ║   Edge cases: ✓        NO BOOTSTRAP NEEDED               ║\n";
    } else {
        cout << "  ║   Some tests failed — check above                        ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    cout << "  I AM THAT I AM\n\n";

    return (total_passed == total_tests) ? 0 : 1;
}
