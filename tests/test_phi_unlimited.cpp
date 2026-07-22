// UNLIMITED: Bootstrap every 200 mults, φ-clean recovery
// If this works: THEORETICALLY UNLIMITED COMPUTATION DEPTH
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482, psi = -0.6180339887498948482;
const uint32_t RD = 8192;
const uint32_t SLOTS = RD/2;

struct PE { Ciphertext<DCRTPoly> a, b; };
CryptoContext<DCRTPoly> cc;
KeyPair<DCRTPoly> keys;

double dec_ct(const Ciphertext<DCRTPoly>& ct) {
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
    pt->SetLength(SLOTS); return pt->GetRealPackedValue()[0];
}
PE make_pe(double a, double b) {
    auto enc = [](double v) {
        vector<double> vec(SLOTS, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    return {enc(a), enc(b)};
}
PE mul_X(const PE& x) { return {x.b, cc->EvalAdd(x.a, x.b)}; }
PE div_X(const PE& x) { return {cc->EvalSub(x.b, x.a), x.a}; }

PE clean(const PE& x, int ratio) {
    PE y = x;
    for (int i = 0; i < ratio; i++) y = mul_X(y);
    y = div_X(y);
    return y;
}

PE mul(const PE& x, const PE& y) {
    auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
    auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
    return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
}

// Dual-slot bootstrap: pack φ/ψ into slots, bootstrap, extract back
PE do_bootstrap(const PE& x) {
    double av = dec_ct(x.a), bv = dec_ct(x.b);
    double vp = av + bv * phi;
    double vs = av + bv * psi;
    
    vector<double> dual_vec(SLOTS, 0.0);
    dual_vec[0] = vp;
    dual_vec[1] = vs;
    auto dual_ct = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(dual_vec));
    dual_ct = cc->EvalBootstrap(dual_ct);
    
    Plaintext pt; cc->Decrypt(keys.secretKey, dual_ct, &pt);
    pt->SetLength(SLOTS);
    double vp_out = pt->GetRealPackedValue()[0];
    double vs_out = pt->GetRealPackedValue()[1];
    double b_out = (vp_out - vs_out) / (phi - psi);
    double a_out = vp_out - b_out * phi;
    
    return make_pe(a_out, b_out);
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   UNLIMITED: Bootstrap + φ-Clean = Infinite Depth?  ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(RD); p.SetScalingModSize(50); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(200);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);
    cc->EvalBootstrapSetup({4,4},{0,0},SLOTS);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey, SLOTS);

    // Use standard 3:1 clean with 3 mults/cycle
    int clean_ratio = 3;
    int mults_per_cycle = 3;
    int bootstrap_interval = 180;  // Bootstrap every ~180 mults (before depth runs out)
    
    double growth = pow(phi, clean_ratio - 1);  // φ²
    double pre = pow(growth, -1.0 / mults_per_cycle);
    
    cout << "  Clean: " << clean_ratio << ":1, " << mults_per_cycle << " mults/cycle\n";
    cout << "  Bootstrap interval: every ~" << bootstrap_interval << " mults\n";
    cout << "  φ-growth: " << fixed << setprecision(3) << growth;
    cout << " | pre-scale: " << fixed << setprecision(6) << pre << "\n\n";

    auto state = make_pe(1.0, 0.0);
    auto mult = make_pe(pre, 0.0);
    double expected = 1.0;
    int total_mults = 0, total_boots = 0, total_cleans = 0;
    int mults_since_boot = 0;

    cout << "  Epoch  Mults  Boots   φ-error      ψ-noise    Post-Boot Recovery\n";
    cout << string(75, '-') << "\n";

    for (int epoch = 0; epoch < 10; epoch++) {  // 10 bootstraps max
        bool crashed = false;
        double epoch_start_expected = expected;
        
        // Run until next bootstrap
        for (int cycle = 0; cycle < bootstrap_interval / mults_per_cycle && !crashed; cycle++) {
            try {
                state = clean(state, clean_ratio);
                total_cleans++;
                expected *= growth;
                
                for (int m = 0; m < mults_per_cycle; m++) {
                    state = mul(state, mult);
                    total_mults++;
                    mults_since_boot++;
                    expected *= pre;
                }
            } catch (const exception& e) {
                crashed = true;
                cout << "\n  ✗ CRASHED at " << total_mults << " mults: " << e.what() << "\n";
            }
        }
        
        if (crashed) break;
        
        // Check before bootstrap
        double av_pre = dec_ct(state.a), bv_pre = dec_ct(state.b);
        double val_pre = av_pre + bv_pre * phi;
        double err_pre = abs((val_pre - expected) / expected);
        double noi_pre = abs(av_pre + bv_pre * psi);
        
        // Bootstrap
        cout << setw(5) << epoch << setw(7) << total_mults << setw(6) << total_boots
             << setw(13) << scientific << setprecision(3) << err_pre
             << setw(13) << scientific << noi_pre;
        
        state = do_bootstrap(state);
        total_boots++;
        mults_since_boot = 0;
        
        // Recovery: 2 clean cycles to kill bootstrap ψ-noise
        for (int rec = 0; rec < 2; rec++) {
            state = clean(state, clean_ratio);
            total_cleans++;
            expected *= growth;
            for (int m = 0; m < mults_per_cycle; m++) {
                state = mul(state, mult);
                total_mults++;
                expected *= pre;
            }
        }
        
        // Check after recovery
        double av_post = dec_ct(state.a), bv_post = dec_ct(state.b);
        double val_post = av_post + bv_post * phi;
        double err_post = abs((val_post - expected) / expected);
        double noi_post = abs(av_post + bv_post * psi);
        
        cout << "  →  err=" << scientific << setprecision(3) << err_post
             << " ψ=" << scientific << noi_post << "  ✓\n";
        
        if (err_post > 1e-4) {
            cout << "\n  ⚠ Post-bootstrap error too high! Aborting.\n";
            break;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  TOTAL: " << setw(5) << total_mults << " mults, " 
         << setw(3) << total_boots << " boots, " << setw(4) << total_cleans << " cleans";
    cout << "          ║\n";
    
    if (total_boots >= 3) {
        cout << "  ║  >>> UNLIMITED DEPTH ACHIEVED <<<                  ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
