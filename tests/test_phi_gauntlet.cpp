// THE GAUNTLET: Push until it breaks or we get bored
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>
using namespace lbcrypto;
using namespace std;
using namespace chrono;

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
    cout << "\n";
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout << "  ║              THE GAUNTLET — Push to Failure              ║\n";
    cout << "  ╚══════════════════════════════════════════════════════════╝\n\n";

    auto start_time = high_resolution_clock::now();

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

    int clean_ratio = 3;
    int mults_per_cycle = 3;
    int bootstrap_interval_mults = 180;
    
    double growth = pow(phi, clean_ratio - 1);
    double pre = pow(growth, -1.0 / mults_per_cycle);
    
    cout << "  Config: RingDim=" << RD << " | Clean=" << clean_ratio << ":1 | ";
    cout << "Depth=200 | Boot every ~" << bootstrap_interval_mults << " mults\n";
    cout << "  Target: Run until death, 1000+ mults\n\n";

    auto state = make_pe(1.0, 0.0);
    auto mult = make_pe(pre, 0.0);
    double expected = 1.0;
    int total_mults = 0, total_boots = 0, total_cleans = 0;
    int mults_since_boot = 0;
    bool alive = true;

    cout << "  Boot#   Mults   φ-error(pre)  ψ-noise(pre)  φ-error(post) ψ-noise(post)\n";
    cout << string(78, '-') << "\n";

    for (int boot_num = 0; boot_num < 50 && alive; boot_num++) {
        // Run multiplications until bootstrap threshold
        int cycles_this_epoch = bootstrap_interval_mults / mults_per_cycle;
        
        for (int c = 0; c < cycles_this_epoch && alive; c++) {
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
                cout << "\n  ✗ CRASHED during computation at mult " << total_mults << ": " << e.what() << "\n";
                alive = false;
            }
        }
        
        if (!alive) break;
        
        // Measure pre-bootstrap
        double av_pre = dec_ct(state.a), bv_pre = dec_ct(state.b);
        double err_pre = abs((av_pre + bv_pre * phi - expected) / expected);
        double noi_pre = abs(av_pre + bv_pre * psi);
        
        // Bootstrap
        try {
            state = do_bootstrap(state);
            total_boots++;
            mults_since_boot = 0;
        } catch (const exception& e) {
            cout << "\n  ✗ CRASHED during bootstrap " << total_boots << " at mult " << total_mults << ": " << e.what() << "\n";
            alive = false;
            break;
        }
        
        // Recovery clean (2 cycles)
        for (int rec = 0; rec < 2 && alive; rec++) {
            try {
                state = clean(state, clean_ratio);
                total_cleans++;
                expected *= growth;
                for (int m = 0; m < mults_per_cycle; m++) {
                    state = mul(state, mult);
                    total_mults++;
                    expected *= pre;
                }
            } catch (const exception& e) {
                cout << "\n  ✗ CRASHED during recovery at mult " << total_mults << ": " << e.what() << "\n";
                alive = false;
            }
        }
        
        if (!alive) break;
        
        // Measure post-recovery
        double av_post = dec_ct(state.a), bv_post = dec_ct(state.b);
        double err_post = abs((av_post + bv_post * phi - expected) / expected);
        double noi_post = abs(av_post + bv_post * psi);
        
        // Progress indicator
        cout << setw(5) << total_boots << setw(8) << total_mults
             << setw(15) << scientific << setprecision(3) << err_pre
             << setw(14) << scientific << noi_pre
             << setw(15) << scientific << err_post
             << setw(14) << scientific << noi_post;
        
        // Status
        if (err_post > 1e-2) {
            cout << "  ⚠ DIVERGED\n";
            alive = false;
        } else if (err_post > 1e-4) {
            cout << "  △ WARNING\n";
        } else {
            cout << "  ✓\n";
        }
        
        // Every 5 bootstraps, show cumulative stats
        if (total_boots % 5 == 0 && alive) {
            auto elapsed = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();
            cout << "  │ " << total_mults << " mults, " << total_boots << " boots, " 
                 << total_cleans << " cleans | " << elapsed << "s elapsed\n";
        }
    }

    auto elapsed = duration_cast<seconds>(high_resolution_clock::now() - start_time).count();
    
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  FINAL: " << setw(5) << total_mults << " mults, " 
         << setw(3) << total_boots << " boots, " << setw(5) << total_cleans << " cleans";
    cout << "                   ║\n";
    cout <<   "  ║  Time: " << elapsed << "s";
    
    if (alive && total_boots >= 5) {
        cout << " | Status: UNLIMITED DEPTH ACHIEVED          ║\n";
    } else if (alive) {
        cout << " | Status: STILL RUNNING                     ║\n";
    } else {
        cout << " | Status: TERMINATED                        ║\n";
    }
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    return 0;
}
