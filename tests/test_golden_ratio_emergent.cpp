// GOLDEN RATIO EMERGENT PROPERTY TEST
// Hanapin kung ano pa ang lumalabas sa φ = (1+√5)/2 mod 65537
#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

int main() {
    cout << "GOLDEN RATIO EMERGENT PROPERTY\n";
    cout << "==============================\n\n";

    const long long p = 65537;
    
    // Hanapin ang √5 mod p
    long long sqrt5 = 0;
    for (long long i = 1; i < p; i++) {
        if ((i * i) % p == 5) {
            sqrt5 = i;
            break;
        }
    }
    
    cout << "√5 mod " << p << " = " << sqrt5 << "\n";
    cout << "Verify: " << sqrt5 << "² = " << (sqrt5 * sqrt5) % p << " mod " << p << "\n\n";
    
    // Compute φ = (1+√5)/2 mod p
    long long inv2 = (p + 1) / 2;  // Inverse ng 2 mod p
    long long phi = ((1 + sqrt5) * inv2) % p;
    long long psi = ((1 - sqrt5 + p) * inv2) % p;
    
    cout << "φ = (1+√5)/2 = " << phi << " mod " << p << "\n";
    cout << "ψ = (1-√5)/2 = " << psi << " mod " << p << "\n\n";
    
    // Verify properties
    cout << "PROPERTIES:\n";
    cout << "  φ + ψ = " << (phi + psi) % p << " (should be 1)\n";
    cout << "  φ · ψ = " << (phi * psi) % p << " (should be " << p-1 << " = -1)\n";
    cout << "  φ² = " << (phi * phi) % p << " (should be φ+1 = " << (phi+1)%p << ")\n";
    cout << "  ψ² = " << (psi * psi) % p << " (should be ψ+1 = " << (psi+1)%p << ")\n\n";
    
    // Compute Fibonacci/Lucas sequence mod p
    cout << "LUCAS SEQUENCE (mod " << p << "):\n";
    cout << "  L(0) = 2\n";
    cout << "  L(1) = 1\n";
    
    long long L_prev2 = 2;  // L(0)
    long long L_prev1 = 1;  // L(1)
    
    for (int n = 2; n <= 20; n++) {
        long long L_n = (L_prev1 + L_prev2) % p;
        cout << "  L(" << n << ") = " << L_n;
        
        // Check kung may period-2 or other patterns
        if (n % 2 == 0) cout << "  [EVEN]";
        if (n % 3 == 0) cout << "  [DIV by 3]";
        if (n % 5 == 0) cout << "  [DIV by 5]";
        cout << "\n";
        
        L_prev2 = L_prev1;
        L_prev1 = L_n;
    }
    
    // Compute powers of φ
    cout << "\nPOWERS OF φ (mod " << p << "):\n";
    long long phi_power = 1;
    for (int n = 0; n <= 10; n++) {
        cout << "  φ^" << n << " = " << phi_power << "\n";
        phi_power = (phi_power * phi) % p;
    }
    
    // Check periodicity
    cout << "\nPERIODICITY ANALYSIS:\n";
    cout << "  φ^0 = " << 1 << "\n";
    cout << "  φ^1 = " << phi << "\n";
    cout << "  φ^2 = " << (phi * phi) % p << "\n";
    cout << "  φ^3 = " << (phi * phi * phi) % p << "\n";
    cout << "  φ^4 = " << (phi * phi * phi * phi) % p << "\n";
    
    // Find order of φ
    cout << "\n  Finding order of φ...\n";
    long long temp = phi;
    for (int order = 1; order <= 20; order++) {
        if (temp == 1) {
            cout << "  Order of φ = " << order << "\n";
            break;
        }
        temp = (temp * phi) % p;
    }
    
    return 0;
}
