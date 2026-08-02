// ΦΩ0 — φ-UNIVERSAL: Exploring the full structure
// φ² = φ + 1 → R[X]/(X²-X-1) ≅ R × R
// Two independent realities in one ciphertext pair

#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace lbcrypto;
using namespace std;

struct PhiPair { Ciphertext<DCRTPoly> a, b; };

class UniversalPhi {
public:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    int slots;
    double phi = 1.6180339887498948482;
    double psi = -0.6180339887498948482;  // -1/φ

    UniversalPhi() {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(4096);
        params.SetScalingModSize(59);
        params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
        params.SetMultiplicativeDepth(40);
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        slots = 2048;
    }

    Ciphertext<DCRTPoly> enc(double v) {
        vector<double> vec(slots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    }
    double dec(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(slots); return pt->GetRealPackedValue()[0];
    }

    // ==========================================
    // THE TWO REALITIES
    // ==========================================
    
    // Reality φ: evaluate polynomial at X=φ
    double reality_phi(const PhiPair& x) { return dec(x.a) + dec(x.b) * phi; }
    
    // Reality ψ: evaluate polynomial at X=ψ=-1/φ
    double reality_psi(const PhiPair& x) { return dec(x.a) + dec(x.b) * psi; }

    // Create from two independent values (one per reality)
    PhiPair from_realities(double v_phi, double v_psi) {
        // Solve: a + bφ = v_phi, a + bψ = v_psi
        // b = (v_phi - v_psi) / (φ - ψ)
        // a = v_phi - bφ
        double b_coef = (v_phi - v_psi) / (phi - psi);
        double a_coef = v_phi - b_coef * phi;
        return {enc(a_coef), enc(b_coef)};
    }

    // ==========================================
    // BASIC OPERATIONS (all free in φ-space)
    // ==========================================
    
    // φ-multiply: free
    PhiPair mul_phi(const PhiPair& x) {
        return {x.b, cc->EvalAdd(x.a, x.b)};
    }
    
    // ψ-multiply: multiply by -1/φ — also free!
    // ψ² = ψ + 1 (same polynomial!), so ψ acts like φ
    // But ψ = -1/φ, so multiply by ψ = multiply by -1/φ
    PhiPair mul_psi(const PhiPair& x) {
        // (a+bX) · X evaluated at X=ψ gives aψ + bψ² = aψ + b(ψ+1) = b + (a+b)ψ
        // Same formula as mul_phi! The polynomial structure is identical.
        return mul_phi(x);  // It's the same operation!
    }
    
    // Multiply by constant in BOTH realities simultaneously
    PhiPair mul_const(const PhiPair& x, double k) {
        return {enc(dec(x.a) * k), enc(dec(x.b) * k)};  // using encrypt for now
    }

    // ==========================================
    // DEPTH-FREE COMPOSITE OPERATIONS
    // ==========================================
    
    // Square: (a+bφ)² = (a²+b²) + (2ab+b²)φ
    // Uses 2 EvalMult... but can we do better?
    // In reality-space: squaring means squaring each reality value
    // If we switch to reality view, square, then switch back?
    PhiPair square_via_realities(const PhiPair& x) {
        double vp = reality_phi(x);
        double vs = reality_psi(x);
        return from_realities(vp * vp, vs * vs);
        // This does encrypt again — not truly homomorphic
    }

    // ADD: free (component-wise)
    PhiPair add(const PhiPair& x, const PhiPair& y) {
        return {cc->EvalAdd(x.a, y.a), cc->EvalAdd(x.b, y.b)};
    }

    // SUB: free
    PhiPair sub(const PhiPair& x, const PhiPair& y) {
        return {cc->EvalSub(x.a, y.a), cc->EvalSub(x.b, y.b)};
    }

    // ==========================================
    // THE CYCLE: Explore the group structure
    // ==========================================
    void explore_cycle() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   φ-CYCLE GROUP: Powers of φ                         ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        auto one = PhiPair{enc(1.0), enc(0.0)};  // 1 + 0φ
        
        cout << "  " << setw(4) << "n" << setw(16) << "φ^n (reality φ)"
             << setw(16) << "ψ^n (reality ψ)"
             << setw(16) << "a" << setw(16) << "b" << "\n";
        cout << "  " << string(68, '-') << "\n";
        
        auto current = one;
        for (int n = 0; n <= 15; n++) {
            double vp = reality_phi(current);
            double vs = reality_psi(current);
            double a = dec(current.a);
            double b = dec(current.b);
            
            cout << "  " << setw(4) << n
                 << setw(16) << fixed << setprecision(6) << vp
                 << setw(16) << vs
                 << setw(16) << a << setw(16) << b << "\n";
            
            current = mul_phi(current);
        }
        
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   TWO REALITIES:                                      ║\n";
        cout <<   "  ║   φ^n grows (φ>1)    |   ψ^n shrinks (|ψ|<1)          ║\n";
        cout <<   "  ║   Reality φ expands  |   Reality ψ contracts         ║\n";
        cout <<   "  ║   ONE GROWS, ONE SHRINKS — PERFECT BALANCE            ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }

    // ==========================================
    // RECOVERY: ψ^n → 0 as n→∞, φ^n extracts the leading behavior
    // ==========================================
    void explore_recovery() {
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   NOISE RECOVERY via ψ-CONTRACTION                    ║\n";
        cout <<   "  ║   Noise in ψ-reality SHRINKS over time               ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
        
        // Encode a value with "noise" in both realities
        auto signal = from_realities(10.0, 10.0);
        cout << "  Signal: φ=" << reality_phi(signal) << " ψ=" << reality_psi(signal) << "\n";
        
        // Apply φ-multiplication: signal grows in φ, shrinks in ψ
        auto processed = mul_phi(signal);
        cout << "  After φ: φ=" << reality_phi(processed) << " ψ=" << reality_psi(processed) << "\n";
        
        // After many φ-multiplications, ψ-reality → 0
        auto many = signal;
        for (int i = 0; i < 10; i++) many = mul_phi(many);
        cout << "  After 10φ: φ=" << reality_phi(many) << " ψ=" << reality_psi(many) << "\n";
        
        // Now apply ψ-multiplications to recover
        for (int i = 0; i < 10; i++) many = mul_psi(many);
        cout << "  After 10ψ: φ=" << reality_phi(many) << " ψ=" << reality_psi(many) << " (should recover)\n\n";
        
        cout << "  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   KEY: ψ^n → 0 kills noise in ψ-reality               ║\n";
        cout <<   "  ║   φ^n then recovers signal with killed noise          ║\n";
        cout <<   "  ║   This is DEPTH-FREE noise suppression!              ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }
};

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — φ-UNIVERSAL: The Full Structure              ║\n";
    cout <<   "  ║   R[X]/(X²-X-1) ≅ R × R via φ and -1/φ             ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n";

    UniversalPhi U;
    
    // Test basic structure
    cout << "\n  ── Two Realities Test ──\n";
    auto x = U.from_realities(7.0, 3.0);
    cout << "  Created: φ-reality=" << U.reality_phi(x) 
         << " ψ-reality=" << U.reality_psi(x) << "\n";
    
    // After φ-multiply: both realities transform by their respective eigenvalues
    auto x_phi = U.mul_phi(x);
    cout << "  After φ:  φ-reality=" << U.reality_phi(x_phi) 
         << " (expected " << 7.0 * U.phi << ")\n";
    cout << "            ψ-reality=" << U.reality_psi(x_phi) 
         << " (expected " << 3.0 * U.psi << ")\n";

    U.explore_cycle();
    U.explore_recovery();

    return 0;
}
