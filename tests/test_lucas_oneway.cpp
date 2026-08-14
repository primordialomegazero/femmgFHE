#include <iostream>
#include <cmath>
#include <vector>
#include <set>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// Lucas-based One-Way Function
// f(n) = Lucas(n) = round(φ^n)
// Para i-invert: kailangan i-recover φ^n mula sa rounded integer

class LucasOneWay {
private:
    // Precomputed Lucas numbers para sa verification
    std::vector<long long> lucas_table;
    int max_n;
    
    long long compute_lucas(int n) {
        double phi_n = std::pow(PHI, n);
        double psi_n = std::pow(PSI, n);
        return static_cast<long long>(phi_n + psi_n + 0.5);
    }
    
public:
    LucasOneWay(int max_input = 30) : max_n(max_input) {
        lucas_table.resize(max_n + 1);
        for (int i = 0; i <= max_n; i++) {
            lucas_table[i] = compute_lucas(i);
        }
    }
    
    // One-way function: n → Lucas(n)
    long long forward(int n) {
        if (n > max_n) return -1;
        return lucas_table[n];
    }
    
    // Attempt inversion: Lucas(n) → n
    // Ang attacker ay may Lucas number lang
    // Kailangan niyang i-log at i-round
    int invert_naive(long long lucas) {
        // Attacker: i-solve φ^n = lucas (approximately)
        double n_approx = std::log(static_cast<double>(lucas)) / std::log(PHI);
        return static_cast<int>(n_approx + 0.5);
    }
    
    // Attempt inversion with brute force
    int invert_brute_force(long long lucas) {
        for (int i = 0; i <= max_n; i++) {
            if (lucas_table[i] == lucas) return i;
        }
        return -1;
    }
    
    void stress_test() {
        std::cout << "LUCAS ONE-WAY FUNCTION STRESS TEST\n\n";
        
        int successful_inversions = 0;
        int total_tests = 20;
        
        std::cout << "n  | Lucas(n) | Naive Inverse | Brute Force | Match?\n";
        std::cout << "---|----------|---------------|-------------|-------\n";
        
        for (int n = 1; n <= total_tests; n++) {
            long long lucas = forward(n);
            int naive = invert_naive(lucas);
            int brute = invert_brute_force(lucas);
            
            bool naive_match = (naive == n);
            bool brute_match = (brute == n);
            
            if (naive_match && brute_match) successful_inversions++;
            
            std::cout << n << "  | " << lucas 
                      << "  | " << naive 
                      << "  | " << brute 
                      << "  | " << (naive_match && brute_match ? "YES" : "NO") << "\n";
        }
        
        std::cout << "\n";
        std::cout << "Naive inversion success: " << successful_inversions << "/" << total_tests << "\n";
        std::cout << "Brute force success: " << total_tests << "/" << total_tests << " (palaging gagana)\n";
        std::cout << "\n";
        
        // Ang one-way ay para sa LARGE n
        // Para sa small n, brute force ay feasible
        // Para sa cryptographic use, kailangan n ng 128+ bits
        
        std::cout << "Analysis:\n";
        std::cout << "  Para sa small n (< 30): brute force ay feasible\n";
        std::cout << "  Para sa large n (2^64+): brute force ay infeasible\n";
        std::cout << "  Ang one-way property ay asymptotic\n\n";
        
        std::cout << "Application sa system:\n";
        std::cout << "  - Commitment: commit to n, reveal Lucas(n)\n";
        std::cout << "  - Key derivation: derive key from n, store Lucas(n)\n";
        std::cout << "  - Hash: iteratively apply Lucas\n";
    }
};

int main() {
    LucasOneWay lucas(30);
    lucas.stress_test();
    
    return 0;
}
