// PURE MUL_X: What happens with NO div_X at all?
// Does ψ-noise hit a floor or keep decaying?
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

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   PURE MUL_X: Zero div_X — how low can ψ go?        ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(59); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(5);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    auto state = make_pe(cc, keys, 1.0, 1.0);  // ψ starts at 1.0
    
    cout << "  mul_X count    φ-value        ψ-value        ψ/φ ratio\n";
    cout << string(62, '-') << "\n";
    
    for (int cnt : {0, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233}) {
        // Reset state each time for clean measurement
        auto s = make_pe(cc, keys, 1.0, 1.0);
        for (int i = 0; i < cnt; i++) s = mul_X(cc, s);
        
        double av = dec_ct(cc, keys, s.a);
        double bv = dec_ct(cc, keys, s.b);
        double phi_val = av + bv * phi;
        double psi_val = av + bv * psi;
        double ratio = abs(psi_val / phi_val);
        
        // Theoretical: after N mul_X, ψ/φ ≈ |ψ/φ|^N = |ψ²|^N = (0.382)^N
        double theoretical = pow(abs(psi), cnt);
        
        cout << setw(8) << cnt
             << setw(16) << scientific << setprecision(6) << phi_val
             << setw(16) << scientific << psi_val
             << setw(14) << scientific << ratio
             << "  (theory: " << scientific << theoretical << ")\n";
    }
    
    cout << "\n  ψ = -1/φ ≈ -0.618. |ψ| < 1 → exponential decay.\n";
    cout << "  Each mul_X multiplies ψ-component by ψ.\n";
    cout << "  After 21 mul_X: ψ^21 ≈ 4×10⁻⁵. After 89: ψ^89 ≈ 10⁻¹⁸.\n";
    cout << "  ψ-noise approaches machine epsilon with enough mul_X.\n\n";
    return 0;
}
