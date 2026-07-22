// SPRINT: Fast bootstrap cycling, log to file
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <fstream>
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
    Plaintext pt; cc->Decrypt(keys.secretKey, dual_ct, &pt);
    pt->SetLength(SLOTS);
    double vp_out = pt->GetRealPackedValue()[0], vs_out = pt->GetRealPackedValue()[1];
    double b_out = (vp_out - vs_out) / (phi - psi);
    double a_out = vp_out - b_out * phi;
    return make_pe(a_out, b_out);
}

int main() {
    ofstream log("gauntlet_log.txt");
    auto logboth = [&](const string& s) { cout << s; log << s; };

    logboth("\n  SPRINT: Bootstrap every 60 mults, log to gauntlet_log.txt\n");
    logboth("  RingDim=8192 | Clean=3:1 | Max 100 bootstraps\n\n");

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(RD); p.SetScalingModSize(50); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(80);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE);
    cc->EvalBootstrapSetup({4,4},{0,0},SLOTS);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey, SLOTS);

    int clean_ratio = 3, mults_per_cycle = 3, boot_interval = 60;
    double growth = pow(phi, clean_ratio - 1);
    double pre = pow(growth, -1.0 / mults_per_cycle);

    auto state = make_pe(1.0, 0.0);
    auto mult = make_pe(pre, 0.0);
    double expected = 1.0;
    int total_mults = 0, total_boots = 0;

    logboth("  Boot  Mults   φ-error       ψ-noise       Status\n");
    logboth(string(58, '-') + "\n");

    for (int b = 0; b < 100; b++) {
        for (int c = 0; c < boot_interval / mults_per_cycle; c++) {
            state = clean(state, clean_ratio);
            expected *= growth;
            for (int m = 0; m < mults_per_cycle; m++) {
                state = mul(state, mult);
                total_mults++; expected *= pre;
            }
        }
        
        double av = dec_ct(state.a), bv = dec_ct(state.b);
        double err = abs((av + bv * phi - expected) / expected);
        double noi = abs(av + bv * psi);
        
        string status = "✓";
        if (err > 1e-2) status = "DIVERGED";
        else if (err > 1e-4) status = "WARN";
        
        ostringstream oss;
        oss << setw(4) << total_boots << setw(7) << total_mults
            << setw(13) << scientific << setprecision(3) << err
            << setw(13) << scientific << noi
            << "  " << status << "\n";
        logboth(oss.str());
        
        if (status == "DIVERGED") break;
        
        state = do_bootstrap(state);
        total_boots++;
        
        // Single recovery clean
        for (int rec = 0; rec < 1; rec++) {
            state = clean(state, clean_ratio);
            expected *= growth;
            for (int m = 0; m < mults_per_cycle; m++) {
                state = mul(state, mult);
                total_mults++; expected *= pre;
            }
        }
    }

    ostringstream final;
    final << "\n  FINAL: " << total_mults << " mults, " << total_boots << " boots\n";
    logboth(final.str());
    log.close();
    return 0;
}
