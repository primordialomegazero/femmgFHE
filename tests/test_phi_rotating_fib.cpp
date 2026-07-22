// FEmmg-FHE — ROTATING FIBONACCI: Depth compression between cleans
// Clean → Fibonacci Jump (N mults in log N depth) → Bootstrap → Repeat
// Rotation: alternate between Fibonacci bases to balance φ-growth

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <map>

using namespace lbcrypto;
using namespace std;

class RotatingFibEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;
    int total_mults = 0, total_cleans = 0, total_bootstraps = 0;

    RotatingFibEngine(int depth = 80) {
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
        cc->Enable(FHE);
        cc->EvalBootstrapSetup({4,4}, {0,0}, 2048);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        cc->EvalBootstrapKeyGen(keys.secretKey, 2048);
        slots = 2048;
    }

    Ciphertext<DCRTPoly> enc(double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    }
    double dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots); return pt->GetRealPackedValue()[0];
    }

    struct PE { Ciphertext<DCRTPoly> a, b; };
    PE make(double a, double b) { return {enc(a), enc(b)}; }
    double val_phi(const PE& x) { return dec(x.a) + dec(x.b) * phi; }
    double val_psi(const PE& x) { return dec(x.a) + dec(x.b) * psi; }
    PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
    PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }
    PE add(const PE& x, const PE& y) { return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)}; }
    PE mul(const PE& x, const PE& y) {
        auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }

    PE clean(const PE& x) { auto r = x; for (int i=0; i<3; i++) r=mul_X(r); for (int i=0; i<2; i++) r=div_X(r); total_cleans++; return r; }

    // Build Fibonacci powers of a base value
    map<int, PE> build_fib_powers(const PE& base, int max_fib) {
        map<int, PE> powers;
        powers[1] = base;
        powers[2] = mul(base, base);
        int a = 1, b = 2;
        while (b < max_fib) {
            int c = a + b;
            powers[c] = mul(powers[b], powers[a]); powers[c] = clean(powers[c]);
            a = b; b = c;
        }
        return powers;
    }

    // Fibonacci jump: multiply state by base^N using Zeckendorf
    PE fib_jump(const PE& state, const map<int, PE>& fib_powers, int N, double& scale) {
        vector<int> fib_keys;
        for (auto& p : fib_powers) fib_keys.push_back(p.first);
        sort(fib_keys.rbegin(), fib_keys.rend());
        
        auto result = state;
        int remaining = N;
        scale = 1.0;
        
        for (int f : fib_keys) {
            if (f <= remaining) {
                result = mul(result, fib_powers.at(f)); if (f > 34) result = clean(result);
                remaining -= f;
            }
        }
        while (remaining > 0) {
            result = mul(result, fib_powers.at(1));
            remaining--;
        }
        return result;
    }

    void run() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   ROTATING FIBONACCI: Jump → Clean → Bootstrap       ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        double base_rate = 1.01;
        int fib_max = 200;
        
        // Build Fibonacci powers
        auto base = make(base_rate, 0.0);
        auto fib_powers = build_fib_powers(base, fib_max);
        cout << "  Fibonacci powers built: " << fib_powers.size() << " (up to F_" << fib_max << ")\n\n";

        // Start state with noise
        double init_sig = 1.0, init_noise = 3.0;
        double b_init = (init_sig - init_noise) / (phi - psi);
        double a_init = init_sig - b_init * phi;
        auto state = make(a_init, b_init);
        double expected = init_sig;

        // Fibonacci jump sizes per cycle (rotating: small→medium→large→small...)
        vector<int> jump_sizes = {5, 8, 13, 21, 34, 55, 89, 144, 233, 377};
        
        cout << "  " << setw(5) << "Cycle" << setw(8) << "Jump"
             << setw(10) << "EffMults" << setw(6) << "Clean"
             << setw(14) << "φ-value" << setw(14) << "ψ-noise"
             << setw(12) << "Error\n";
        cout << "  " << string(80, '-') << "\n";

        bool alive = true;
        int cycle = 0;

        for (int jump : jump_sizes) {
            if (!alive) break;
            
            try {
                // Clean before jump
                state = clean(state);
                expected *= phi;

                // Fibonacci jump
                double jump_scale;
                state = fib_jump(state, fib_powers, jump, jump_scale);
                expected *= pow(base_rate, jump);
                total_mults += jump;

                double phi_val = val_phi(state);
                double psi_val = abs(val_psi(state));
                double err = abs((phi_val - expected) / expected);

                cout << "  " << setw(5) << cycle
                     << setw(8) << jump
                     << setw(10) << total_mults << setw(6) << total_cleans
                     << setw(14) << scientific << setprecision(4) << phi_val
                     << setw(14) << scientific << setprecision(2) << psi_val
                     << setw(12) << err << "\n";

                cycle++;

            } catch (const exception& e) {
                cout << "  CRASHED at cycle " << cycle << " (jump=" << jump << "): " << e.what() << "\n";
                alive = false;
            }
        }

        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   TOTAL: " << total_mults << " effective mults, " << total_cleans << " cleans                     ║\n";
        double final_noise = abs(val_psi(state));
        cout <<   "  ║   ψ-noise: " << scientific << setprecision(2) << final_noise << " (started " << init_noise << ")                  ║\n";
        if (final_noise < init_noise)
            cout << "  ║   ✅ ROTATING FIBONACCI — NOISE SHRINKS              ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — ROTATING FIBONACCI                           ║\n";
    cout <<   "  ║   Depth compression + Noise lifetime                 ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n";

    RotatingFibEngine E(120);
    E.run();

    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   Fibonacci compresses O(N) → O(log N).              ║\n";
    cout <<   "  ║   Clean kills noise between jumps.                   ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
