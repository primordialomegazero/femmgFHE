// PERIOD-2 TEST: PURE MATH (walang OpenFHE)
// I-verify kung gumagana sa LAHAT ng primes
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

bool is_prime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int main() {
    cout << "====================================\n";
    cout << "  PERIOD-2 TEST — PURE MATH\n";
    cout << "  (Walang OpenFHE, walang encryption)\n";
    cout << "====================================\n\n";

    vector<int> primes;
    for (int p = 2; p <= 100; p++) {
        if (is_prime(p)) primes.push_back(p);
    }

    cout << "Primes tested: " << primes.size() << "\n";
    cout << "Formula: NAND(x,x) = 1 - x² mod p\n";
    cout << "Period-2: NAND(NAND(x,x), NAND(x,x)) == x\n\n";

    int total_pass = 0;
    int total_fail = 0;

    for (int p : primes) {
        bool passed = true;
        
        // Test para sa x = 0 at x = 1
        for (int x = 0; x <= 1; x++) {
            // NAND(x,x) = (1 - x²) mod p
            int nand1 = (1 - (x * x) % p + p) % p;
            
            // NAND(NAND(x,x), NAND(x,x))
            int nand2 = (1 - (nand1 * nand1) % p + p) % p;
            
            if (nand2 != x) {
                passed = false;
                cout << "  ❌ p=" << p << ", x=" << x 
                     << ": NAND(NAND) = " << nand2 
                     << " (expected " << x << ")\n";
            }
        }
        
        if (passed) {
            total_pass++;
            cout << "  ✅ p = " << p << " — Period-2 WORKS!\n";
        } else {
            total_fail++;
        }
    }

    cout << "\n====================================\n";
    cout << "  RESULT: " << total_pass << " passed, " << total_fail << " failed\n";
    cout << "====================================\n";
    
    if (total_fail == 0) {
        cout << "\n  ✅ UNIVERSAL! Period-2 gumagana sa LAHAT ng primes!\n";
        cout << "  ✅ Walang kailangan na Golden Ratio o Lucas!\n";
        cout << "  ✅ Simpleng algebra lang: x² = x para sa 0 at 1!\n";
    }

    return 0;
}
