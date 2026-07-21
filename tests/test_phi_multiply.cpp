// ΦΩ0 — Can we multiply by arbitrary constants using only φ-process?
// If yes: arbitrary CT×CT without EvalMult

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

struct PhiElement { Ciphertext<DCRTPoly> a, b; };

class PhiEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double phi_inv = 0.6180339887498948482;  // φ - 1 = 1/φ

    PhiEngine() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(4096);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(30);
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        slots = 2048;
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

    PhiElement make(double a, double b) { return {enc(a), enc(b)}; }
    double val(const PhiElement& x) { return dec(x.a) + dec(x.b) * phi; }

    // φ-multiply: (a+bφ)φ = b + (a+b)φ — FREE
    PhiElement mul_phi(const PhiElement& x) {
        return {x.b, cc->EvalAdd(x.a, x.b)};
    }

    // φ-divide: (a+bφ)/φ = (b-a) + aφ — FREE
    PhiElement div_phi(const PhiElement& x) {
        auto new_a = cc->EvalSub(x.b, x.a);  // b - a
        return {new_a, x.a};                 // a
    }

    // Add two φ-elements — FREE (just add components)
    PhiElement add(const PhiElement& x, const PhiElement& y) {
        return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)};
    }

    // Subtract — FREE
    PhiElement sub(const PhiElement& x, const PhiElement& y) {
        return {cc->EvalSub(x.a, y.a), cc->EvalSub(x.b, y.b)};
    }

    // Multiply by 2: x + x — FREE (just add to self)
    PhiElement mul2(const PhiElement& x) { return add(x, x); }

    // Multiply by scalar using φ-decomposition
    // Any number k can be decomposed into sum of φ-powers (Fibonacci)
    // Then k·x = Σ(φ^i · x) for each bit
    PhiElement mul_scalar_phi(const PhiElement& x, int k) {
        if (k == 0) return make(0.0, 0.0);
        if (k == 1) return x;
        if (k == -1) {
            // -1 = 1 - 2... messy. Let's handle small cases
            return make(-dec(x.a), -dec(x.b));  // fallback: use encryption
        }

        // Decompose k into Fibonacci numbers (Zeckendorf)
        vector<int> fib = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987};
        vector<int> parts;
        int remaining = k;
        for (int i = fib.size() - 1; i >= 0 && remaining > 0; i--) {
            if (fib[i] <= remaining) {
                parts.push_back(fib[i]);
                remaining -= fib[i];
                i--;  // skip next (non-consecutive)
            }
        }

        // Now compute: k·x = Σ φ^n · x / φ^n? No...
        // Actually: F_n ≈ φ^n / √5
        // So F_n · x ≈ (φ^n / √5) · x
        // This means multiplying by F_n requires φ^n and a division by √5
        
        // For now, let's just test with direct EvalMult as fallback
        // and compare against φ-based approach
        auto result = make(0.0, 0.0);
        auto shifted = x;
        int power = 0;
        
        for (int f : parts) {
            // Find which Fibonacci index
            while (power < fib.size() && fib[power] < f) {
                shifted = mul_phi(shifted);  // multiply by φ
                power++;
            }
            result = add(result, shifted);
        }
        return result;
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — φ-MULTIPLICATION: Can we avoid EvalMult?     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    PhiEngine E;

    // Test basic operations
    auto x = E.make(3.0, 0.0);
    
    cout << "  x = 3: " << E.val(x) << "\n";
    cout << "  x·φ = " << E.val(E.mul_phi(x)) << " (expected " << 3.0 * E.phi << ")\n";
    cout << "  x/φ = " << E.val(E.div_phi(x)) << " (expected " << 3.0 / E.phi << ")\n";
    cout << "  2x = " << E.val(E.mul2(x)) << " (expected 6)\n\n";

    // Test: multiply by Fibonacci numbers using φ-shifts
    // F_n ≈ φ^n / √5, so F_n · x = (φ^n · x) / √5
    // But we can test directly: can we multiply by 2,3,5,8... using φ-operations?
    
    cout << "  ── Fibonacci Multiplication Test ──\n";
    cout << "  Multiplying x=3 by Fibonacci numbers using φ-shifts:\n\n";
    
    auto current = x;
    int fib_prev = 1, fib_curr = 1;
    
    for (int i = 0; i < 10; i++) {
        double result = E.val(current);
        double expected = 3.0 * fib_curr;
        cout << "  F_" << (i+1) << "=" << fib_curr << " × 3 = " << result;
        if (abs(result - expected) > 0.1) cout << " (expected " << expected << ")";
        cout << "\n";
        
        // Next: multiply by φ
        current = E.mul_phi(current);
        int fib_next = fib_prev + fib_curr;
        fib_prev = fib_curr;
        fib_curr = fib_next;
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   OBSERVATION:                                        ║\n";
    cout <<   "  ║   Multiplying by φ^n gives F_n scaling, not exact F_n ║\n";
    cout <<   "  ║   We need to divide by √5 to get exact Fibonacci      ║\n";
    cout <<   "  ║   But! We can represent ANY number in base φ          ║\n";
    cout <<   "  ║   The φ-ary representation uses digits 0 and 1        ║\n";
    cout <<   "  ║   Every real number = Σ d_i · φ^i (d_i ∈ {0,1})      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
