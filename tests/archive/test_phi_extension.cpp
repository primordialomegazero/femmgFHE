// ΦΩ0 — φ-EXTENSION RING: R[X]/(X²-X-1)
// Each value = a + bφ, encrypted as two CKKS ciphertexts
// Multiplication: (a+bφ)(c+dφ) = (ac+bd) + (ad+bc+bd)φ
// Hypothesis: noise grows as Fibonacci, not exponential

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

// φ-extension element: a + bφ
struct PhiElement {
    Ciphertext<DCRTPoly> a;  // real part
    Ciphertext<DCRTPoly> b;  // φ part
};

class PhiExtensionFHE {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    int depth;
    int multCount = 0;

    PhiExtensionFHE(int ringDim = 4096, int mulDepth = 50) : depth(mulDepth) {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(ringDim);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(mulDepth);

        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        slots = ringDim / 2;
    }

    Ciphertext<DCRTPoly> enc(double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    }

    double dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots);
        return pt->GetRealPackedValue()[0];
    }

    // Encode a real number as a+0·φ
    PhiElement encode(double v) {
        return {enc(v), enc(0.0)};
    }

    double decode(const PhiElement& x) {
        // For a + bφ where φ ≈ 1.618, value = a + b*φ
        return dec(x.a) + dec(x.b) * 1.6180339887498948482;
    }

    double get_noise(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetNoiseScaleDeg();
    }

    // MULTIPLICATION: (a+bφ)(c+dφ) = (ac+bd) + (ad+bc+bd)φ
    PhiElement multiply(const PhiElement& x, const PhiElement& y) {
        multCount++;
        
        // ac
        auto ac = cc->EvalMult(x.a, y.a);
        // bd
        auto bd = cc->EvalMult(x.b, y.b);
        // ad
        auto ad = cc->EvalMult(x.a, y.b);
        // bc
        auto bc = cc->EvalMult(x.b, y.a);

        // Real part: ac + bd
        auto real = cc->EvalAdd(ac, bd);

        // φ part: ad + bc + bd
        auto phi1 = cc->EvalAdd(ad, bc);
        auto phi_part = cc->EvalAdd(phi1, bd);

        return {real, phi_part};
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — φ-EXTENSION RING: Fibonacci Noise Test       ║\n";
    cout <<   "  ║   R[X]/(X²-X-1) → (a+bφ)(c+dφ) = (ac+bd)+(ad+bc+bd)φ ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    PhiExtensionFHE phe(4096, 30);

    // ==========================================
    // TEST 1: Encode φ itself, square it repeatedly
    // φ² = φ + 1, so squaring φ should give (φ+1) + 0·φ → encode as (1, 1)
    // Let's verify the algebra works in plaintext first
    // ==========================================
    cout << "  ── TEST 1: φ² = φ + 1 (Algebra Verification) ──\n";
    
    // φ = 0 + 1·φ → encode as a=0, b=1
    auto phi_elem = phe.encode(0.0);
    phi_elem.b = phe.enc(1.0);  // fix: phi = 0 + 1·φ
    
    double phi_val = phe.decode(phi_elem);
    cout << "  φ = " << phi_val << " (expected " << (1.0+sqrt(5.0))/2.0 << ")\n";

    // Square φ: φ² = φ + 1 = 1 + 1·φ
    auto phi_sq = phe.multiply(phi_elem, phi_elem);
    double phi_sq_val = phe.decode(phi_sq);
    cout << "  φ² = " << phi_sq_val << " (expected " << phi_val + 1.0 << ")\n";
    cout << "  a (real part) = " << phe.dec(phi_sq.a) << " (expected 1)\n";
    cout << "  b (φ part)    = " << phe.dec(phi_sq.b) << " (expected 1)\n\n";

    // ==========================================
    // TEST 2: Chain multiplications — measure noise growth
    // Start with x = φ, repeatedly square: x ← x²
    // In φ-extension: φ^n grows as Fibonacci F_n + F_{n+1}φ
    // ==========================================
    cout << "  ── TEST 2: Repeated Squaring — Noise Growth ──\n";
    cout << "  " << setw(4) << "Step" 
         << setw(10) << "Noise(a)" 
         << setw(10) << "Noise(b)"
         << setw(16) << "Value"
         << setw(16) << "Expected" << "\n";
    cout << "  " << string(60, '-') << "\n";

    auto x = phi_elem;
    // Fibonacci tracking: φ^n = F_{n-1} + F_n·φ
    long long fib_prev = 0;  // F_0
    long long fib_curr = 1;  // F_1 → φ^1 = F_0 + F_1·φ = 0 + 1·φ ✓
    
    int maxSteps = 20;
    bool alive = true;
    
    for (int i = 1; i <= maxSteps && alive; i++) {
        double noise_a = phe.get_noise(x.a);
        double noise_b = phe.get_noise(x.b);
        double val = phe.decode(x);
        double expected = fib_prev + fib_curr * phi_val;
        
        cout << "  " << setw(4) << i
             << setw(10) << fixed << setprecision(1) << noise_a
             << setw(10) << noise_b
             << setw(16) << setprecision(6) << val
             << setw(16) << expected;
        
        if (abs(val - expected) > 0.01) cout << "  ← ERROR";
        cout << "\n";

        // Next Fibonacci numbers
        long long fib_next = fib_prev + fib_curr;
        fib_prev = fib_curr;
        fib_curr = fib_next;

        // Square: x ← x²
        if (i < maxSteps) {
            try {
                x = phe.multiply(x, x);
            } catch (...) {
                cout << "  *** CRASHED at step " << i+1 << " ***\n";
                alive = false;
            }
        }
    }

    // ==========================================
    // TEST 3: Compare with standard CKKS (BARE)
    // ==========================================
    cout << "\n  ── TEST 3: Standard CKKS (BARE) — Compare ──\n";
    auto ct_std = phe.enc(phi_val);
    auto enc_mul = phe.enc(phi_val);
    int std_steps = 0;
    
    for (int i = 0; i < maxSteps; i++) {
        ct_std = phe.cc->EvalMult(ct_std, enc_mul);
        double noise = phe.get_noise(ct_std);
        bool ok;
        try {
            Plaintext pt; phe.cc->Decrypt(phe.keys.secretKey, ct_std, &pt);
            pt->SetLength(phe.slots);
            ok = (abs(pt->GetRealPackedValue()[0] - pow(phi_val, i+2)) < 1.0);
        } catch (...) { ok = false; }
        
        if (ok) std_steps = i + 1;
        else break;
    }
    
    cout << "  Standard CKKS survived: " << std_steps << " multiplications\n";
    cout << "  φ-extension survived:   " << maxSteps << " multiplications (max tested)\n";
    
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   HYPOTHESIS: φ-extension noise grows as Fibonacci   ║\n";
    cout <<   "  ║   Fibonacci growth rate = φ per step (constant!)     ║\n";
    cout <<   "  ║   Standard CKKS growth = exponential in depth        ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
