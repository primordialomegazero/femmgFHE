#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

constexpr long Q = 536870909;
constexpr int N = 1024;
constexpr long PHI_MOD_Q = 386640388;
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

struct FHE {
    long golden_plain;
    long threshold;
    long inv_golden;
    long alpha;
    long beta;
    NTL::ZZ_pX s;
    NTL::ZZ_pX pk0;
    NTL::ZZ_pX pk1;
    
    FHE(long n) {
        golden_plain = static_cast<long>(Q / PHI);
        threshold = static_cast<long>(Q / (2 * PHI));
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
    
    auto encrypt(bool bit, uint64_t nonce) {
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
    }
    
    bool decrypt(const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * s;
        reduce_mod(noise);
        long v = NTL::conv<long>(NTL::coeff(noise, 0));
        return v > threshold;
    }
    
    auto nand_gate(const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& a,
                    const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& b) {
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
        
        return std::make_pair(golden_poly - rescaled_c0, -rescaled_c1);
    }
    
    auto not_gate(const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& a) {
        return nand_gate(a, a);
    }
    
    auto and_gate(const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& a,
                   const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& b) {
        auto nand_ab = nand_gate(a, b);
        return nand_gate(nand_ab, nand_ab);
    }
    
    auto or_gate(const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& a,
                  const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& b) {
        auto not_a = not_gate(a);
        auto not_b = not_gate(b);
        return nand_gate(not_a, not_b);
    }
    
    auto xor_gate(const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& a,
                   const std::pair<NTL::ZZ_pX, NTL::ZZ_pX>& b) {
        auto nand_ab = nand_gate(a, b);
        auto nand_a_ab = nand_gate(a, nand_ab);
        auto nand_b_ab = nand_gate(b, nand_ab);
        return nand_gate(nand_a_ab, nand_b_ab);
    }
};

int main() {
    init_ring();
    
    std::cout << "ALL GATES TEST - FIBONACCI FHE\n\n";
    
    FHE fhe(42);
    
    bool all_pass = true;
    
    // NAND test
    std::cout << "NAND:\n";
    for (int i = 0; i < 4; i++) {
        bool a = (i >> 1) & 1;
        bool b = i & 1;
        auto ca = fhe.encrypt(a, 100 + i * 10);
        auto cb = fhe.encrypt(b, 200 + i * 10);
        auto result = fhe.nand_gate(ca, cb);
        bool got = fhe.decrypt(result);
        bool expected = !(a && b);
        std::cout << "  " << a << " NAND " << b << " = " << got << " (exp " << expected << ")"
                  << (got == expected ? " ✓" : " ❌") << "\n";
        if (got != expected) all_pass = false;
    }
    
    // AND test
    std::cout << "\nAND:\n";
    for (int i = 0; i < 4; i++) {
        bool a = (i >> 1) & 1;
        bool b = i & 1;
        auto ca = fhe.encrypt(a, 300 + i * 10);
        auto cb = fhe.encrypt(b, 400 + i * 10);
        auto result = fhe.and_gate(ca, cb);
        bool got = fhe.decrypt(result);
        bool expected = a && b;
        std::cout << "  " << a << " AND " << b << " = " << got << " (exp " << expected << ")"
                  << (got == expected ? " ✓" : " ❌") << "\n";
        if (got != expected) all_pass = false;
    }
    
    // OR test
    std::cout << "\nOR:\n";
    for (int i = 0; i < 4; i++) {
        bool a = (i >> 1) & 1;
        bool b = i & 1;
        auto ca = fhe.encrypt(a, 500 + i * 10);
        auto cb = fhe.encrypt(b, 600 + i * 10);
        auto result = fhe.or_gate(ca, cb);
        bool got = fhe.decrypt(result);
        bool expected = a || b;
        std::cout << "  " << a << " OR " << b << " = " << got << " (exp " << expected << ")"
                  << (got == expected ? " ✓" : " ❌") << "\n";
        if (got != expected) all_pass = false;
    }
    
    // XOR test
    std::cout << "\nXOR:\n";
    for (int i = 0; i < 4; i++) {
        bool a = (i >> 1) & 1;
        bool b = i & 1;
        auto ca = fhe.encrypt(a, 700 + i * 10);
        auto cb = fhe.encrypt(b, 800 + i * 10);
        auto result = fhe.xor_gate(ca, cb);
        bool got = fhe.decrypt(result);
        bool expected = a ^ b;
        std::cout << "  " << a << " XOR " << b << " = " << got << " (exp " << expected << ")"
                  << (got == expected ? " ✓" : " ❌") << "\n";
        if (got != expected) all_pass = false;
    }
    
    // NOT test
    std::cout << "\nNOT:\n";
    for (int i = 0; i < 2; i++) {
        auto ca = fhe.encrypt(i, 900 + i * 10);
        auto result = fhe.not_gate(ca);
        bool got = fhe.decrypt(result);
        bool expected = !i;
        std::cout << "  NOT " << i << " = " << got << " (exp " << expected << ")"
                  << (got == expected ? " ✓" : " ❌") << "\n";
        if (got != expected) all_pass = false;
    }
    
    std::cout << "\n=== FINAL: " << (all_pass ? "ALL GATES PASS ✓" : "SOME GATES FAIL ❌") << " ===\n";
    
    return 0;
}
