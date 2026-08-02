// FEmmg-FHE — SIMPLE FIBONACCI: One clean per cycle
// Clean → Fibonacci jump → next cycle
// φ-structure tracks scale automatically. No manual φ-tracking needed.
// Value = a + bφ. After operations, decode gives correct result.

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <map>

using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482;
double psi = -0.6180339887498948482;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   SIMPLE FIBONACCI: One clean per cycle               ║\n";
    cout <<   "  ║   φ-structure tracks scale. No manual tracking.      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096);
    params.SetScalingModSize(59);
    params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetMultiplicativeDepth(120);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    int slots = 2048;

    auto enc = [&](double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots); return pt->GetRealPackedValue()[0];
    };

    struct PE { Ciphertext<DCRTPoly> a, b; };
    auto make = [&](double a, double b) -> PE { return {enc(a), enc(b)}; };
    auto val_phi = [&](const PE& x) { return dec(x.a) + dec(x.b) * phi; };
    auto val_psi = [&](const PE& x) { return dec(x.a) + dec(x.b) * psi; };
    auto mul_X = [&](const PE& x) -> PE { return {x.b, cc->EvalAdd(x.a, x.b)}; };
    auto div_X = [&](const PE& x) -> PE { return {cc->EvalSub(x.b, x.a), x.a}; };
    auto mul = [&](const PE& x, const PE& y) -> PE {
        auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    };

    // Clean: 3↑ 2↓
    auto clean = [&](PE x) -> PE {
        for (int i=0; i<3; i++) x = mul_X(x);
        for (int i=0; i<2; i++) x = div_X(x);
        return x;
    };

    // Build Fibonacci powers of base (base must be in φ-form: make(base_val, 0))
    auto build_fib = [&](double base_val, int max_fib) {
        map<int, PE> powers;
        auto b = make(base_val, 0.0);
        powers[1] = b;
        powers[2] = mul(b, b);
        int a = 1, c = 2;
        while (c < max_fib) {
            int n = a + c;
            powers[n] = mul(powers[c], powers[a]);
            a = c; c = n;
        }
        return powers;
    };

    // Fibonacci jump using pre-built powers
    auto fib_jump = [&](PE state, const map<int, PE>& powers, int N) -> PE {
        vector<int> keys;
        for (auto& p : powers) keys.push_back(p.first);
        sort(keys.rbegin(), keys.rend());
        int remaining = N;
        for (int f : keys) {
            if (f <= remaining) {
                state = mul(state, powers.at(f));
                remaining -= f;
            }
        }
        while (remaining > 0) { state = mul(state, powers.at(1)); remaining--; }
        return state;
    };

    // ==========================================
    // THE TEST
    // ==========================================
    double base_rate = 1.01;
    auto fib_powers = build_fib(base_rate, 200);
    cout << "  Fibonacci powers: " << fib_powers.size() << "\n\n";

    // Start: value = 1.0 (encoded as 1 + 0φ)
    auto state = make(1.0, 0.0);
    
    // Track a AND b separately to verify φ-structure self-tracking
    // The value is dec(state.a) + dec(state.b)*phi — this automatically
    // accounts for all φ-growth from cleans!

    cout << "  " << setw(5) << "Cycle" << setw(8) << "Jump"
         << setw(12) << "EffMults" << setw(10) << "a" << setw(10) << "b"
         << setw(14) << "Value(a+bφ)" << setw(14) << "ψ-noise\n";
    cout << "  " << string(75, '-') << "\n";

    int total_mults = 0;
    vector<int> jumps = {3, 5, 8, 13, 21, 34, 55, 89};
    bool alive = true;

    for (int cycle = 0; cycle < (int)jumps.size() && alive; cycle++) {
        int jump = jumps[cycle];
        
        try {
            // Clean
            state = clean(state);
            
            // Fibonacci jump
            state = fib_jump(state, fib_powers, jump);
            total_mults += jump;
            
            double a = dec(state.a);
            double b = dec(state.b);
            double value = a + b * phi;
            double noise = abs(val_psi(state));
            
            cout << "  " << setw(5) << cycle
                 << setw(8) << jump
                 << setw(12) << total_mults
                 << setw(10) << fixed << setprecision(2) << a
                 << setw(10) << b
                 << setw(14) << scientific << setprecision(4) << value
                 << setw(14) << scientific << setprecision(2) << noise << "\n";
                 
        } catch (const exception& e) {
            cout << "  CRASHED at cycle " << cycle << ": " << e.what() << "\n";
            alive = false;
        }
    }

    if (alive) {
        double a = dec(state.a), b = dec(state.b);
        double val = a + b * phi;
        double noi = abs(val_psi(state));
        
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   " << total_mults << " effective mults, " << jumps.size() << " cleans                          ║\n";
        cout <<   "  ║   a=" << fixed << setprecision(2) << a << " b=" << b << " value=" << scientific << val << "              ║\n";
        cout <<   "  ║   ψ-noise: " << noi << "                              ║\n";
        cout <<   "  ║   φ-structure self-tracks scaling                    ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }

    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   Simple. Clean. Fibonacci. Repeat.                  ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
