// BOOTSTRAP SPARSE: Push bootstrap interval to the limit
// Test: 200, 300, 400, 500 mults per bootstrap
// Goal: Find maximum safe interval
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482, psi = -0.6180339887498948482;
const uint32_t RD = 8192, SLOTS = RD/2;

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
    double vp = av + bv * phi, vs = av + bv * psi;
    vector<double> dual_vec(SLOTS, 0.0);
    dual_vec[0] = vp; dual_vec[1] = vs;
    auto dual_ct = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(dual_vec));
    dual_ct = cc->EvalBootstrap(dual_ct);
    Plaintext pt; cc->Decrypt(keys.secretKey, dual_ct, &pt); pt->SetLength(SLOTS);
    double vp_out = pt->GetRealPackedValue()[0], vs_out = pt->GetRealPackedValue()[1];
    double b_out = (vp_out - vs_out) / (phi - psi);
    double a_out = vp_out - b_out * phi;
    return make_pe(a_out, b_out);
}

void test_interval(int boot_interval, int clean_ratio) {
    double growth = pow(phi, clean_ratio - 1);
    double pre = pow(growth, -1.0/3.0);
    auto state = make_pe(1.0, 0.0);
    auto mult = make_pe(pre, 0.0);
    double expected = 1.0;
    int tm = 0, tb = 0;
    bool alive = true;
    
    for (int b = 0; b < 3 && alive; b++) {
        for (int c = 0; c < boot_interval/3; c++) {
            state = clean(state, clean_ratio);
            expected *= growth;
            for (int m = 0; m < 3; m++) {
                state = mul(state, mult);
                tm++; expected *= pre;
            }
        }
        double av = dec_ct(state.a), bv = dec_ct(state.b);
        double err = abs((av + bv*phi - expected)/expected);
        double noi = abs(av + bv*psi);
        
        if (err > 1e-2 || noi > 1e-2) {
            cout << "  FAILED at " << tm << " mults, err=" << scientific << setprecision(2) << err << "\n";
            alive = false;
            break;
        }
        
        try {
            state = do_bootstrap(state); tb++;
            for (int rec = 0; rec < 2; rec++) {
                state = clean(state, clean_ratio);
                expected *= growth;
                for (int m = 0; m < 3; m++) {
                    state = mul(state, mult);
                    tm++; expected *= pre;
                }
            }
        } catch (const exception& e) {
            cout << "  BOOT CRASH at " << tm << " mults\n";
            alive = false;
        }
    }
    if (alive) {
        double av = dec_ct(state.a), bv = dec_ct(state.b);
        double err = abs((av + bv*phi - expected)/expected);
        double noi = abs(av + bv*psi);
        cout << "  OK: " << tm << " mults, " << tb << " boots, err=" << scientific << setprecision(2) << err << " noi=" << noi << "\n";
    }
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   SPARSE BOOTSTRAP: Find maximum safe interval       ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(RD); p.SetScalingModSize(50); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(500);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE); cc->EvalBootstrapSetup({4,4},{0,0},SLOTS);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey, SLOTS);
    cout << "  RingDim=8192, Depth=500\n\n";

    vector<int> intervals = {60, 120, 180, 240, 300, 400, 500};
    vector<int> ratios = {3, 5};
    
    for (int ratio : ratios) {
        cout << "  Clean ratio " << ratio << ":1\n";
        cout << "  Interval  Result\n  " << string(30, '-') << "\n";
        for (int interval : intervals) {
            cout << "  " << setw(8) << interval << "  ";
            cout.flush();
            test_interval(interval, ratio);
        }
        cout << "\n";
    }
    return 0;
}
