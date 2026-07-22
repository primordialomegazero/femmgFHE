// BINET JUMP: Compute φ^N in O(log N) via matrix exponentiation
// Instead of Zeckendorf decomposition, use fast exponentiation
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

PE mul_pe(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto ac = cc->EvalMult(x.a, y.a), bd = cc->EvalMult(x.b, y.b);
    auto ad = cc->EvalMult(x.a, y.b), bc = cc->EvalMult(x.b, y.a);
    return {cc->EvalAdd(ac, bd), cc->EvalAdd(cc->EvalAdd(ad, bc), bd)};
}

// Fast exponentiation: X^N using binary exponentiation (square and multiply)
PE pow_X(CryptoContext<DCRTPoly>& cc, const PE& X, int N) {
    PE result = make_pe(cc, cc->GetCryptoContext()->GetKeyGen()->GetKeyPair().secretKey, 1.0, 0.0);
    // Actually we need the same cc/keys... let's do this differently
    // Return X^N using repeated mul_pe with binary exponentiation
    return result;  // Placeholder — needs proper implementation with cc/keys threading
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BINET JUMP: Fast φ-exponentiation concept test      ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    CCParams<CryptoContextCKKSRNS> p;
    p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    p.SetRingDim(4096); p.SetScalingModSize(59); p.SetFirstModSize(60);
    p.SetScalingTechnique(FLEXIBLEAUTO); p.SetMultiplicativeDepth(60);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto keys = cc->KeyGen(); cc->EvalMultKeyGen(keys.secretKey);

    // Test: Compute X^N for various N using both direct iteration and binary exp
    // X = (0 + 1·X) — the ring element X itself
    auto X = make_pe(cc, keys, 0.0, 1.0);
    
    // Direct iteration vs binary exponentiation comparison
    vector<int> powers = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89};
    
    cout << "  Power N    Direct φ^N    Binary φ^N    Match?\n";
    cout << string(55, '-') << "\n";
    
    for (int N : powers) {
        // Direct: multiply X by itself N times
        auto direct = make_pe(cc, keys, 1.0, 0.0);  // 1
        auto X_copy = X;
        for (int i = 0; i < N; i++) {
            direct = mul_pe(cc, direct, X_copy);
        }
        
        // Binary exponentiation (square and multiply)
        auto binary = make_pe(cc, keys, 1.0, 0.0);  // 1
        auto base = X;
        int exp = N;
        while (exp > 0) {
            if (exp & 1) binary = mul_pe(cc, binary, base);
            base = mul_pe(cc, base, base);
            exp >>= 1;
        }
        
        double dv = dec_ct(cc, keys, direct.a) + dec_ct(cc, keys, direct.b) * phi;
        double bv = dec_ct(cc, keys, binary.a) + dec_ct(cc, keys, binary.b) * phi;
        double expected_phi_N = pow(phi, N);
        
        cout << setw(8) << N 
             << setw(15) << scientific << setprecision(6) << dv
             << setw(15) << scientific << bv
             << "  φ^" << N << "=" << scientific << expected_phi_N
             << "  " << (abs(dv-bv)<1e-10 ? "✓" : "✗") << "\n";
    }
    
    // Fibonacci numbers via Binet: F(N) = (φ^N - ψ^N)/√5
    cout << "\n  Fibonacci via Binet:\n";
    cout << "  N     F(N) computed    F(N) actual    Error\n";
    cout << string(55, '-') << "\n";
    
    for (int N : {1,2,3,5,8,13,21,34,55}) {
        double phi_N = pow(phi, N), psi_N = pow(psi, N);
        double fib_computed = round((phi_N - psi_N) / sqrt(5.0));
        long long fib_actual = 0;
        long long a=0, b=1;
        for(int i=0;i<N;i++){long long t=b;b=a+b;a=t;}
        fib_actual = a;
        
        cout << setw(4) << N 
             << setw(18) << fixed << setprecision(0) << fib_computed
             << setw(14) << fib_actual
             << setw(14) << abs(fib_computed-fib_actual) << "\n";
    }
    
    cout << "\n  Binary exponentiation requires O(log N) multiplications\n";
    cout << "  vs O(N) for direct iteration — but needs depth management\n\n";
    return 0;
}
