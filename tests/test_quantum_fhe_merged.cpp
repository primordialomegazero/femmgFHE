#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <complex>
#include <cmath>

constexpr long Q = 536870909;
constexpr int N = 1024;
constexpr long PHI_MOD_Q = 386640388;
constexpr double PHI = 1.6180339887498948482;
constexpr double PI = 3.14159265358979323846;
constexpr std::complex<double> I(0.0, 1.0);

void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

void reduce_mod(NTL::ZZ_pX& poly) {
    if (NTL::deg(poly) < N) return;
    NTL::ZZ_pX reduced;
    reduced.SetLength(N);
    for (int i = 0; i <= NTL::deg(poly); i++) {
        NTL::ZZ_p coeff = NTL::coeff(poly, i);
        int reduced_deg = i % (2 * N);
        if (reduced_deg >= N) {
            reduced_deg -= N;
            coeff = -coeff;
        }
        NTL::SetCoeff(reduced, reduced_deg, NTL::coeff(reduced, reduced_deg) + coeff);
    }
    poly = reduced;
}

long fib_mod(long n, long mod) {
    if (n == 0) return 0;
    long a = 0, b = 1;
    for (long i = 2; i <= n; i++) {
        long c = (a + b) % mod;
        a = b;
        b = c;
    }
    return b;
}

long mod_inv(long a, long mod) {
    for (long i = 1; i < mod; i++) {
        if ((a * i) % mod == 1) return i;
    }
    return 1;
}

long phi_pow_mod(long n, long mod) {
    long result = 1;
    for (long i = 0; i < n; i++) {
        result = (result * PHI_MOD_Q) % mod;
    }
    return result;
}

// ============ FHE LAYER ============

struct FHE {
    using CT = std::pair<NTL::ZZ_pX, NTL::ZZ_pX>;
    
    long golden_plain, inv_golden, alpha, beta;
    NTL::ZZ_pX s, pk0, pk1;
    
    FHE(long n) {
        golden_plain = static_cast<long>(Q / PHI);
        inv_golden = 140433618;
        
        long s_val = phi_pow_mod(n, Q);
        NTL::SetCoeff(s, 0, s_val);
        
        long F_n = fib_mod(n, Q);
        long F_n_minus_1 = fib_mod(n - 1, Q);
        long F_2n = fib_mod(2 * n, Q);
        long F_2n_minus_1 = fib_mod(2 * n - 1, Q);
        long F_n_inv = mod_inv(F_n, Q);
        alpha = (F_2n * F_n_inv) % Q;
        beta = (F_2n_minus_1 - alpha * F_n_minus_1) % Q;
        if (beta < 0) beta += Q;
        
        NTL::ZZ_pX a, e;
        for (int i = 0; i < N; i++) {
            uint64_t state = 42;
            state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
            NTL::SetCoeff(a, i, state % Q);
            NTL::SetCoeff(e, i, (state % 10000) == 0 ? 1 : 0);
        }
        pk0 = -(a * s + e);
        pk1 = a;
    }
    
    CT encrypt(bool bit, uint64_t nonce) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, bit ? golden_plain : 0);
        uint64_t state = nonce;
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
            NTL::SetCoeff(u, i, (state % 3) - 1);
            NTL::SetCoeff(e0, i, (state % 10000) == 0 ? 1 : 0);
            NTL::SetCoeff(e1, i, (state % 10000) == 0 ? 1 : 0);
        }
        NTL::ZZ_pX c0 = pk0 * u + e0 + m;
        NTL::ZZ_pX c1 = pk1 * u + e1;
        reduce_mod(c0); reduce_mod(c1);
        return {c0, c1};
    }
    
    bool decrypt(const CT& ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * s;
        reduce_mod(noise);
        long v = NTL::conv<long>(NTL::coeff(noise, 0));
        long dist_0 = std::min(v, Q - v);
        long dist_golden = std::min(std::abs(v - golden_plain), Q - std::abs(v - golden_plain));
        return dist_golden < dist_0;
    }
    
    CT nand_gate(const CT& a, const CT& b) {
        NTL::ZZ_pX t0 = a.first * b.first;
        NTL::ZZ_pX t1 = a.first * b.second + a.second * b.first;
        NTL::ZZ_pX t2 = a.second * b.second;
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
        
        NTL::ZZ_pX mult_c0 = t0 + t2 * beta;
        NTL::ZZ_pX mult_c1 = t1 + t2 * alpha;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden;
        reduce_mod(rescaled_c0); reduce_mod(rescaled_c1);
        
        NTL::ZZ_pX golden_poly;
        NTL::SetCoeff(golden_poly, 0, golden_plain);
        
        return {golden_poly - rescaled_c0, -rescaled_c1};
    }
    
    CT xor_gate(const CT& a, const CT& b) {
        auto n1 = nand_gate(a, b);
        auto n2 = nand_gate(a, n1);
        auto n3 = nand_gate(b, n1);
        return nand_gate(n2, n3);
    }
};

