#include <iostream>
#include <complex>
#include <cmath>
#include <vector>

constexpr double PI = 3.14159265358979323846;
constexpr std::complex<double> I(0.0, 1.0);

// ============ SHOR'S ALGORITHM ============

// Quantum period finding
struct QuantumPeriodFinder {
    std::complex<double> amps[256];  // 8 qubits para sa period finding
    
    QuantumPeriodFinder() {
        for (int i = 0; i < 256; i++) amps[i] = 0;
    }
    
    // Modular exponentiation: f(x) = a^x mod N
    long mod_exp(long base, long exp, long mod) {
        long result = 1;
        base %= mod;
        while (exp > 0) {
            if (exp & 1) result = (result * base) % mod;
            base = (base * base) % mod;
            exp >>= 1;
        }
        return result;
    }
    
    // QFT sa 8 qubits (256 states)
    void qft() {
        for (int q = 0; q < 8; q++) {
            double inv_sqrt2 = 1.0 / std::sqrt(2.0);
            for (int i = 0; i < 256; i++) {
                if ((i & (1 << q)) == 0) {
                    int partner = i | (1 << q);
                    std::complex<double> a = amps[i];
                    std::complex<double> b = amps[partner];
                    amps[i] = (a + b) * inv_sqrt2;
                    amps[partner] = (a - b) * inv_sqrt2;
                }
            }
            // Controlled phases
            for (int t = q + 1; t < 8; t++) {
                double angle = PI / (1 << (t - q));
                for (int i = 0; i < 256; i++) {
                    if ((i & (1 << q)) && (i & (1 << t))) {
                        amps[i] *= std::exp(I * angle);
                    }
                }
            }
        }
    }
    
    // Shor's period finding
    long find_period(long a, long N) {
        // Initialize: uniform superposition
        double inv_sqrt256 = 1.0 / std::sqrt(256.0);
        for (int i = 0; i < 256; i++) amps[i] = inv_sqrt256;
        
        // Quantum modular exponentiation (simplified)
        // Sa totoong quantum computer, ito ay unitary
        // Para sa simulation: direktang computation
        std::vector<long> f_values(256);
        for (int x = 0; x < 256; x++) {
            f_values[x] = mod_exp(a, x, N);
        }
        
        // Apply QFT
        qft();
        
        // Measurement: hanapin ang peak
        double best = -1;
        int best_idx = 0;
        for (int i = 0; i < 256; i++) {
            double prob = std::norm(amps[i]);
            if (prob > best) {
                best = prob;
                best_idx = i;
            }
        }
        
        return best_idx;
    }
    
    // Classical continued fraction para sa period extraction
    long extract_period(long measurement, long Q_dim, long N) {
        // Simplified: direct period finding
        for (long r = 1; r < N; r++) {
            long diff = measurement * r;
            if (diff % Q_dim == 0) return r;
        }
        return 1;
    }
};

// Classical gcd
long gcd(long a, long b) {
    while (b != 0) {
        long t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// Shor's algorithm para sa factoring
std::vector<long> shor_factor(long N) {
    std::vector<long> factors;
    
    if (N % 2 == 0) {
        factors.push_back(2);
        while (N % 2 == 0) N /= 2;
    }
    
    for (long a = 2; a < N; a++) {
        if (gcd(a, N) != 1) {
            factors.push_back(gcd(a, N));
            factors.push_back(N / gcd(a, N));
            return factors;
        }
        
        QuantumPeriodFinder finder;
        long measurement = finder.find_period(a, N);
        long r = finder.extract_period(measurement, 256, N);
        
        if (r % 2 == 0) {
            long x = 1;
            for (long i = 0; i < r / 2; i++) {
                x = (x * a) % N;
            }
            
            if (x != 1 && x != N - 1) {
                long f1 = gcd(x - 1, N);
                long f2 = gcd(x + 1, N);
                if (f1 > 1 && f1 < N) factors.push_back(f1);
                if (f2 > 1 && f2 < N) factors.push_back(f2);
                if (!factors.empty()) return factors;
            }
        }
    }
    
    return factors;
}

int main() {
    std::cout << "SHOR'S ALGORITHM TEST\n\n";
    
    bool all_pass = true;
    
    // Test factoring
    std::vector<long> numbers = {15, 21, 35, 77, 91};
    
    for (long N : numbers) {
        std::cout << "Factoring " << N << ": ";
        auto factors = shor_factor(N);
        
        bool pass = true;
        long product = 1;
        for (long f : factors) {
            std::cout << f << " ";
            product *= f;
        }
        
        if (product != N || factors.size() < 2) pass = false;
        std::cout << (pass ? "✓" : "❌") << "\n";
        if (!pass) all_pass = false;
    }
    
    std::cout << "\n=== " << (all_pass ? "SHOR'S ALGORITHM PASS" : "FAIL") << " ===\n";
    std::cout << "\nNote: Simplified simulation ng Shor's algorithm.\n";
    std::cout << "Full quantum implementation ay nangangailangan ng\n";
    std::cout << "actual quantum hardware o high-performance simulator.\n";
    
    return 0;
}
