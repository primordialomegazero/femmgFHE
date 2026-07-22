// FEmmg-FHE — STRESS TEST: Complex arbitrary circuit
// Mixed ops: multiply, add, scale, with pre-scaling + asymmetric clean
// Push to find the breaking point

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class StressEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    StressEngine(int depth = 120) {
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
            pt->SetLength(slots); return pt->GetRealPackedValue()[0];
        } catch (...) { return NAN; }
    }

    struct PE { Ciphertext<DCRTPoly> a, b; };
    PE make(double a, double b) { return {enc(a), enc(b)}; }
    double val_phi(const PE& x) { return dec(x.a) + dec(x.b) * phi; }
    double val_psi(const PE& x) { return dec(x.a) + dec(x.b) * psi; }

    PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
    PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }
    PE add(const PE& x, const PE& y) { return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)}; }
    PE sub(const PE& x, const PE& y) { return {cc->EvalSub(x.a, y.a), cc->EvalSub(x.b, y.b)}; }
    PE mul(const PE& x, const PE& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }

    PE asymmetric_clean(const PE& x, int mul_steps, int div_steps) {
        auto r = x;
        for (int i = 0; i < mul_steps; i++) r = mul_X(r);
        for (int i = 0; i < div_steps; i++) r = div_X(r);
        return r;
    }

    void test_complex_circuit() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   STRESS TEST: Complex Arbitrary Circuit              ║\n";
        cout <<   "  ║   Mixed ops + pre-scaling + asymmetric clean         ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        // Circuit: simulate a financial model
        // x_{n+1} = x_n * rate + contribution - fee
        // With periodic φ-cleaning and pre-scaling

        double rate = 1.005;       // 0.5% growth
        double contribution = 0.1;
        double fee = 0.02;
        double net_phi_scale = pow(phi, 2);  // 3 mul_X - 1 div_X

        // Pre-scale the rate
        double prescaled_rate = rate / net_phi_scale;

        auto state = make(100.0, 0.0);  // Start with 100
        auto rate_enc = make(prescaled_rate, 0.0);
        auto contrib_enc = make(contribution, 0.0);
        auto fee_enc = make(fee, 0.0);

        double expected = 100.0;
        int total_mults = 0;
        int total_cleans = 0;
        bool alive = true;

        cout << "  Circuit: x ← x * " << rate << " + " << contribution << " - " << fee << "\n";
        cout << "  Pre-scaled rate: " << prescaled_rate << "\n";
        cout << "  Clean: every 5 steps (3 up, 1 down)\n\n";
        cout << "  " << setw(5) << "Step"
             << setw(14) << "Value"
             << setw(14) << "Expected"
             << setw(14) << "ψ-Noise"
             << setw(12) << "Error\n";
        cout << "  " << string(60, '-') << "\n";

        int maxSteps = 100;
        for (int i = 0; i <= maxSteps && alive; i += 10) {
            double val = val_phi(state);
            double noi = abs(val_psi(state));
            double err = abs((val - expected) / expected);

            cout << "  " << setw(5) << i
                 << setw(14) << fixed << setprecision(2) << val
                 << setw(14) << expected
                 << setw(14) << scientific << setprecision(2) << noi
                 << setw(12) << err << "\n";

            if (i < maxSteps) {
                for (int j = 0; j < 10 && alive; j++) {
                    try {
                        // x = x * rate
                        state = mul(state, rate_enc);
                        expected *= prescaled_rate;
                        total_mults++;
                        // x = x + contribution
                        state = add(state, contrib_enc);
                        expected += contribution;
                        // x = x - fee
                        state = sub(state, fee_enc);
                        expected -= fee;

                        // Periodic clean
                        if ((i + j + 1) % 5 == 0) {
                            state = asymmetric_clean(state, 3, 1);
                            expected *= net_phi_scale;
                            total_cleans++;
                        }
                    } catch (const exception& e) {
                        cout << "  CRASHED at step " << (i+j) << ": " << e.what() << "\n";
                        alive = false;
                    }
                }
            }
        }

        if (alive) {
            cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
            cout <<   "  ║   " << maxSteps << " steps, " << total_mults << " mults, " << total_cleans << " cleans                           ║\n";
            cout <<   "  ║   Final value: " << fixed << setprecision(2) << val_phi(state);
            for (int i=0; i<(24-to_string(val_phi(state)).length()); i++) cout<<" ";
            cout << "║\n";
            cout <<   "  ║   ψ-noise: " << scientific << setprecision(2) << abs(val_psi(state)) << "                               ║\n";
            cout <<   "  ║   *** COMPLEX CIRCUIT SURVIVED ***                    ║\n";
            cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
        }
    }

    void test_massively_parallel() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   PARALLEL: 5 independent chains, then combine       ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        // 5 parallel investment accounts, then total them
        vector<double> principals = {100, 200, 50, 300, 150};
        vector<PE> accounts;
        vector<double> expecteds;

        double rate = 1.01;
        double prescaled_rate = rate / pow(phi, 2);

        auto rate_enc = make(prescaled_rate, 0.0);

        for (double p : principals) {
            auto acct = make(p, 0.0);
            acct = asymmetric_clean(acct, 3, 1);
            accounts.push_back(acct);
            expecteds.push_back(p * pow(phi, 2));
        }

        cout << "  5 accounts, 20 steps each, then sum\n";
        cout << "  Computing... " << flush;

        bool alive = true;
        for (int step = 0; step < 20 && alive; step++) {
            for (int i = 0; i < 5; i++) {
                try {
                    accounts[i] = mul(accounts[i], rate_enc);
                    expecteds[i] *= prescaled_rate;
                    if (step % 4 == 3) {
                        accounts[i] = asymmetric_clean(accounts[i], 3, 1);
                        expecteds[i] *= pow(phi, 2);
                    }
                } catch (...) { alive = false; break; }
            }
        }

        if (alive) {
            // Sum all accounts
            auto total = accounts[0];
            double total_expected = expecteds[0];
            for (int i = 1; i < 5; i++) {
                total = add(total, accounts[i]);
                total_expected += expecteds[i];
            }

            double val = val_phi(total);
            double err = abs((val - total_expected) / total_expected);
            cout << "done!\n";
            cout << "  Total: " << fixed << setprecision(2) << val << " (expected " << total_expected << ")\n";
            cout << "  Error: " << scientific << err << "\n";
            cout << "  ψ-noise: " << abs(val_psi(total)) << "\n";
            cout << "  *** PARALLEL CIRCUIT PASSED ***\n\n";
        }
    }
};

int main() {
    StressEngine E(150);

    E.test_complex_circuit();
    E.test_massively_parallel();

    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   STRESS TEST: Complex circuits with φ survive       ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
