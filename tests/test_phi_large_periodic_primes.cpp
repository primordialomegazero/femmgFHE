// φ-LARGE PERIODIC PRIMES
// Hanapin ang mas malalaking primes na may maliit na φ-period
// para sa security at recycling
//
// ANG TARGET: 128-bit primes na may φ-period ≤ 20

#include <NTL/ZZ.h>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-LARGE PERIODIC PRIMES\n";
    std::cout << "  Security + Recycling\n";
    std::cout << "========================================\n\n";

    // Ang φ-period ay related sa Fibonacci/Lucas numbers
    // φ^n ≡ 1 (mod p) ay may period na nagdi-divide sa p-1
    // o p+1 (depende kung ang 5 ay quadratic residue o hindi)
    
    // Para sa malalaking primes, ang period ay malaki
    // pero may "small period" sa φ² = φ+1 relation
    
    std::cout << "ANG RELATION SA FIBONACCI:\n";
    std::cout << "==========================\n\n";
    std::cout << "  φ^n = F(n)φ + F(n-1)\n";
    std::cout << "  Kaya φ^n ≡ 1 (mod p) kapag:\n";
    std::cout << "    F(n) ≡ 0 (mod p) at F(n-1) ≡ 1 (mod p)\n\n";
    
    std::cout << "  Ang period ng φ mod p ay ang\n";
    std::cout << "  'Pisano period' ng p.\n\n";
    
    // Mga primes na may maliit na Pisano period
    std::cout << "PRIMES NA MAY MALIIT NA PISANO PERIOD:\n";
    std::cout << "======================================\n\n";
    
    std::cout << "  Prime | Pisano Period | φ-period\n";
    std::cout << "  ------|---------------|---------\n";
    std::cout << "  5     | 20            | 4\n";
    std::cout << "  11    | 10            | 10\n";
    std::cout << "  19    | 18            | 9\n";
    std::cout << "  29    | 14            | 14\n";
    std::cout << "  31    | 30            | 15\n\n";

    std::cout << "ANG KEY INSIGHT:\n";
    std::cout << "================\n\n";
    std::cout << "  Ang φ-period ay kalahati ng Pisano period.\n";
    std::cout << "  Para sa malalaking primes, kailangan hanapin\n";
    std::cout << "  ang primes na may maliit na Pisano period.\n\n";
    
    std::cout << "  PERO: May mas magandang approach!\n";
    std::cout << "  Sa halip na maghanap ng rare primes,\n";
    std::cout << "  gamitin ang φ² = φ+1 bilang natural na\n";
    std::cout << "  level recycling sa KAHIT ANONG prime.\n\n";

    std::cout << "  ANG φ²-RECYCLING:\n";
    std::cout << "  - Bawat 2 operations, ang φ² = φ+1\n";
    std::cout << "  - Ito ay natural na 'increment' sa level\n";
    std::cout << "  - Hindi kailangan ng special prime!\n\n";

    return 0;
}
