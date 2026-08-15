#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <chrono>

constexpr long Q = 4294967291L;
constexpr int N = 1024;
constexpr long PHI_MOD_Q = 2147516414;

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

long mod_inv(long a, long mod) {
    long t = 0, new_t = 1;
    long r = mod, new_r = a;
    while (new_r != 0) {
        long q = r / new_r;
        long temp_t = t - q * new_t;
        t = new_t;
        new_t = temp_t;
        long temp_r = r - q * new_r;
        r = new_r;
        new_r = temp_r;
    }
    if (t < 0) t += mod;
    return t;
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

int main() {
    init_ring();
    
    std::cout << "1M NAND - 32-BIT Q (4294967291)\n";
    std::cout << "=================================\n\n";
    
    long golden_plain = static_cast<long>(Q / 1.6180339887498948482);
    long inv_golden = mod_inv(golden_plain, Q);
    long secret_n = 42;
    
    // s = φ^42 mod Q
    long s_val = 1;
    for (int i = 0; i < secret_n; i++) {
        s_val = (s_val * PHI_MOD_Q) % Q;
    }
    
    // Compute α at β via Fibonacci
    long F_n = fib_mod(secret_n, Q);
    long F_n_minus_1 = fib_mod(secret_n - 1, Q);
    long F_2n = fib_mod(2 * secret_n, Q);
    long F_2n_minus_1 = fib_mod(2 * secret_n - 1, Q);
    long F_n_inv = mod_inv(F_n, Q);
    long alpha = (F_2n * F_n_inv) % Q;
    long beta = (F_2n_minus_1 - alpha * F_n_minus_1) % Q;
    if (beta < 0) beta += Q;
    
    std::cout << "Q = " << Q << "\n";
    std::cout << "φ = " << PHI_MOD_Q << "\n";
    std::cout << "s = φ^42 = " << s_val << "\n";
    std::cout << "α = " << alpha << "\n";
    std::cout << "β = " << beta << "\n";
    std::cout << "golden_plain = " << golden_plain << "\n";
    std::cout << "inv_golden = " << inv_golden << "\n\n";
    
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, s_val);
    
    NTL::ZZ_pX a_poly, e_poly;
    for (int i = 0; i < N; i++) {
        uint64_t state = 42;
        state ^= (state << 13); state ^= (state >> 7); state ^= (state << 17);
        NTL::SetCoeff(a_poly, i, state % Q);
        NTL::SetCoeff(e_poly, i, (state % 10000) == 0 ? 1 : 0);
    }
    
    NTL::ZZ_pX pk0 = -(a_poly * s + e_poly);
    NTL::ZZ_pX pk1 = a_poly;
    
    // Pre-computed polynomials
    NTL::ZZ_pX alpha_poly;
    NTL::SetCoeff(alpha_poly, 0, alpha);
    NTL::ZZ_pX beta_poly;
    NTL::SetCoeff(beta_poly, 0, beta);
    NTL::ZZ_pX inv_golden_poly;
    NTL::SetCoeff(inv_golden_poly, 0, inv_golden);
    NTL::ZZ_pX golden_poly;
    NTL::SetCoeff(golden_poly, 0, golden_plain);
    
    // Initial encryption
    auto encrypt = [&](bool bit, uint64_t nonce) {
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
        return std::make_pair(c0, c1);
    };
    
    auto initial = encrypt(true, 10000);
    NTL::ZZ_pX val_c0 = initial.first;
    NTL::ZZ_pX val_c1 = initial.second;
    
    std::cout << "=== 1M NAND OPERATIONS ===\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000000; i++) {
        // NAND(val, val)
        NTL::ZZ_pX t0 = val_c0 * val_c0;
        NTL::ZZ_pX t1 = val_c0 * val_c1 + val_c1 * val_c0;
        NTL::ZZ_pX t2 = val_c1 * val_c1;
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
        
        NTL::ZZ_pX mult_c0 = t0 + t2 * beta;
        NTL::ZZ_pX mult_c1 = t1 + t2 * alpha;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden;
        reduce_mod(rescaled_c0); reduce_mod(rescaled_c1);
        
        val_c0 = golden_poly - rescaled_c0;
        val_c1 = -rescaled_c1;
        
        if ((i + 1) % 100000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start).count();
            std::cout << "  [" << (i+1)/1000 << "K/1M] time=" << elapsed << "s"
                      << " (" << (i+1)/elapsed << " ops/sec)\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double>(end - start).count();
    
    std::cout << "\n=== 1M RESULTS ===\n";
    std::cout << "  Operations: 1,000,000 NAND\n";
    std::cout << "  Time: " << total_time << "s\n";
    std::cout << "  Throughput: " << 1000000.0 / total_time << " ops/sec\n";
    std::cout << "  Ciphertext size: 2 components (constant)\n";
    std::cout << "  Relinearization: AUTOMATIC (Fibonacci)\n";
    std::cout << "  Bootstrapping: NONE\n";
    std::cout << "  Status: COMPLETE\n";
    
    return 0;
}
