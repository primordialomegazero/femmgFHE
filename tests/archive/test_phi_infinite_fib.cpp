// FEmmg-FHE — INFINITE FIBONACCI: Clean → Jump → Bootstrap → Repeat
// Bootstrap when levels low. φ-structure self-tracks. Unlimited.

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
    cout <<   "  ║   INFINITE FIBONACCI: Jump → Clean → Bootstrap       ║\n";
    cout <<   "  ║   Unlimited depth via φ-structure + level recovery   ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY);
    params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096);
    params.SetScalingModSize(59);
    params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO);
    params.SetMultiplicativeDepth(80);
    auto cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);
    cc->EvalBootstrapSetup({4,4}, {0,0}, 2048);
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey, 2048);
    int slots = 2048;

    auto enc = [&](double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    auto dec = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots); return pt->GetRealPackedValue()[0];
    };
    auto bootstrap_ct = [&](const Ciphertext<DCRTPoly>& ct) {
        return cc->EvalBootstrap(ct);
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
    auto clean = [&](PE x) -> PE {
        for (int i=0; i<3; i++) x = mul_X(x);
        for (int i=0; i<2; i++) x = div_X(x);
        return x;
    };
    auto bootstrap_pe = [&](PE x) -> PE {
        x.a = bootstrap_ct(x.a);
        x.b = bootstrap_ct(x.b);
        return x;
    };

    // Build Fibonacci powers
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
    // THE INFINITE TEST
    // ==========================================
    double base_rate = 1.01;
    auto fib_powers = build_fib(base_rate, 300);
    cout << "  Fibonacci powers: " << fib_powers.size() << "\n\n";

    auto state = make(1.0, 0.0);
    int total_mults = 0, total_cleans = 0, total_boots = 0;
    
    // Cycling pattern: small Fibonacci numbers that don't exhaust levels
    vector<int> pattern = {5, 8, 13, 21, 34};  // Rotating through these
    
    cout << "  " << setw(5) << "Round" << setw(8) << "Jump"
         << setw(10) << "TotalMult" << setw(6) << "Clean" << setw(6) << "Boot"
         << setw(10) << "a" << setw(10) << "b"
         << setw(12) << "ψ-noise\n";
    cout << "  " << string(80, '-') << "\n";

    bool alive = true;
    int round = 0;
    int jumps_since_bootstrap = 0;
    
    while (alive && round < 50) {
        int jump = pattern[round % pattern.size()];
        
        try {
            // Clean before jump
            state = clean(state);
            total_cleans++;
            
            // Fibonacci jump
            state = fib_jump(state, fib_powers, jump);
            total_mults += jump;
            jumps_since_bootstrap += jump;
            
            // Bootstrap every ~60 effective mults
            if (jumps_since_bootstrap >= 55) {
                state = bootstrap_pe(state);
                total_boots++;
                jumps_since_bootstrap = 0;
            }
            
            double a = dec(state.a), b = dec(state.b);
            double noise = abs(val_psi(state));
            
            cout << "  " << setw(5) << round
                 << setw(8) << jump
                 << setw(10) << total_mults << setw(6) << total_cleans << setw(6) << total_boots
                 << setw(10) << fixed << setprecision(2) << a
                 << setw(10) << b
                 << setw(12) << scientific << setprecision(2) << noise << "\n";
            
            round++;
            
        } catch (const exception& e) {
            cout << "  ║   CRASHED at round " << round << " (jump=" << jump << ", total=" << total_mults << ")\n";
            cout << "  ║   " << e.what() << "\n";
            alive = false;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   " << total_mults << " effective mults, " << total_cleans << " cleans, " << total_boots << " bootstraps              ║\n";
    if (alive) cout << "  ║   ✅ INFINITE FIBONACCI — SURVIVED " << round << " ROUNDS              ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
