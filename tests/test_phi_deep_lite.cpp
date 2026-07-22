// DEEP LITE: RingDim=8192
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

int main() {
    cout << "\n  === DEEP LITE: RingDim=" << RD << ", Slots=" << SLOTS << " ===\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(RD); p.SetScalingModSize(50); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(200);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    struct Config { int ratio; int mpc; string name; };
    vector<Config> configs = {
        {3, 3, "Standard (3:1, 3 mult/cycle)"},
        {5, 2, "Aggressive (5:1, 2 mult/cycle)"},
        {8, 1, "Max clean (8:1, 1 mult/cycle)"},
    };
    
    for (auto& cfg : configs) {
        double growth = pow(phi, cfg.ratio - 1);
        double pre_val = pow(growth, -1.0 / cfg.mpc);
        
        auto state = make_pe(1.0, 0.0);
        auto mult = make_pe(pre_val, 0.0);
        double expected = 1.0;
        int total_mults = 0;
        
        cout << "  " << cfg.name << " | growth=" << fixed << setprecision(2) << growth 
             << " pre=" << setprecision(5) << pre_val << "\n  ";
        
        bool crashed = false;
        for (int cycle = 0; cycle < 300 && !crashed; cycle++) {
            try {
                state = clean(state, cfg.ratio);
                expected *= growth;
                for (int m = 0; m < cfg.mpc; m++) {
                    state = mul(state, mult);
                    total_mults++; expected *= pre_val;
                }
                if (total_mults % 50 == 0) {
                    double av = dec_ct(state.a), bv = dec_ct(state.b);
                    double val = av + bv * phi;
                    double err = abs((val - expected) / expected);
                    double noi = abs(av + bv * psi);
                    cout << "[" << total_mults << "m e=" << scientific << setprecision(1) << err 
                         << " n=" << noi << "] ";
                    cout.flush();
                }
            } catch (const exception& e) {
                crashed = true;
                cout << "\n  CRASHED at " << total_mults << " mults: " << e.what() << "\n";
            }
        }
        if (!crashed) {
            double av = dec_ct(state.a), bv = dec_ct(state.b);
            double val = av + bv * phi;
            double err = abs((val - expected) / expected);
            double noi = abs(av + bv * psi);
            cout << " DONE[" << total_mults << "m] e=" << scientific << setprecision(3) 
                 << err << " n=" << noi;
        }
        cout << "\n\n";
    }
    return 0;
}
