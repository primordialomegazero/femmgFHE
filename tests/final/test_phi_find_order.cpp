// FIND ORDER: Find moduli where φ has small order
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

// φ = (1+√5)/2 in ring Z_m[√5]
// We want φ^L ≡ 1 mod m
// This means: find m, L such that this holds

// Compute φ^n exactly using Fibonacci and Lucas numbers
// φ^n = (L_n + F_n·√5)/2 where L_n = Lucas numbers
// So φ^n ≡ 1 means L_n/2 ≡ 1 mod m and F_n/2 ≡ 0 mod m

long long gcd(long long a, long long b) {
    return b == 0 ? a : gcd(b, a % b);
}

int main() {
    cout << "\n  FINDING MODULI WHERE φ HAS SMALL ORDER\n\n";
    
    // Generate Fibonacci and Lucas numbers
    vector<long long> F = {0, 1};
    vector<long long> L = {2, 1};
    
    for (int i = 2; i <= 100; i++) {
        F.push_back(F[i-1] + F[i-2]);
        L.push_back(L[i-1] + L[i-2]);
    }
    
    cout << "  n | F_n | L_n | φ^n formula\n";
    cout << "  ----------------------------------------\n";
    for (int n = 0; n <= 20; n++) {
        cout << "  " << n << " | " << F[n] << " | " << L[n] << " | (";
        cout << L[n] << " + " << F[n] << "√5)/2\n";
    }
    
    // For φ^L ≡ 1 mod m, we need:
    // L_L/2 ≡ 1 mod m  AND  F_L/2 ≡ 0 mod m
    // So m divides F_L/2 and m divides (L_L/2 - 1)
    
    cout << "\n  Values of F_n/2:\n";
    for (int n = 1; n <= 30; n++) {
        if (F[n] % 2 == 0) {
            cout << "  n=" << n << ": F_" << n << "/2 = " << F[n]/2;
            // Check if L_n/2 - 1 shares factors
            long long val = L[n]/2 - 1;
            long long g = gcd(F[n]/2, val);
            if (g > 1) {
                cout << "  gcd with (L_n/2 - 1)=" << val << " is " << g;
                cout << " *** modulus candidate: " << g;
            }
            cout << "\n";
        }
    }
    
    // More systematic: find m where φ^L = 1
    cout << "\n  Searching for φ^L ≡ 1 mod m (small L, small m):\n";
    for (long long m = 2; m <= 1000; m++) {
        // Check if √5 exists mod m (i.e., 5 is quadratic residue)
        // Actually we just need the ring Z_m[φ] to be defined
        
        // For each m, compute φ^n iteratively until it cycles
        // φ in Z_m: represented as pair (a,b) where φ = (1+√5)/2
        // Actually simpler: use the recurrence
        
        // Start with φ^0 = 1 = (1, 0) in basis (1, φ)
        // Multiply by φ: (a,b) → (b, a+b) — this is mulY!
        
        long long a = 1, b = 0; // φ^0 = 1
        bool found = false;
        
        for (long long L = 1; L <= 200 && !found; L++) {
            // mulY: (a,b) → (b, a+b)
            long long new_a = b % m;
            long long new_b = (a + b) % m;
            a = new_a;
            b = new_b;
            
            // Check if back to (1,0)
            if (a == 1 && b == 0) {
                cout << "  m=" << m << " order=" << L;
                // Check if this modulus is useful (not too small, not too large)
                if (L >= 10 && L <= 50 && m >= 100) {
                    cout << " *** GOOD CANDIDATE";
                }
                cout << "\n";
                found = true;
            }
        }
    }
    
    cout << "\n  Done.\n\n";
    return 0;
}
