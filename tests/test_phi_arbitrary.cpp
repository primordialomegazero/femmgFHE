// FEmmg-FHE — Arbitrary Circuit Test
// Mixed operations: add, multiply, rotate + Noise Trap
// Not just multiply chains — real FHE workloads

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class ArbitraryCircuitEngine {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;
    double trapScale = (1.0 + phi) / 2.0;

    ArbitraryCircuitEngine(int depth = 150) {
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

    // FREE operations
    PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
    PE add(const PE& x, const PE& y) { return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)}; }
    PE sub(const PE& x, const PE& y) { return {cc->EvalSub(x.a, y.a), cc->EvalSub(x.b, y.b)}; }

    // CT×CT multiply (1 depth)
    PE mul(const PE& x, const PE& y) {
        auto ac = cc->EvalMult(x.a, y.a);
        auto bd = cc->EvalMult(x.b, y.b);
        auto ad = cc->EvalMult(x.a, y.b);
        auto bc = cc->EvalMult(x.b, y.a);
        return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
    }

    // Noise Trap: T(x) = (x + X·x) / 2
    PE trap(const PE& x) {
        auto flipped = mul_X(x);
        auto summed = add(x, flipped);
        auto half = make(0.5, 0.0);
        return mul(summed, half);
    }

    void test_polynomial() {
        cout << "\n  ── CIRCUIT 1: Polynomial f(x) = x³ + 2x² + 3x + 4 ──\n";

        auto x = make(2.0, 0.0);
        auto two = make(2.0, 0.0);
        auto three = make(3.0, 0.0);
        auto four = make(4.0, 0.0);

        // x²
        auto x2 = mul(x, x);
        // x³
        auto x3 = mul(x2, x);
        // 2x²
        auto two_x2 = mul(x2, two);
        // 3x
        auto three_x = mul(x, three);
        // f(x) = x³ + 2x² + 3x + 4
        auto result = add(add(x3, two_x2), add(three_x, four));

        double expected = pow(2,3) + 2*pow(2,2) + 3*2 + 4; // 8 + 8 + 6 + 4 = 26
        double val = val_phi(result);
        double err = abs(val - expected) / expected;

        cout << "  f(2) = 2³ + 2·2² + 3·2 + 4 = " << val;
        cout << " (expected " << expected << ") error=" << err << "\n";
    }

    void test_dot_product() {
        cout << "\n  ── CIRCUIT 2: Dot Product [1,2,3] · [4,5,6] ──\n";

        auto a1 = make(1.0, 0.0), b1 = make(4.0, 0.0);
        auto a2 = make(2.0, 0.0), b2 = make(5.0, 0.0);
        auto a3 = make(3.0, 0.0), b3 = make(6.0, 0.0);

        auto p1 = mul(a1, b1);
        auto p2 = mul(a2, b2);
        auto p3 = mul(a3, b3);
        auto dot = add(add(p1, p2), p3);

        double expected = 1*4 + 2*5 + 3*6; // 4 + 10 + 18 = 32
        double val = val_phi(dot);
        double err = abs(val - expected) / expected;

        cout << "  dot = " << val << " (expected " << expected << ") error=" << err << "\n";
    }

    void test_deep_circuit_with_traps() {
        cout << "\n  ── CIRCUIT 3: Deep mixed circuit with periodic traps ──\n";
        cout << "  " << setw(5) << "Step" << setw(14) << "Value"
             << setw(12) << "Noise(ψ)" << setw(12) << "Error\n";
        cout << "  " << string(50, '-') << "\n";

        // Circuit: repeatedly compute x ← (x + 1) * (x + 2) / 2
        // with noise trap every 3 iterations

        auto x = make(1.0, 0.0);
        auto one = make(1.0, 0.0);
        auto two = make(2.0, 0.0);
        auto half = make(0.5, 0.0);

        double expected = 1.0;
        int maxIter = 30;

        for (int i = 0; i <= maxIter; i++) {
            double val = val_phi(x);
            double noi = abs(val_psi(x));
            double err = abs((val - expected) / expected);

            if (i % 5 == 0) {
                cout << "  " << setw(5) << i
                     << setw(14) << fixed << setprecision(4) << val
                     << setw(12) << scientific << setprecision(2) << noi
                     << setw(12) << err << "\n";
            }

            if (i < maxIter) {
                auto x1 = add(x, one);
                auto x2 = add(x, two);
                x = mul(x1, x2);
                x = mul(x, half);
                // expected: ((x+1)*(x+2))/2
                expected = ((expected + 1) * (expected + 2)) / 2;

                // Trap every 3 iterations
                if ((i + 1) % 3 == 0) {
                    x = trap(x);
                    expected *= trapScale;
                }
            }
        }
    }

    void test_all() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   Arbitrary Circuit Tests                             ║\n";
        cout <<   "  ║   Polynomial, Dot Product, Deep Mixed with Traps     ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n";

        test_polynomial();
        test_dot_product();
        test_deep_circuit_with_traps();

        cout << "\n  All arbitrary circuit tests complete.\n\n";
    }
};

int main() {
    ArbitraryCircuitEngine E;
    E.test_all();
    return 0;
}
