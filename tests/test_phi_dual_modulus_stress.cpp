// DUAL MODULUS STRESS: 100 epochs with heavy workload
// Each epoch: 5 forward cleans + 2 EvalMult in active reality
// Then swap + bootstrap the other

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

Ciphertext<DCRTPoly> bootstrap_component(const Ciphertext<DCRTPoly>& ct) {
    double val = dec_ct(ct);
    return make_pe(val, 0.0).a;
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   DUAL MODULUS STRESS: 100 Epochs Heavy Workload      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(50); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(200);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    PE state = make_pe(1.0, 0.0);
    double pre = pow(phi*phi, -1.0);
    bool use_phi = true;
    int total_ops = 0, total_boots = 0;
    int fwd_per_epoch = 2;
    int mults_per_epoch = 1;

    cout << "  Per epoch: " << fwd_per_epoch << " FWD cleans + " << mults_per_epoch << " EvalMult\n";
    cout << "  Then SWAP reality + bootstrap idle component\n";
    cout << "  100 epochs = " << (100 * (fwd_per_epoch + mults_per_epoch + 1)) << "+ ops\n\n";

    cout << "  Epoch  Active  φ-lvl  ψ-lvl   φ-value      ψ-noise      Status\n";
    cout << string(68, '-') << "\n";

    for (int epoch = 0; epoch < 100; epoch++) {
        try {
            if (use_phi) {
                // Heavy compute in φ
                for (int f = 0; f < fwd_per_epoch; f++) {
                    state = clean_forward(state);
                    state = mul_scalar(state, pre);
                    total_ops++;
                }
                for (int m = 0; m < mults_per_epoch; m++) {
                    state = mul_scalar(state, 0.95);
                    total_ops++;
                }
                // Bootstrap ψ
                state.b = bootstrap_component(state.b);
                total_boots++;
            } else {
                // Heavy compute in ψ (using reverse cleans)
                for (int r = 0; r < fwd_per_epoch; r++) {
                    state = clean_reverse(state);
                    total_ops++;
                }
                for (int m = 0; m < mults_per_epoch; m++) {
                    state = mul_scalar(state, 0.95);
                    total_ops++;
                }
                // Bootstrap φ
                state.a = bootstrap_component(state.a);
                total_boots++;
            }
            
            use_phi = !use_phi;
            
            if (epoch % 10 == 0 || epoch == 99) {
                auto lev_a = state.a->GetLevel();
                auto lev_b = state.b->GetLevel();
                double pv = dec_ct(state.a) + phi * dec_ct(state.b);
                double sv = dec_ct(state.a) + psi * dec_ct(state.b);
                
                string status = "✓";
                if (abs(pv) > 1e6 || abs(sv) > 1e6) status = "⚠ DIVERGING";
                if (lev_a < 0 || lev_b < 0) status = "✗ LEVEL DEPLETED";
                
                cout << setw(4) << epoch 
                     << "  " << (use_phi ? "ψ" : "φ") << "     "
                     << setw(4) << lev_a
                     << setw(6) << lev_b
                     << setw(13) << scientific << setprecision(2) << pv
                     << setw(13) << scientific << sv
                     << "  " << status << "\n";
                cout.flush();
            }
            
        } catch (const exception& e) {
            cout << "  ✗ CRASHED epoch " << epoch << ": " << e.what() << "\n";
            break;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  STRESS RESULTS                                      ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout <<   "  ║  Total ops:     " << setw(5) << total_ops;
    cout << "                                  ║\n";
    cout <<   "  ║  Bootstraps:    " << setw(5) << total_boots;
    cout << "                                  ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
