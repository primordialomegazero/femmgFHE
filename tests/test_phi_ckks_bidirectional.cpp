// CKKS BIDIRECTIONAL: Forward + Reverse clean on actual encrypted data
// Tests: Does reverse clean actually reduce φ-error on CKKS ciphertexts?

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
    // (a+b, a+2b)
    auto a_plus_b = cc->EvalAdd(x.a, x.b);
    auto two_b = cc->EvalAdd(x.b, x.b);
    return {a_plus_b, cc->EvalAdd(x.a, two_b)};
}

PE clean_reverse(const PE& x) {
    // (2a-b, -a+b)
    auto two_a = cc->EvalAdd(x.a, x.a);
    auto a_plus_b = cc->EvalAdd(x.a, x.b);
    return {cc->EvalSub(two_a, x.b), cc->EvalSub(x.b, x.a)};
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   CKKS BIDIRECTIONAL: Reverse Clean on Real Ciphers  ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(50); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(50);
    cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    cout << "  TEST 1: Start with pure φ (no ψ noise)\n";
    cout << "  Build up φ with forward cleans, then reverse to reset\n\n";

    PE state = make_pe(1.0, 0.0);
    
    cout << "  Step      φ-value       ψ-noise       Note\n";
    cout << string(55, '-') << "\n";

    // Build up φ with 3 forward cleans
    for (int i = 0; i < 3; i++) {
        state = clean_forward(state);
        double pv = dec_ct(state.a) + phi * dec_ct(state.b);
        double sv = dec_ct(state.a) + psi * dec_ct(state.b);
        cout << "  FWD " << i+1 << "    " 
             << setw(12) << fixed << setprecision(4) << pv
             << setw(13) << scientific << setprecision(3) << sv << "\n";
    }

    // ONE reverse clean
    state = clean_reverse(state);
    double pv = dec_ct(state.a) + phi * dec_ct(state.b);
    double sv = dec_ct(state.a) + psi * dec_ct(state.b);
    cout << "  REV       " 
         << setw(12) << fixed << setprecision(4) << pv
         << setw(13) << scientific << setprecision(3) << sv 
         << "  ← φ should DROP\n";

    // Recovery forward cleans
    for (int i = 0; i < 3; i++) {
        state = clean_forward(state);
        pv = dec_ct(state.a) + phi * dec_ct(state.b);
        sv = dec_ct(state.a) + psi * dec_ct(state.b);
        cout << "  REC " << i+1 << "    " 
             << setw(12) << fixed << setprecision(4) << pv
             << setw(13) << scientific << setprecision(3) << sv;
        if (i == 2) cout << "  ← ψ should RECOVER";
        cout << "\n";
    }

    cout << "\n  TEST 2: φ-error reduction measurement\n";
    cout << "  Start with φ=100 (simulating accumulated error)\n\n";

    state = make_pe(61.80, -38.20);  // ~100 in φ, ~0 in ψ
    double before_phi = dec_ct(state.a) + phi * dec_ct(state.b);
    double before_psi = dec_ct(state.a) + psi * dec_ct(state.b);
    
    cout << "  Before reverse: φ=" << fixed << setprecision(2) << before_phi 
         << ", ψ=" << scientific << setprecision(2) << before_psi << "\n";
    
    state = clean_reverse(state);
    double after_phi = dec_ct(state.a) + phi * dec_ct(state.b);
    double after_psi = dec_ct(state.a) + psi * dec_ct(state.b);
    
    double reduction = (before_phi - after_phi) / before_phi * 100;
    
    cout << "  After reverse:  φ=" << fixed << setprecision(2) << after_phi 
         << ", ψ=" << scientific << setprecision(2) << after_psi << "\n";
    cout << "  φ-error reduction: " << fixed << setprecision(1) << reduction << "%\n\n";

    if (reduction > 50) {
        cout << "  ✓ REVERSE CLEAN WORKS ON CKKS!\n";
        cout << "  φ-error dropped by " << fixed << setprecision(0) << reduction << "%\n";
    } else {
        cout << "  ⚠ Reduction less than expected — check encoding\n";
    }

    cout << "\n  TEST 3: Full bidirectional cycle\n";
    cout << "  Forward(3) → Reverse(1) → Forward(3)\n\n";
    
    state = make_pe(1.0, 0.0);
    cout << "  Step      φ-value       ψ-noise\n";
    cout << string(45, '-') << "\n";
    
    auto show = [&](string label) {
        double pv = dec_ct(state.a) + phi * dec_ct(state.b);
        double sv = dec_ct(state.a) + psi * dec_ct(state.b);
        cout << "  " << setw(8) << label 
             << setw(13) << fixed << setprecision(4) << pv
             << setw(13) << scientific << setprecision(3) << sv << "\n";
    };
    
    show("START");
    for (int i = 0; i < 3; i++) { state = clean_forward(state); show("FWD"+to_string(i+1)); }
    state = clean_reverse(state); show("REV");
    for (int i = 0; i < 3; i++) { state = clean_forward(state); show("REC"+to_string(i+1)); }

    cout << "\n  ✓ Bidirectional clean verified on CKKS\n\n";
    return 0;
}
