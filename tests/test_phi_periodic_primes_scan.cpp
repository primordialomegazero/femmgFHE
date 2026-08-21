// φ-PERIODIC PRIMES — FULL SCAN
// Hanapin LAHAT ng primes na may maliit na φ-period
// para sa natural na modulus recycling
//
// ANG TARGET: primes na may period ≤ 20
// (para practical na level recycling every 20 gates max)

#include <iostream>
#include <vector>
#include <cmath>

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-PERIODIC PRIMES — FULL SCAN\n";
    std::cout << "  Target: Period ≤ 20\n";
    std::cout << "========================================\n\n";

    // I-scan ang lahat ng primes hanggang 1000
    std::vector<int> small_period_primes;
    std::vector<int> periods;

    for (int p = 2; p <= 1000; p++) {
        // I-check kung prime
        bool is_prime = true;
        for (int d = 2; d * d <= p; d++) {
            if (p % d == 0) { is_prime = false; break; }
        }
        if (!is_prime) continue;

        // Check kung may √5 mod p
        int sqrt5 = -1;
        for (int x = 0; x < p; x++) {
            if ((x * x) % p == 5 % p) {
                sqrt5 = x;
                break;
            }
        }
        if (sqrt5 == -1) continue;

        // Compute φ mod p
        long long inv2 = (p + 1) / 2;
        long long phi_mod = ((1 + sqrt5) * inv2) % p;

        // Find period: φ^n ≡ 1 (mod p)
        long long period = 1;
        long long current = phi_mod;
        while (current != 1 && period <= 50) {
            current = (current * phi_mod) % p;
            period++;
        }

        if (period <= 20) {
            small_period_primes.push_back(p);
            periods.push_back(period);
        }
    }

    std::cout << "Primes na may φ-period ≤ 20 (hanggang 1000):\n\n";
    std::cout << "  Prime | Period | φ mod p\n";
    std::cout << "  ------|--------|--------\n";

    for (size_t i = 0; i < small_period_primes.size(); i++) {
        int p = small_period_primes[i];
        long long sqrt5 = -1;
        for (int x = 0; x < p; x++) {
            if ((x * x) % p == 5 % p) { sqrt5 = x; break; }
        }
        long long inv2 = (p + 1) / 2;
        long long phi_mod = ((1 + sqrt5) * inv2) % p;
        std::cout << "  " << p << "    | " << periods[i] << "     | " << phi_mod << "\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  KEY FINDINGS:\n";
    std::cout << "  - Prime na may pinakamaliit na period\n";
    std::cout << "    ay ideal para sa modulus recycling\n";
    std::cout << "  - Period 4 = recycle every 4 gates\n";
    std::cout << "  - Period 9 = recycle every 9 gates\n";
    std::cout << "========================================\n";

    return 0;
}
