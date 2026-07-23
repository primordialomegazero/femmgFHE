// BIDIRECTIONAL 100: 100 epochs with modulus tracking + EvalMult workload
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
    cout <<   "  ║   BIDIRECTIONAL 100: 100 Epochs + Modulus Tracking   ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(50); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(500);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    PE state = make_pe(1.0, 0.0);
    double pre = pow(phi*phi, -1.0);
    int total_fwd = 0, total_rev = 0, total_mult = 0;
    int fwd_per_epoch = 3;   // 5 forward cleans
    int rec_per_epoch = 2;   // 2 recovery cleans

    cout << "  Per epoch: " << fwd_per_epoch << " FWD + 1 REV + " << rec_per_epoch << " REC";
    cout << " + 2 EvalMult\n";
    cout << "  100 epochs = " << (100 * (fwd_per_epoch + 1 + rec_per_epoch + 2)) << "+ ops\n\n";

    // Show every 10 epochs
    for (int epoch = 0; epoch < 100; epoch++) {
        try {
            // Forward cleans with pre-scale
            for (int f = 0; f < fwd_per_epoch; f++) {
                state = clean_forward(state);
                state = mul_scalar(state, pre);
                total_fwd++;
            }
            
            // EvalMult workload: 2 multiplications per epoch
            auto mult = make_pe(0.9, 0.0);  // Decay factor
            state = mul_scalar(state, 0.9);
            total_mult++;
            state = mul_scalar(state, 0.9);
            total_mult++;
            
            // Reverse reset
            state = clean_reverse(state);
            total_rev++;
            
            // Recovery
            for (int r = 0; r < rec_per_epoch; r++) {
                state = clean_forward(state);
                state = mul_scalar(state, pre);
                total_fwd++;
            }
            
            if (epoch % 10 == 0 || epoch == 99) {
                double pv = dec_ct(state.a) + phi * dec_ct(state.b);
                double sv = dec_ct(state.a) + psi * dec_ct(state.b);
                
                // Get modulus level
                auto level = state.a->GetLevel();
                
                cout << setw(5) << epoch 
                     << setw(6) << (total_fwd + total_rev)
                     << setw(6) << total_mult
                     << setw(5) << level
                     << setw(13) << scientific << setprecision(2) << pv
                     << setw(13) << scientific << sv
                     << "  ✓\n";
            }
            
        } catch (const exception& e) {
            cout << "  ✗ CRASHED epoch " << epoch << ": " << e.what() << "\n";
            break;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  RESULTS                                             ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout <<   "  ║  Forward cleans: " << setw(5) << total_fwd;
    cout << "                                ║\n";
    cout <<   "  ║  Reverse cleans: " << setw(5) << total_rev;
    cout << "                                ║\n";
    cout <<   "  ║  EvalMult ops:   " << setw(5) << total_mult;
    cout << "                                ║\n";
    cout <<   "  ║  Total ops:      " << setw(5) << (total_fwd + total_rev + total_mult);
    cout << "                                ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
