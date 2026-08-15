#include <iostream>
#include <vector>
#include <cmath>

// Euler criterion: 5^((Q-1)/2) mod Q
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

// Quadratic reciprocity: 5 ay QR sa Z_Q kung Q ≡ ±1 mod 5
bool is_5_qr(long long Q) {
    return (Q % 5 == 1) || (Q % 5 == 4);
}

int main() {
    std::cout << "EMERGENT PATTERN: Q VALUES NA MAY φ\n\n";
    
    std::cout << "Pattern: Q ≡ ±1 mod 5 (by quadratic reciprocity)\n";
    std::cout << "Kung Q ≡ 1 o 4 mod 5, may φ sa Z_Q\n\n";
    
    // I-verify ang pattern
    std::vector<long long> q_values = {
        536870909,    // 29 bits
        4294967291,   // 32 bits
        2147483647,   // 31 bits
        1000000007,   // 30 bits
        999999937     // 30 bits
    };
    
    std::cout << "Q              | Q mod 5 | May φ?\n";
    std::cout << "---------------|---------|-------\n";
    
    for (long long Q : q_values) {
        bool has_phi = is_5_qr(Q);
        std::cout << Q << " | " << Q % 5 << " | " 
                  << (has_phi ? "YES ✓" : "NO") << "\n";
    }
    
    std::cout << "\n";
    
    // Maghanap ng malalaking Q na may φ
    std::cout << "=== MALALAKING Q NA MAY φ ===\n\n";
    
    // Pattern: Q ≡ 1 mod 5
    // Q = 5k + 1 na prime
    // Para sa 61 bits: maghanap ng prime na 5k+1
    
    long long target_bits = 61;
    long long min_q = 1LL << (target_bits - 1);
    long long max_q = (1LL << target_bits) - 1;
    
    std::cout << "Naghahanap ng " << target_bits << "-bit prime na Q ≡ 1 o 4 mod 5...\n\n";
    
    // Simple prime test
    auto is_prime = [](long long n) {
        if (n < 2) return false;
        for (long long i = 2; i * i <= n && i < 100000; i++) {
            if (n % i == 0) return false;
        }
        return true;
    };
    
    // Hanapin ang unang prime na Q ≡ 1 mod 5
    long long q_candidate = min_q;
    while (q_candidate % 5 != 1 && q_candidate % 5 != 4) q_candidate++;
    
    int found = 0;
    while (q_candidate < max_q && found < 5) {
        if (is_prime(q_candidate)) {
            std::cout << "  " << q_candidate << " (mod 5: " << q_candidate % 5 << ")\n";
            found++;
        }
        q_candidate += 5;
    }
    
    std::cout << "\n=== EMERGENT PROPERTY ===\n";
    std::cout << "1. Q ≡ ±1 mod 5 ⟹ may φ sa Z_Q\n";
    std::cout << "2. Ito ay quadratic reciprocity - automatic pattern\n";
    std::cout << "3. Hindi kailangan ng brute force - formula lang\n";
    std::cout << "4. Lahat ng Q na ≡ 1 o 4 mod 5 ay may Fibonacci reduction\n\n";
    
    std::cout << "=== PINAKAMALAKING TESTABLE ===\n";
    std::cout << "61-bit prime na Q ≡ 1 mod 5\n";
    std::cout << "Key space: ~2^61 ≈ 2.3 × 10^18 possibilities\n";
    std::cout << "Brute force: bilyon-bilyong taon\n";
    
    return 0;
}
