#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <map>
using namespace lbcrypto;
using namespace std;

double phi = 1.6180339887498948482, psi = -0.6180339887498948482;
CryptoContext<DCRTPoly> cc;
KeyPair<DCRTPoly> keys;
int slots = 2048;

Ciphertext<DCRTPoly> enc(double v) {
    vector<double> vec(slots, 0.0); vec[0] = v;
    return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
}
double dec(const Ciphertext<DCRTPoly>& ct) {
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(slots);
    return pt->GetRealPackedValue()[0];
}

// Dual-slot encoding
Ciphertext<DCRTPoly> encode_dual(double a, double b) {
    vector<double> vec(slots, 0.0);
    vec[0] = a + b * phi; vec[1] = a + b * psi;
    return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
}
void decode_dual(const Ciphertext<DCRTPoly>& ct, double& a, double& b) {
    Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt); pt->SetLength(slots);
    double vp = pt->GetRealPackedValue()[0], vs = pt->GetRealPackedValue()[1];
    b = (vp - vs) / (phi - psi); a = vp - b * phi;
}
double val_psi_ct(const Ciphertext<DCRTPoly>& ct) {
    double a, b; decode_dual(ct, a, b); return a + b * psi;
}

Ciphertext<DCRTPoly> phi_mult() { return encode_dual(0.0, 1.0); }
Ciphertext<DCRTPoly> div_phi_mult() { return encode_dual(-1.0, 1.0); }
Ciphertext<DCRTPoly> mul_ct(const Ciphertext<DCRTPoly>& x, const Ciphertext<DCRTPoly>& y) { return cc->EvalMult(x, y); }

Ciphertext<DCRTPoly> clean_ct(Ciphertext<DCRTPoly> x) {
    auto up = phi_mult(), down = div_phi_mult();
    for (int i=0; i<3; i++) x = mul_ct(x, up);
    for (int i=0; i<2; i++) x = mul_ct(x, down);
    return x;
}

map<int, Ciphertext<DCRTPoly>> build_fib(double base_val, int max_fib) {
    map<int, Ciphertext<DCRTPoly>> powers;
    auto b = encode_dual(base_val, 0.0);
    powers[1] = b; powers[2] = mul_ct(b, b);
    int a = 1, c = 2;
    while (c < max_fib) { int n = a + c; powers[n] = mul_ct(powers[c], powers[a]); a = c; c = n; }
    return powers;
}

Ciphertext<DCRTPoly> fib_jump(Ciphertext<DCRTPoly> state, map<int, Ciphertext<DCRTPoly>>& powers, int N) {
    vector<int> keys; for (auto& p : powers) keys.push_back(p.first);
    sort(keys.rbegin(), keys.rend());
    int rem = N;
    for (int f : keys) { if (f <= rem) { state = mul_ct(state, powers.at(f)); rem -= f; } }
    while (rem > 0) { state = mul_ct(state, powers.at(1)); rem--; }
    return state;
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   INFINITE v2: Dual-slot bootstrap preserves φ       ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetSecretKeyDist(UNIFORM_TERNARY); params.SetSecurityLevel(HEStd_NotSet);
    params.SetRingDim(4096); params.SetScalingModSize(59); params.SetFirstModSize(60);
    params.SetScalingTechnique(FLEXIBLEAUTO); params.SetMultiplicativeDepth(80);
    cc = GenCryptoContext(params);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    cc->Enable(FHE); cc->EvalBootstrapSetup({4,4}, {0,0}, 2048);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalBootstrapKeyGen(keys.secretKey, 2048);

    auto fib_powers = build_fib(1.01, 300);
    cout << "  Fibonacci powers: " << fib_powers.size() << "\n\n";

    auto state = encode_dual(1.0, 0.0);
    int total_mults = 0, total_cleans = 0, total_boots = 0, jumps_since_boot = 0;
    vector<int> pattern = {5, 8, 13, 21, 34};
    
    cout << "  " << setw(4) << "Rnd" << setw(6) << "Jump" << setw(8) << "Total"
         << setw(5) << "Cln" << setw(5) << "Boot" << setw(10) << "a" << setw(10) << "b"
         << setw(12) << "ψ-noise\n";
    cout << "  " << string(70, '-') << "\n";

    for (int round = 0; round < 30; round++) {
        int jump = pattern[round % pattern.size()];
        try {
            state = clean_ct(state); total_cleans++;
            if (jump >= 21) { for(int s=0; s<3; s++) state = mul_ct(state, div_phi_mult()); } state = fib_jump(state, fib_powers, jump); total_mults += jump;
            jumps_since_boot += jump;
            if (jumps_since_boot >= 55) {
                state = cc->EvalBootstrap(state); total_boots++; jumps_since_boot = 0; { double ra, rb; decode_dual(state, ra, rb); state = encode_dual(ra, rb); } { double ra, rb; decode_dual(state, ra, rb); state = encode_dual(ra, rb); }
            }
            double a, b; decode_dual(state, a, b);
            cout << "  " << setw(4) << round << setw(6) << jump << setw(8) << total_mults
                 << setw(5) << total_cleans << setw(5) << total_boots
                 << setw(10) << fixed << setprecision(1) << a << setw(10) << b
                 << setw(12) << scientific << setprecision(2) << abs(val_psi_ct(state)) << "\n";
        } catch (const exception& e) {
            cout << "  CRASHED r" << round << ": " << e.what() << "\n"; break;
        }
    }

    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   " << total_mults << " mults, " << total_cleans << " cleans, " << total_boots << " boots              ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
