// FEmmg-FHE — PURE: X-multiplication as natural purification
// No separate trap. The computation purifies itself.
// Combined with Fibonacci compression for unlimited depth.

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <map>

using namespace lbcrypto;
using namespace std;

class PureEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    PureEngine(int depth = 80) {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(4096);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(depth);
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
        try {
            Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
            pt->SetLength(slots);
            return pt->GetRealPackedValue()[0];
        } catch (...) { return NAN; }
    }

    struct PE { Ciphertext<DCRTPoly> a, b; };
    PE make(double a, double b) { return {enc(a), enc(b)}; }
    double val_phi(const PE& x) { return dec(x.a) + dec(x.b) * phi; }
    double val_psi(const PE& x) { return dec(x.a) + dec(x.b) * psi; }

    // X-multiplication: THE fundamental operation. FREE. Purifies naturally.
    PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
    PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }
    PE add(const PE& x, const PE& y) { return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)}; }
    
    PE mul(const PE& x, const PE& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }

    // Build Fibonacci powers with natural X-purification built in
    map<int, PE> build_fib_powers(const PE& base, int max_fib) {
        map<int, PE> powers;
        powers[1] = base;
        powers[2] = mul(base, base);
        
        int a = 1, b = 2;
        while (b < max_fib) {
            int c = a + b;
            powers[c] = mul(powers[b], powers[a]);
            // Natural purification: one X-step per Fibonacci power
            powers[c] = mul_X(powers[c]);
            a = b; b = c;
        }
        return powers;
    }

    // Multiply x by base^n using Fibonacci decomposition + natural purification
    PE pow_fib_pure(const PE& x, const map<int, PE>& fib_powers, int n, int& x_steps) {
        vector<int> fib_keys;
        for (auto& p : fib_powers) fib_keys.push_back(p.first);
        sort(fib_keys.rbegin(), fib_keys.rend());
        
        auto result = x;
        int remaining = n;
        
        for (int f : fib_keys) {
            if (f <= remaining) {
                result = mul(result, fib_powers.at(f));
                remaining -= f;
                // Natural purification after each combination
                result = mul_X(result);
                x_steps++;
            }
        }
        
        while (remaining > 0) {
            result = mul(result, fib_powers.at(1));
            remaining--;
            result = mul_X(result);
            x_steps++;
        }
        
        return result;
    }

    void test_pure_purification() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   PURE: X-multiplication IS the purification         ║\n";
        cout <<   "  ║   No trap. Just let ψ do its work.                   ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        auto base = make(1.01, 0.0);
        cout << "  Building Fibonacci powers with natural purification...\n";
        auto fib_powers = build_fib_powers(base, 500);
        cout << "  " << fib_powers.size() << " powers built\n\n";

        auto state = make(1.0, 0.0);
        double expected = 1.0;
        int total_x_steps = 0;

        cout << "  " << setw(5) << "EffMults"
             << setw(6) << "XSteps"
             << setw(12) << "Value"
             << setw(14) << "ψ-Noise"
             << setw(12) << "Error\n";
        cout << "  " << string(55, '-') << "\n";

        vector<int> jumps = {10, 20, 50, 100, 200, 500};
        int total_mults = 0;

        for (int jump : jumps) {
            int x_steps = 0;
            state = pow_fib_pure(state, fib_powers, jump - total_mults, x_steps);
            total_mults = jump;
            total_x_steps += x_steps;

            // Track expected: multiply by base^jump, then φ per X-step
            expected = pow(1.01, total_mults) * pow(phi, total_x_steps);

            double val = val_phi(state);
            double noi = abs(val_psi(state));
            double err = abs((val - expected) / expected);

            cout << "  " << setw(5) << total_mults
                 << setw(6) << total_x_steps
                 << setw(12) << fixed << setprecision(2) << val
                 << setw(14) << scientific << setprecision(2) << noi
                 << setw(12) << err << "\n";
        }

        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   " << total_mults << " effective mults, " << total_x_steps << " X-steps (all FREE)              ║\n";
        cout <<   "  ║   ψ-noise: " << scientific << setprecision(2) << abs(val_psi(state)) << " (natural purification)           ║\n";
        cout <<   "  ║   Computation purifies itself.                        ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }

    void test_self_cleaning_chain() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   SELF-CLEANING CHAIN                                 ║\n";
        cout <<   "  ║   Every multiply followed by X-step (free)            ║\n";
        cout <<   "  ║   ψ-noise naturally decays as computation proceeds    ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        auto state = make(1.0, 0.0);
        auto multiplier = make(1.005, 0.0);
        double expected = 1.0;
        int x_steps = 0;

        cout << "  " << setw(5) << "Step" 
             << setw(8) << "XSteps"
             << setw(14) << "φ-Value"
             << setw(14) << "ψ-Noise"
             << setw(12) << "Error\n";
        cout << "  " << string(55, '-') << "\n";

        int maxSteps = 100;
        for (int i = 0; i <= maxSteps; i += 10) {
            double val = val_phi(state);
            double noi = abs(val_psi(state));
            double exp_val = expected * pow(phi, x_steps);
            double err = abs((val - exp_val) / exp_val);

            cout << "  " << setw(5) << i
                 << setw(8) << x_steps
                 << setw(14) << fixed << setprecision(4) << val
                 << setw(14) << scientific << setprecision(2) << noi
                 << setw(12) << err << "\n";

            if (i < maxSteps) {
                for (int j = 0; j < 10; j++) {
                    state = mul(state, multiplier);
                    expected *= 1.005;
                    state = mul_X(state);  // Natural purification
                    x_steps++;
                }
            }
        }

        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   " << maxSteps << " mults, " << x_steps << " X-steps (FREE)                         ║\n";
        cout <<   "  ║   ψ-noise: " << scientific << setprecision(2) << abs(val_psi(state)) << "                               ║\n";
        cout <<   "  ║   The computation cleans itself.                      ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }
};

int main() {
    PureEngine E(80);

    E.test_pure_purification();
    E.test_self_cleaning_chain();

    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   PURE: No trap. No fight. Just φ.                   ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
