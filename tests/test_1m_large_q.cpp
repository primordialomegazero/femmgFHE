#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <chrono>

// 61-bit prime na Q ≡ 1 mod 5 (may φ)
constexpr long long Q = 1152921504606847081LL;
constexpr int N = 1024;
constexpr double PHI = 1.6180339887498948482;

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

// Modular exponentiation para sa malaking Q
long long mod_pow(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = (__int128_t(result) * base) % mod;
        base = (__int128_t(base) * base) % mod;
        exp >>= 1;
    }
    return result;
}

// Fibonacci mod Q (fast doubling para sa malaking Q)
long long fib_mod(long long n, long long mod) {
    if (n == 0) return 0;
    long long a = 0, b = 1;
    for (long long i = 2; i <= n; i++) {
        long long c = (a + b) % mod;
        a = b;
        b = c;
    }
    return b;
}

// Modular inverse
long long mod_inv(long long a, long long mod) {
    // Extended Euclidean Algorithm
    long long t = 0, new_t = 1;
    long long r = mod, new_r = a;
    while (new_r != 0) {
        long long q = r / new_r;
        long long temp_t = t - q * new_t;
        t = new_t;
        new_t = temp_t;
        long long temp_r = r - q * new_r;
        r = new_r;
        new_r = temp_r;
    }
    if (r > 1) return -1;  // No inverse
    if (t < 0) t += mod;
    return t;
}

// Hanapin ang φ sa Z_Q
long long find_phi(long long Q) {
    // Tonelli-Shanks para sa sqrt(5)
    long long sqrt5 = -1;
    for (long long i = 1; i < 1000000; i++) {
        if ((i * i) % Q == 5) {
            sqrt5 = i;
            break;
        }
    }
    if (sqrt5 < 0) return -1;
    
    long long inv2 = mod_inv(2, Q);
    return ((1 + sqrt5) * inv2) % Q;
}

int main() {
    init_ring();
    
    std::cout << "1M NAND OPERATIONS - 61-BIT Q\n";
    std::cout << "Q = " << Q << "\n\n";
    
    long long phi_q = find_phi(Q);
    std::cout << "φ = " << phi_q << "\n";
    
    // Verify φ² = φ+1
    long long phi_sq = mod_pow(phi_q, 2, Q);
    long long phi_plus_1 = (phi_q + 1) % Q;
    std::cout << "φ² = " << phi_sq << "\n";
    std::cout << "φ+1 = " << phi_plus_1 << "\n";
    std::cout << "Match: " << (phi_sq == phi_plus_1 ? "YES ✓" : "NO ✗") << "\n\n";
    
    // Secret n
    long long secret_n = 42;
    long long s_val = mod_pow(phi_q, secret_n, Q);
    
    // Compute α at β
    long long F_n = fib_mod(secret_n, Q);
    long long F_n_minus_1 = fib_mod(secret_n - 1, Q);
    long long F_2n = fib_mod(2 * secret_n, Q);
    long long F_2n_minus_1 = fib_mod(2 * secret_n - 1, Q);
    long long F_n_inv = mod_inv(F_n, Q);
    long long alpha = (F_2n * F_n_inv) % Q;
    long long beta = (F_2n_minus_1 - alpha * F_n_minus_1) % Q;
    if (beta < 0) beta += Q;
    
    std::cout << "α = " << alpha << "\n";
    std::cout << "β = " << beta << "\n\n";
    
    // Golden plain at inverse
    long long golden_plain = static_cast<long long>(Q / PHI);
    long long inv_golden = mod_inv(golden_plain, Q);
    
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
    
    // Initial encrypt
    auto initial = encrypt(true, 10000);
    NTL::ZZ_pX val_c0 = initial.first;
    NTL::ZZ_pX val_c1 = initial.second;
    
    // Pre-computed polynomials para sa speed
    NTL::ZZ_pX alpha_poly;
    NTL::SetCoeff(alpha_poly, 0, alpha);
    NTL::ZZ_pX beta_poly;
    NTL::SetCoeff(beta_poly, 0, beta);
    NTL::ZZ_pX inv_golden_poly;
    NTL::SetCoeff(inv_golden_poly, 0, inv_golden);
    NTL::ZZ_pX golden_poly;
    NTL::SetCoeff(golden_poly, 0, golden_plain);
    
    auto nand_gate_fast = [&](NTL::ZZ_pX& a_c0, NTL::ZZ_pX& a_c1,
                               NTL::ZZ_pX& b_c0, NTL::ZZ_pX& b_c1) {
        NTL::ZZ_pX t0 = a_c0 * b_c0;
        NTL::ZZ_pX t1 = a_c0 * b_c1 + a_c1 * b_c0;
        NTL::ZZ_pX t2 = a_c1 * b_c1;
        reduce_mod(t0); reduce_mod(t1); reduce_mod(t2);
        
        NTL::ZZ_pX mult_c0 = t0 + t2 * beta;
        NTL::ZZ_pX mult_c1 = t1 + t2 * alpha;
        reduce_mod(mult_c0); reduce_mod(mult_c1);
        
        NTL::ZZ_pX rescaled_c0 = mult_c0 * inv_golden;
        NTL::ZZ_pX rescaled_c1 = mult_c1 * inv_golden;
        reduce_mod(rescaled_c0); reduce_mod(rescaled_c1);
        
        return std::make_pair(golden_poly - rescaled_c0, -rescaled_c1);
    };
    
    std::cout << "=== 1M NAND OPERATIONS ===\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000000; i++) {
        auto result = nand_gate_fast(val_c0, val_c1, val_c0, val_c1);
        val_c0 = result.first;
        val_c1 = result.second;
        
        if ((i + 1) % 100000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start).count();
            std::cout << "  [" << (i+1)/1000 << "K/1M] time=" << elapsed << "s\n";
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double>(end - start).count();
    
    // Verify final result
    NTL::ZZ_pX noise = val_c0 + val_c1 * s;
    reduce_mod(noise);
    NTL::ZZ_p v_zz = NTL::coeff(noise, 0);
    long long v = NTL::rep(v_zz);
    
    // Distance-based decryption
    long long dist_0 = std::min(v, Q - v);
    long long dist_golden = std::min(std::abs(v - golden_plain), Q - std::abs(v - golden_plain));
    bool final_bit = dist_golden < dist_0;
    
    std::cout << "\n=== 1M RESULTS (61-BIT Q) ===\n";
    std::cout << "  Operations: 1,000,000 NAND\n";
    std::cout << "  Time: " << total_time << "s\n";
    std::cout << "  Final noise: " << v << "\n";
    std::cout << "  Final bit: " << final_bit << " (expected 1 kasi 1M ay even)\n";
    std::cout << "  Status: " << (final_bit ? "PASS ✓" : "FAIL ❌") << "\n";
    
    return 0;
}
