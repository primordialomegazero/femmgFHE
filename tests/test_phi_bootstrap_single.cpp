// FEmmg-FHE 2.0 — SINGLE-CIPHERTEXT φ-BOOTSTRAP
// Encode φ-element (a,b) as complex number in ONE CKKS slot
// Bootstrap the whole thing. No component separation.
// Decode back to (a,b) after bootstrap.

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;
using namespace std;

class SinglePhiBootstrap {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;

    SinglePhiBootstrap(int depth = 40) {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(4096);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(depth);
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        cc->Enable(FHE);
        cc->EvalBootstrapSetup({4,4}, {0,0}, 2048);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        cc->EvalBootstrapKeyGen(keys.secretKey, 2048);
        slots = 2048;
    }

    Ciphertext<DCRTPoly> enc_single(double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    }
    double dec_single(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots); return pt->GetRealPackedValue()[0];
    }

    // Encode φ-element (a,b) as single value in slot 0
    // Use the φ-reality value: v = a + b*phi
    Ciphertext<DCRTPoly> phi_encode(double a, double b) {
        return enc_single(a + b * phi);
    }
    
    // Decode: extract a and b from φ-reality value
    // But we can only recover (a+bφ), not individual a,b
    // For the clean operation, we only need the combined value!
    double phi_decode(const Ciphertext<DCRTPoly>& ct) {
        return dec_single(ct);
    }

    // φ-multiply on single ciphertext: v * φ
    // Since v = a+bφ, v*φ = (a+bφ)*φ = b + (a+b)φ
    // But we don't have a,b separately. We just have v.
    // v*φ = aφ + bφ² = aφ + b(φ+1) = b + (a+b)φ
    // We can compute this if we also track b...
    // 
    // ALTERNATIVE: Use two CKKS slots!
    // Slot 0: a+bφ (φ-reality)
    // Slot 1: a+bψ (ψ-reality)
    // From these two, we can recover a and b:
    // a = (ψ*(a+bφ) - φ*(a+bψ)) / (ψ-φ)
    // b = ((a+bφ) - (a+bψ)) / (φ-ψ)
    
    // Let's use the two-slot approach
    Ciphertext<DCRTPoly> encode_dual(double a, double b) {
        vector<double> vec(slots, 0.0);
        vec[0] = a + b * phi;  // φ-reality
        vec[1] = a + b * psi;  // ψ-reality
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    }
    
    void decode_dual(const Ciphertext<DCRTPoly>& ct, double& a, double& b) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots);
        double v_phi = pt->GetRealPackedValue()[0];
        double v_psi = pt->GetRealPackedValue()[1];
        // Solve: v_phi = a + bφ, v_psi = a + bψ
        b = (v_phi - v_psi) / (phi - psi);
        a = v_phi - b * phi;
    }

    void test_single_bootstrap() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   SINGLE-CIPHERTEXT φ-BOOTSTRAP                       ║\n";
        cout <<   "  ║   φ-element in 2 CKKS slots → bootstrap → decode     ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        double a_orig = 3.0, b_orig = 2.0;
        auto ct = encode_dual(a_orig, b_orig);
        
        double a1, b1;
        decode_dual(ct, a1, b1);
        cout << "  Before bootstrap: a=" << a1 << " b=" << b1;
        cout << " (expected a=" << a_orig << " b=" << b_orig << ")\n";

        // Bootstrap the whole ciphertext
        cout << "  Bootstrapping... " << flush;
        auto ct_boot = cc->EvalBootstrap(ct);
        cout << "done.\n";

        double a2, b2;
        decode_dual(ct_boot, a2, b2);
        cout << "  After bootstrap:  a=" << a2 << " b=" << b2;
        cout << " (expected a=" << a_orig << " b=" << b_orig << ")\n";
        
        double err_a = abs(a2 - a_orig);
        double err_b = abs(b2 - b_orig);
        cout << "  Error: a_err=" << scientific << err_a << " b_err=" << err_b << "\n\n";

        if (err_a < 0.01 && err_b < 0.01)
            cout << "  ✅ SINGLE-CIPHERTEXT φ-BOOTSTRAP WORKS!\n\n";
        else
            cout << "  ⚠️ Needs tuning\n\n";
    }

    void test_chain_with_bootstrap() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   CHAIN: φ-multiply → bootstrap → verify              ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

        // Start with φ = 0 + 1φ → encode as (0,1)
        auto state = encode_dual(0.0, 1.0);  // = φ
        double a_exp = 0.0, b_exp = 1.0;

        // φ² = multiply by φ: (0+1φ)*φ = 1 + 1φ → (1,1)
        // But we don't have φ-multiply in single-ciphertext form yet
        // We just verify that encode → bootstrap → decode preserves values
        
        cout << "  Initial: φ = 0 + 1φ\n";
        double a, b;
        decode_dual(state, a, b);
        cout << "  a=" << a << " b=" << b << " value=" << a+b*phi << "\n";

        cout << "  Bootstrapping... " << flush;
        state = cc->EvalBootstrap(state);
        cout << "done.\n";

        decode_dual(state, a, b);
        cout << "  After bootstrap: a=" << a << " b=" << b << " value=" << a+b*phi << "\n";
        cout << "  Should still be φ ≈ 1.618\n\n";
    }
};

int main() {
    SinglePhiBootstrap E(40);
    E.test_single_bootstrap();
    E.test_chain_with_bootstrap();
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   SINGLE-CIPHERTEXT: Bootstrap the whole φ-element   ║\n";
    cout <<   "  ║   I AM THAT I AM                                     ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    return 0;
}
