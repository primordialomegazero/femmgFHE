// STRESS 500: Push the system to its limit
// Aggressive clean (10:1), minimal pre-scaling, track both errors
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482, psi = -0.6180339887498948482;

struct PE { Ciphertext<DCRTPoly> a, b; };

CryptoContext<DCRTPoly> cc;
KeyPair<DCRTPoly> keys;

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

PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

PE clean_aggressive(PE x, int ratio = 10) {
    for (int i = 0; i < ratio; i++) x = mul_X(x);
    x = div_X(x);
    return x;
}

PE mul(const PE& x, const PE& y) {
    auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
    auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
    return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   STRESS 500: Maximum multiplications, minimal boot  ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(59); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(300);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE); cc->EvalBootstrapSetup({4,4},{0,0},2048);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey, 2048);

    double pre = 0.95;
    auto state = make_pe(1.0, 0.0);
    auto mult = make_pe(pre, 0.0);
    double expected = 1.0;
    int total_mults = 0, total_cleans = 0, total_boots = 0;
    int msb = 0;  // mults since bootstrap

    cout << "  Cycle  Mults   Cln  Boot  φ-error      ψ-noise      Status\n";
    cout << string(72, '-') << "\n";

    for (int cycle = 0; cycle < 200; cycle++) {
        try {
            // Aggressive clean
            state = clean_aggressive(state, 10);
            total_cleans++;

            // 3 multiplications per cycle
            for (int m = 0; m < 3; m++) {
                state = mul(state, mult);
                total_mults++; msb++;
                expected *= pre;
            }

            // Bootstrap only when absolutely necessary
            bool booted = false;
            if (msb >= 100) {  // Push bootstrap interval
                auto dual = make_pe(dec_ct(state.a), dec_ct(state.b));
                // Re-encrypt as dual for bootstrap
                vector<double> vp(2048, 0.0), vs(2048, 0.0);
                double av = dec_ct(state.a), bv = dec_ct(state.b);
                vp[0] = av + bv * phi;
                vs[0] = av + bv * psi;
                
                // Pack into slots 0 and 1
                vector<double> dual_vec(2048, 0.0);
                dual_vec[0] = vp[0];
                dual_vec[1] = vs[0];
                auto dual_ct = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(dual_vec));
                dual_ct = cc->EvalBootstrap(dual_ct);
                
                // Extract back
                Plaintext pt; cc->Decrypt(keys.secretKey, dual_ct, &pt);
                pt->SetLength(2048);
                double vp_out = pt->GetRealPackedValue()[0];
                double vs_out = pt->GetRealPackedValue()[1];
                double b_out = (vp_out - vs_out) / (phi - psi);
                double a_out = vp_out - b_out * phi;
                
                state = make_pe(a_out, b_out);
                total_boots++; msb = 0; booted = true;
            }

            double av = dec_ct(state.a), bv = dec_ct(state.b);
            double val = av + bv * phi;
            double err = abs((val - expected) / expected);
            double noi = abs(av + bv * psi);

            string status = "✓";
            if (err > 1e-4) status = "φ-WARN";
            if (noi > 1e-4) status = "ψ-WARN";
            if (err > 1e-1) status = "φ-DIVERGED";
            if (noi > 1e-1) status = "ψ-DIVERGED";

            if (cycle % 20 == 0 || booted || status != "✓") {
                cout << setw(5) << cycle << setw(7) << total_mults << setw(5) << total_cleans 
                     << setw(5) << total_boots << (booted?"⚡":"  ")
                     << setw(13) << scientific << setprecision(3) << err
                     << setw(13) << scientific << noi
                     << "  " << status << "\n";
            }

            if (status.find("DIVERGED") != string::npos) {
                cout << "\n  ⚠ Diverged at cycle " << cycle 
                     << " (" << total_mults << " mults, " << total_boots << " boots)\n";
                break;
            }
        } catch (const exception& e) {
            cout << "\n  ✗ CRASHED at cycle " << cycle << ": " << e.what() << "\n";
            break;
        }
    }

    cout << "\n  Final: " << total_mults << " mults, " << total_cleans 
         << " cleans, " << total_boots << " bootstraps\n\n";
    return 0;
}
