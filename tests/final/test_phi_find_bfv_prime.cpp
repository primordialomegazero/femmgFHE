// FIND BFV-COMPATIBLE φ-CYCLE PRIME
// Wider search, more flexible criteria

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
using namespace std;

bool is_prime(long long n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (long long d = 3; d * d <= n; d += 2) {
        if (n % d == 0) return false;
    }
    return true;
}

// Find √5 mod p
long long sqrt_mod(long long a, long long p) {
    for (long long x = 0; x < p; x++) {
        if ((x * x) % p == a % p) return x;
    }
    return -1;
}

long long mod_inv(long long a, long long p) {
    // a^(p-2) mod p (Fermat)
    long long result = 1;
    long long base = a % p;
    long long exp = p - 2;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % p;
        base = (base * base) % p;
        exp >>= 1;
    }
    return result;
}

int main() {
    cout << "\n  FINDING BFV-COMPATIBLE φ-CYCLE PRIMES\n\n";
    
    // Try different ring dimensions
    int ring_dims[] = {1024, 2048, 4096, 8192, 16384};
    
    for (int rd : ring_dims) {
        long long m = 2 * rd;
        cout << "  RingDim=" << rd << " (m=" << m << "): ";
        
        int found = 0;
        // Search larger k range
        for (long long k = 1; k <= 5000 && found < 8; k++) {
            long long p = k * m + 1;
            
            if (!is_prime(p)) continue;
            
            long long sqrt5 = sqrt_mod(5, p);
            if (sqrt5 == -1) continue;
            
            long long inv2 = mod_inv(2, p);
            long long phi = ((1 + sqrt5) * inv2) % p;
            
            // Find order
            long long val = 1;
            int order = 0;
            for (int i = 1; i <= 200; i++) {
                val = (val * phi) % p;
                if (val == 1) {
                    order = i;
                    break;
                }
            }
            
            if (order >= 15 && order <= 80) {
                cout << " p=" << p << "(ord=" << order << ")";
                found++;
            }
        }
        
        if (found == 0) {
            cout << "none found in range";
        }
        cout << "\n";
    }
    
    // Alternative: what if we don't need √5 in the modulus?
    // We can encode φ and ψ directly as plaintext values
    // and define operations differently
    cout << "\n  ALTERNATIVE APPROACH:\n";
    cout << "  Instead of finding modulus where φ exists natively,\n";
    cout << "  we can encode {ψ, φ} as regular integers and\n";
    cout << "  define NAND using polynomial evaluation.\n";
    cout << "  This works with ANY BFV plaintext modulus.\n\n";
    
    // Polynomial for NAND with {a, b} encoding where a=ψ, b=φ:
    // We have 4 points: (ψ,ψ)→φ+ψ-ψ*ψ? No, we need NAND in the φ-ring
    // Actually: in {ψ, φ} encoding with multiplication:
    // AND(ψ,ψ)=ψ², AND(ψ,φ)=ψφ, AND(φ,ψ)=φψ, AND(φ,φ)=φ²
    // NAND = φ+ψ - AND (since φ+ψ=1)
    // So NAND is linear in the product: NAND = 1 - AND
    
    // For any p large enough to hold these values:
    cout << "  For any prime p > φ² (p > 3):\n";
    cout << "  Encode bit 0 = 0, bit 1 = 1 (regular binary)\n";
    cout << "  NAND = 1 - a*b mod p\n";
    cout << "  This is just standard BFV binary arithmetic!\n\n";
    
    cout << "  THE TRUTH:\n";
    cout << "  The φ-ring encoding adds value when operations\n";
    cout << "  are done IN the φ-ring (CKKS approximate arithmetic).\n";
    cout << "  For exact integer arithmetic (BFV/BGV),\n";
    cout << "  the simplest encoding is binary {0, 1}.\n";
    cout << "  φ-cycles are most useful in approximate/continuous schemes.\n\n";

    return 0;
}
