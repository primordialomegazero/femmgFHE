// ΦΩ0 — PUSH TO LIMIT: How many fused multiply-traps can we do?
// x ⋆ y = (xy + φ(xy))/2
// 100 fused steps = 200 depth

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class MaxEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;

    MaxEngine(int depth) {
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
    double val_psi(const PE& x) { 
        double psi = -0.6180339887498948482; 
        return dec(x.a) + dec(x.b) * psi; 
    }
    PE mul_phi(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }

    PE std_mul(const PE& x, const PE& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }

    PE fused_mul_trap(const PE& x, const PE& y) {
        auto prod = std_mul(x, y);
        auto flipped = mul_phi(prod);
        auto summed = PE{cc->EvalAdd(prod.a, flipped.a), 
                         cc->EvalAdd(prod.b, flipped.b)};
        auto half = make(0.5, 0.0);
        return std_mul(summed, half);
    }

    bool test_steps(int numSteps, double multiplier_val) {
        auto state = make(1.0, 0.0);
        auto multiplier = make(multiplier_val, 0.0);
        double trapScale = (1.0 + phi) / 2.0;
        double expected = 1.0;
        bool alive = true;
        
        for (int i = 0; i < numSteps && alive; i++) {
            try {
                state = fused_mul_trap(state, multiplier);
                expected *= multiplier_val * trapScale;
            } catch (const exception& e) {
                cout << "    CRASHED at step " << (i+1) << ": " << e.what() << "\n";
                alive = false;
            }
        }
        
        if (alive) {
            double sig = val_phi(state);
            double noi = abs(val_psi(state));
            double err = abs((sig - expected) / expected);
            
            cout << "    Signal: " << scientific << setprecision(6) << sig << "\n";
            cout << "    Expected: " << expected << "\n";
            cout << "    Error: " << err << "\n";
            cout << "    Noise(ψ): " << noi << "\n";
            
            if (err < 0.01) {
                cout << "    *** " << numSteps << " FUSED STEPS — PASSED! ***\n";
                return true;
            } else {
                cout << "    High error\n";
                return false;
            }
        }
        return false;
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — PUSH TO LIMIT                                ║\n";
    cout <<   "  ║   How many fused multiply-traps survive?             ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    // Test 1: 100 steps, depth=250
    cout << "  ── TEST 1: 100 fused steps (depth=250) ──\n";
    MaxEngine E1(250);
    E1.test_steps(100, 1.01);

    // Test 2: 150 steps, depth=350
    cout << "\n  ── TEST 2: 150 fused steps (depth=350) ──\n";
    MaxEngine E2(350);
    E2.test_steps(150, 1.005);  // Smaller multiplier for stability

    // Test 3: 200 steps, depth=450
    cout << "\n  ── TEST 3: 200 fused steps (depth=450) ──\n";
    MaxEngine E3(450);
    E3.test_steps(200, 1.003);

    // Test 4: PUSH HARD — 300 steps, depth=650
    cout << "\n  ── TEST 4: 300 fused steps (depth=650) ──\n";
    MaxEngine E4(650);
    E4.test_steps(300, 1.002);

    // Test 5: FIND THE WALL — 500 steps
    cout << "\n  ── TEST 5: 500 fused steps (depth=1050) ──\n";
    MaxEngine E5(1050);
    E5.test_steps(500, 1.001);

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   FUSED MULTIPLY-TRAP: The computation cleans itself ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
