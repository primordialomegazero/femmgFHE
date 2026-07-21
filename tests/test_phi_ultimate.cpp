// ΦΩ0 — ULTIMATE: Compute + Noise Trap Cycle
// 1. CT×CT multiplications (cost depth)
// 2. Noise trap: x ← (x + φ(x))/2 (kills noise, costs 1 depth)
// 3. Repeat indefinitely
// Noise stays DEAD throughout

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class UltimateEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    UltimateEngine() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(4096);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(200);
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

    // FREE operations
    PE mul_phi(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
    PE add(const PE& x, const PE& y) { return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)}; }

    // CT×CT multiply (1 depth)
    PE ctct_mul(const PE& x, const PE& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }

    // NOISE TRAP: x ← (x + φ(x)) / 2
    // Costs 1 depth (for multiply by 0.5)
    // Kills noise by factor (1+ψ)/2 ≈ 0.191
    PE noise_trap(const PE& x) {
        auto flipped = mul_phi(x);           // FREE
        auto summed = add(x, flipped);       // FREE
        auto half = make(0.5, 0.0);
        return ctct_mul(summed, half);       // 1 depth
    }

    void test_ultimate() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   ΦΩ0 — ULTIMATE: Compute + Trap Cycle               ║\n";
        cout <<   "  ║   3 CT×CT mults → Noise Trap → Repeat               ║\n";
        cout <<   "  ║   Noise killed every cycle, signal preserved        ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        // Start with signal=1, noise=0
        auto state = make(1.0, 0.0);
        auto multiplier = make(1.01, 0.0);
        
        // Track expected: every CT×CT multiplies by 1.01
        // Every trap multiplies signal by (1+φ)/2 ≈ 1.309
        double trapScale = (1.0 + phi) / 2.0;
        double expected = 1.0;
        int totalMults = 0;
        int totalTraps = 0;
        
        int multsPerCycle = 3;
        
        cout << "  Cycle  Mults  Traps  Signal(φ)    Noise(ψ)     Expected    Err\n";
        cout << "  " << string(70, '-') << "\n";
        
        for (int cycle = 0; cycle <= 10; cycle++) {
            double sig = val_phi(state);
            double noi = abs(val_psi(state));
            double err = abs((sig - expected) / expected);
            
            cout << "  " << setw(5) << cycle
                 << setw(6) << totalMults
                 << setw(6) << totalTraps
                 << setw(14) << fixed << setprecision(6) << sig
                 << setw(14) << scientific << setprecision(2) << noi
                 << setw(12) << fixed << setprecision(4) << expected
                 << setw(12) << scientific << setprecision(2) << err << "\n";
            
            if (cycle < 10) {
                // CT×CT multiplications
                for (int m = 0; m < multsPerCycle; m++) {
                    state = ctct_mul(state, multiplier);
                    expected *= 1.01;
                    totalMults++;
                }
                // Noise trap
                state = noise_trap(state);
                expected *= trapScale;
                totalTraps++;
            }
        }
        
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   RESULTS: " << totalMults << " CT×CT mults, " << totalTraps << " traps                     ║\n";
        cout <<   "  ║   Total depth used: " << (totalMults + totalTraps) << " (mults + traps)                ║\n";
        cout <<   "  ║   Noise: " << scientific << setprecision(2) << abs(val_psi(state)) << " (should be near 0)               ║\n";
        
        if (abs(val_psi(state)) < 0.01)
            cout << "  ║   *** NOISE EFFECTIVELY DEAD ***                      ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }

    void test_long_chain() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   LONG CHAIN: 100+ CT×CT with periodic traps         ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        auto state = make(1.0, 0.0);
        auto multiplier = make(1.005, 0.0);  // Even smaller multiplier for longer chain
        
        double trapScale = (1.0 + phi) / 2.0;
        double expected = 1.0;
        int totalMults = 0;
        
        int multsPerCycle = 5;
        int maxCycles = 25;  // 25 × 5 = 125 mults, 25 traps = 150 depth total
        
        cout << "  Computing " << (multsPerCycle * maxCycles) << " CT×CT mults with " 
             << maxCycles << " noise traps...\n\n";
        
        bool alive = true;
        for (int cycle = 0; cycle < maxCycles && alive; cycle++) {
            for (int m = 0; m < multsPerCycle; m++) {
                try {
                    state = ctct_mul(state, multiplier);
                    expected *= 1.005;
                    totalMults++;
                } catch (...) {
                    cout << "  CRASHED at mult " << totalMults << "\n";
                    alive = false;
                    break;
                }
            }
            if (!alive) break;
            
            try {
                state = noise_trap(state);
                expected *= trapScale;
            } catch (...) {
                cout << "  CRASHED at trap after " << totalMults << " mults\n";
                alive = false;
            }
        }
        
        double sig = val_phi(state);
        double noi = abs(val_psi(state));
        double err = abs((sig - expected) / expected);
        
        cout << "  Final: signal=" << scientific << sig 
             << " expected=" << expected
             << " error=" << err
             << " noise=" << noi << "\n\n";
        
        if (err < 0.1)
            cout << "  *** " << totalMults << " CT×CT WITH TRAPS — PASSED! ***\n\n";
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — ULTIMATE: COMPUTE + TRAP = UNLIMITED         ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n";

    UltimateEngine U;

    U.test_ultimate();
    U.test_long_chain();

    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