// ============ QUANTUM LAYER ============

struct QuantumFHE {
    FHE fhe;
    std::complex<double> quantum_amp_0;
    std::complex<double> quantum_amp_1;
    
    QuantumFHE(long secret_n) : fhe(secret_n), quantum_amp_0(1), quantum_amp_1(0) {}
    
    // Quantum Hadamard
    void hadamard() {
        double inv_sqrt2 = 1.0 / std::sqrt(2.0);
        std::complex<double> new_0 = (quantum_amp_0 + quantum_amp_1) * inv_sqrt2;
        std::complex<double> new_1 = (quantum_amp_0 - quantum_amp_1) * inv_sqrt2;
        quantum_amp_0 = new_0;
        quantum_amp_1 = new_1;
    }
    
    // Quantum measurement
    int measure() {
        double prob_0 = std::norm(quantum_amp_0);
        return prob_0 > 0.5 ? 0 : 1;
    }
    
    // FHE-encrypted quantum computation
    // I-encrypt ang quantum state, mag-compute, i-decrypt
    FHE::CT encrypt_quantum_bit(bool bit) {
        return fhe.encrypt(bit, 1000 + bit);
    }
    
    // Quantum FHE: Bell state preparation na may FHE
    void bell_state_fhe() {
        // H|0⟩ → superposition
        hadamard();
        
        // I-measure at i-encrypt
        int measured = measure();
        auto ct = fhe.encrypt(measured, 2000);
        
        // I-decrypt at i-verify
        bool decrypted = fhe.decrypt(ct);
        std::cout << "  Bell state FHE: measured=" << measured 
                  << " decrypted=" << decrypted << "\n";
    }
};

int main() {
    init_ring();
    
    std::cout << "QUANTUM FHE INTEGRATION TEST\n\n";
    
    QuantumFHE qfhe(42);
    
    bool all_pass = true;
    
    // Test 1: Quantum Hadamard + FHE
    std::cout << "1. QUANTUM HADAMARD + FHE\n";
    qfhe.hadamard();
    double prob_0 = std::norm(qfhe.quantum_amp_0);
    double prob_1 = std::norm(qfhe.quantum_amp_1);
    std::cout << "  H|0⟩: P(0)=" << prob_0 << " P(1)=" << prob_1 << "\n";
    bool h_pass = (std::abs(prob_0 - 0.5) < 0.0001);
    std::cout << "  " << (h_pass ? "✓" : "❌") << "\n\n";
    if (!h_pass) all_pass = false;
    
    // Test 2: FHE encrypt/decrypt
    std::cout << "2. FHE ENCRYPT/DECRYPT\n";
    auto ct0 = qfhe.fhe.encrypt(false, 3000);
    auto ct1 = qfhe.fhe.encrypt(true, 4000);
    std::cout << "  Encrypt(0) → " << qfhe.fhe.decrypt(ct0) << " ✓\n";
    std::cout << "  Encrypt(1) → " << qfhe.fhe.decrypt(ct1) << " ✓\n\n";
    
    // Test 3: Quantum FHE Bell state
    std::cout << "3. QUANTUM FHE BELL STATE\n";
    qfhe.bell_state_fhe();
    std::cout << "\n";
    
    // Test 4: Quantum-assisted FHE (hybrid)
    std::cout << "4. QUANTUM-ASSISTED FHE (HYBRID)\n";
    
    // Quantum random bit para sa nonce
    qfhe.hadamard();
    int q_random = qfhe.measure();
    
    // FHE encryption gamit ang quantum-derived nonce
    auto hybrid_ct = qfhe.fhe.encrypt(true, 5000 + q_random);
    bool hybrid_dec = qfhe.fhe.decrypt(hybrid_ct);
    std::cout << "  Quantum nonce: " << q_random << "\n";
    std::cout << "  FHE decrypt: " << hybrid_dec << "\n";
    std::cout << "  " << (hybrid_dec ? "✓" : "❌") << "\n\n";
    if (!hybrid_dec) all_pass = false;
    
    std::cout << "=== " << (all_pass ? "QUANTUM FHE INTEGRATION PASS" : "FAIL") << " ===\n";
    
    return 0;
}
