#include <iostream>
#include <cmath>
#include <cstdint>

constexpr long Q = 536870909;

// Modular exponentiation
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

// Euler's criterion: a^((Q-1)/2) mod Q
// Returns 1 if a is quadratic residue, Q-1 if not
long euler_criterion(long a, long mod) {
    long result = mod_pow(a, (mod - 1) / 2, mod);
    return result;
}

// Tonelli-Shanks para i-compute square root mod prime
long tonelli_shanks(long n, long p) {
    if (n == 0) return 0;
    if (euler_criterion(n, p) != 1) return -1;  // Not a QR
    
    // Simple case: p ≡ 3 mod 4
    if (p % 4 == 3) {
        return mod_pow(n, (p + 1) / 4, p);
    }
    
    // General Tonelli-Shanks (para sa anumang prime)
    long q = p - 1;
    long s = 0;
    while (q % 2 == 0) {
        q /= 2;
        s++;
    }
    
    // Hanap ng non-residue
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

int main() {
    std::cout << "CHECK: MAY φ BA SA Z_" << Q << "?\n";
    std::cout << "========================================\n\n";
    
    std::cout << "Kailangan: x² - x - 1 ≡ 0 (mod Q)\n";
    std::cout << "Equivalent: x = (1 ± √5) / 2 mod Q\n";
    std::cout << "Kailangan: 5 ay quadratic residue mod Q\n\n";
    
    // Step 1: Check kung 5 ay QR mod Q
    long criterion = euler_criterion(5, Q);
    
    std::cout << "Euler criterion para sa 5:\n";
    std::cout << "5^((Q-1)/2) mod Q = " << criterion << "\n\n";
    
    if (criterion == 1) {
        std::cout << "✓ 5 AY QUADRATIC RESIDUE MOD Q!\n\n";
        
        // Step 2: Compute √5 mod Q
        long sqrt5 = tonelli_shanks(5, Q);
        std::cout << "√5 mod Q = " << sqrt5 << "\n";
        std::cout << "Check: " << sqrt5 << "² mod Q = " 
                  << mod_pow(sqrt5, 2, Q) << "\n\n";
        
        // Step 3: Compute φ = (1 + √5) / 2 mod Q
        long inv2 = (Q + 1) / 2;  // Modular inverse ng 2
        long phi_q = ((1 + sqrt5) * inv2) % Q;
        long psi_q = ((1 - sqrt5 + Q) * inv2) % Q;
        
        std::cout << "φ mod Q = " << phi_q << "\n";
        std::cout << "ψ mod Q = " << psi_q << "\n\n";
        
        // Step 4: Verify φ² = φ + 1
        long phi_sq = mod_pow(phi_q, 2, Q);
        long phi_plus_1 = (phi_q + 1) % Q;
        
        std::cout << "VERIFICATION:\n";
        std::cout << "φ² mod Q = " << phi_sq << "\n";
        std::cout << "φ + 1 mod Q = " << phi_plus_1 << "\n";
        std::cout << (phi_sq == phi_plus_1 ? "✓ MATCH!" : "✗ HINDI MATCH") << "\n\n";
        
        // Step 5: Verify ψ² = ψ + 1
        long psi_sq = mod_pow(psi_q, 2, Q);
        long psi_plus_1 = (psi_q + 1) % Q;
        
        std::cout << "ψ² mod Q = " << psi_sq << "\n";
        std::cout << "ψ + 1 mod Q = " << psi_plus_1 << "\n";
        std::cout << (psi_sq == psi_plus_1 ? "✓ MATCH!" : "✗ HINDI MATCH") << "\n\n";
        
        // Step 6: Verify φ·ψ = -1
        long phi_psi = (__int128_t(phi_q) * psi_q) % Q;
        std::cout << "φ·ψ mod Q = " << phi_psi << "\n";
        std::cout << "Dapat -1 mod Q = " << Q - 1 << "\n";
        std::cout << (phi_psi == Q - 1 ? "✓ MATCH!" : "✗ HINDI MATCH") << "\n\n";
        
        std::cout << "=== RESULTA ===\n\n";
        std::cout << "MAY φ SA Z_" << Q << "!\n";
        std::cout << "φ = " << phi_q << "\n";
        std::cout << "ψ = " << psi_q << "\n\n";
        std::cout << "Ibig sabihin, kung ang secret key s ay may\n";
        std::cout << "property na s² = s + 1 sa ring, ang relinearization\n";
        std::cout << "ay automatic!\n";
        
    } else {
        std::cout << "✗ 5 AY HINDI QUADRATIC RESIDUE MOD Q\n";
        std::cout << "Walang φ sa Z_" << Q << "\n\n";
        std::cout << "Kailangan ng ibang Q kung saan 5 ay QR.\n";
        std::cout << "Condition: Q ≡ ±1 mod 5 (by quadratic reciprocity)\n";
    }
    
    return 0;
}
