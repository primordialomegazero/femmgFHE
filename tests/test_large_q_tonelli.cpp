#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <vector>

// Tonelli-Shanks para sa modular square root
long mod_pow(long base, long exp, long mod) {
    long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = (__int128_t(result) * base) % mod;
        base = (__int128_t(base) * base) % mod;
        exp >>= 1;
    }
    return result;
}

long euler_criterion(long a, long mod) {
    return mod_pow(a, (mod - 1) / 2, mod);
}

long tonelli_shanks(long n, long p) {
    if (n == 0) return 0;
    if (euler_criterion(n, p) != 1) return -1;
    
    // Simple case: p ≡ 3 mod 4
    if (p % 4 == 3) {
        return mod_pow(n, (p + 1) / 4, p);
    }
    
    // General Tonelli-Shanks
    long q = p - 1;
    long s = 0;
    while (q % 2 == 0) {
        q /= 2;
        s++;
    }
    
    long z = 2;
    while (euler_criterion(z, p) != p - 1) z++;
    
    long m = s;
    long c = mod_pow(z, q, p);
    long t = mod_pow(n, q, p);
    long r = mod_pow(n, (q + 1) / 2, p);
    
    while (t != 1) {
        long i = 0;
        long temp = t;
        while (temp != 1) {
            temp = (__int128_t(temp) * temp) % p;
            i++;
            if (i >= m) return -1;
        }
        
        long b = mod_pow(c, 1LL << (m - i - 1), p);
        r = (__int128_t(r) * b) % p;
        c = (__int128_t(b) * b) % p;
        t = (__int128_t(t) * c) % p;
        m = i;
    }
    
    return r;
}

// Maghanap ng φ sa Z_Q
long find_phi(long Q) {
    // Check kung 5 ay QR
    if (euler_criterion(5 % Q, Q) != 1) return -1;
    
    long sqrt5 = tonelli_shanks(5 % Q, Q);
    if (sqrt5 < 0) return -1;
    
    long inv2 = (Q + 1) / 2;
    long phi = ((1 + sqrt5) * inv2) % Q;
    return phi;
}

int main() {
    std::cout << "LARGE Q TEST (TONELLI-SHANKS)\n\n";
    
    std::vector<long> q_values = {
        536870909,           // 29 bits
        2147483647,          // 31 bits
        4294967291,          // 32 bits
        1000000007,          // 30 bits
        999999937,           // 30 bits
        (1LL << 31) - 1,     // 31 bits (Mersenne)
        (1LL << 61) - 1      // 61 bits (Mersenne)
    };
    
    for (long Q : q_values) {
        std::cout << "Q = " << Q << " (" << (int)std::log2(Q) << " bits)\n";
        
        long phi = find_phi(Q);
        
        if (phi > 0) {
            long phi_sq = (__int128_t(phi) * phi) % Q;
            long phi_plus_1 = (phi + 1) % Q;
            std::cout << "  φ = " << phi << "\n";
            std::cout << "  φ² = " << phi_sq << "\n";
            std::cout << "  φ+1 = " << phi_plus_1 << "\n";
            std::cout << "  Match: " << (phi_sq == phi_plus_1 ? "YES ✓" : "NO ✗") << "\n";
        } else {
            std::cout << "  5 ay hindi QR sa Z_" << Q << "\n";
        }
        std::cout << "\n";
    }
    
    return 0;
}
