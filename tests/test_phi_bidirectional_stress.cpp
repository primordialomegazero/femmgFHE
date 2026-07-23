// BIDIRECTIONAL STRESS V2: With pre-scaling to control φ growth
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482, psi = -0.6180339887498948482;
CryptoContext<DCRTPoly> cc;
KeyPair<DCRTPoly> keys;

struct PE { Ciphertext<DCRTPoly> a, b; };

double dec_ct(const Ciphertext<DCRTPoly>& ct) {
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
    pt->SetLength(2048); return pt->GetRealPackedValue()[0];
}

PE make_pe(double a, double b) {
    auto enc = [](double v) {
        vector<double> vec(2048, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    return {enc(a), enc(b)};
}

PE clean_forward(const PE& x) {
    auto a_plus_b = cc->EvalAdd(x.a, x.b);
    auto two_b = cc->EvalAdd(x.b, x.b);
    return {a_plus_b, cc->EvalAdd(x.a, two_b)};
}

PE clean_reverse(const PE& x) {
    auto two_a = cc->EvalAdd(x.a, x.a);
    return {cc->EvalSub(two_a, x.b), cc->EvalSub(x.b, x.a)};
}

PE mul_scalar(const PE& x, double s) {
    auto ct_s = make_pe(s, 0.0);
    return {cc->EvalMult(x.a, ct_s.a), cc->EvalMult(x.b, ct_s.a)};
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BIDIRECTIONAL STRESS V2: With Pre-Scaling           ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(50); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(200);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    PE state = make_pe(1.0, 0.0);
    double pre = pow(phi*phi, -1.0);  // Pre-scale for forward clean
    int total_ops = 0, total_rev = 0;
    int fwd_per_epoch = 10;   // Reduced for stability
    int rec_per_epoch = 3;

    cout << "  Pre-scale: " << fixed << setprecision(6) << pre << " (per forward clean)\n";
    cout << "  Per epoch: " << fwd_per_epoch << " FWD + 1 REV + " << rec_per_epoch << " REC\n\n";

    cout << "  Epoch  Ops   φ-value       ψ-noise       Status\n";
    cout << string(55, '-') << "\n";

    for (int epoch = 0; epoch < 10; epoch++) {
        try {
            // Forward phase with pre-scaling
            for (int f = 0; f < fwd_per_epoch; f++) {
                state = clean_forward(state);
                state = mul_scalar(state, pre);  // Compensate φ growth
                total_ops++;
            }
            
            // Reverse reset
            state = clean_reverse(state);
            total_ops++; total_rev++;
            
            // Recovery with pre-scaling
            for (int r = 0; r < rec_per_epoch; r++) {
                state = clean_forward(state);
                state = mul_scalar(state, pre);
                total_ops++;
            }
            
            double pv = dec_ct(state.a) + phi * dec_ct(state.b);
            double sv = dec_ct(state.a) + psi * dec_ct(state.b);
            
            string status = (abs(sv) < 1e-6) ? "✓" : (abs(sv) < 1e-3 ? "~" : "⚠");
            
            cout << setw(4) << epoch << setw(5) << total_ops
                 << setw(13) << scientific << setprecision(3) << pv
                 << setw(13) << scientific << sv
                 << "  " << status << "\n";
            
        } catch (const exception& e) {
            cout << "  ✗ CRASHED epoch " << epoch << ": " << e.what() << "\n";
            break;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  Total ops: " << setw(5) << total_ops << " (FWD+REV: " << total_rev << " reverses)";
    cout << "                   ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
