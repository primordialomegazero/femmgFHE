// ASYMMETRIC CLEAN: Test varying mul_X:div_X ratios
// Hypothesis: More mul_X than div_X = ψ-noise death
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482, psi = -0.6180339887498948482;

struct PE { Ciphertext<DCRTPoly> a, b; };

double dec_ct(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keys, const Ciphertext<DCRTPoly>& ct) {
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
    pt->SetLength(2048); return pt->GetRealPackedValue()[0];
}

PE make_pe(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& keys, double a, double b) {
    auto enc = [&](double v) {
        vector<double> vec(2048, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };
    return {enc(a), enc(b)};
}

PE mul_X(CryptoContext<DCRTPoly>& cc, const PE& x) { 
    return {x.b, cc->EvalAdd(x.a, x.b)}; 
}

PE div_X(CryptoContext<DCRTPoly>& cc, const PE& x) { 
    return {cc->EvalSub(x.b, x.a), x.a}; 
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ASYMMETRIC CLEAN: mul_X:div_X ratio sweep          ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(59); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(20);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    // Test different ratios: mul_X count vs div_X count
    vector<pair<int,int>> ratios = {{3,1},{5,1},{10,1},{20,1},{50,1},{100,1},{1000,1},{3,0},{10,0},{100,0}};

    for (auto [mul_cnt, div_cnt] : ratios) {
        auto state = make_pe(cc, keys, 1.0, 1.0);  // Start with ψ=1.0
        for (int i = 0; i < mul_cnt; i++) state = mul_X(cc, state);
        for (int i = 0; i < div_cnt; i++) state = div_X(cc, state);
        
        double av = dec_ct(cc, keys, state.a);
        double bv = dec_ct(cc, keys, state.b);
        double phi_val = av + bv * phi;
        double psi_val = av + bv * psi;
        
        cout << "  Ratio " << setw(4) << mul_cnt << ":" << left << setw(4) << div_cnt << right
             << "  φ=" << setw(12) << scientific << setprecision(4) << phi_val
             << "  ψ=" << setw(12) << scientific << psi_val
             << "  ψ/φ=" << setw(12) << scientific << abs(psi_val/phi_val)
             << "  (ψ decay per mul_X: " << fixed << setprecision(4) 
             << pow(abs(psi_val/phi_val), 1.0/mul_cnt) << ")\n";
    }
    cout << "\n";
    return 0;
}
