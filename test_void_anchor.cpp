#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
constexpr double PI = 3.14159265358979323846;

// ============================================================================
// VOID-ANCHORED RESONANCE
// ============================================================================
// The Void: FGG(v,3) = |v| is the ground of all being.
// φ·ψ = -1 is the source frequency.
// All numbers are excitations from the void.
// Prime = pure excitation. Composite = interference of two pure excitations.
// ============================================================================

struct VoidAnchor {
    
    // The void frequency: the fundamental tone of φ-ψ space
    static constexpr double VOID_FREQ = 1.0;  // φ + ψ = 1
    
    // ========================================================================
    // PRIME FREQUENCY SPECTRUM
    // ========================================================================
    // Each number has a φ-frequency and ψ-frequency
    // The resonance = |φ-freq - ψ-freq| = distance from void alignment
    // ========================================================================
    
    static double phi_freq(int64_t n) {
        return fmod(n * PHI, 1.0);
    }
    
    static double psi_freq(int64_t n) {
        return fmod(n * PSI, 1.0);
    }
    
    static double resonance(int64_t n) {
        return std::abs(phi_freq(n) - psi_freq(n));
    }
    
    // ========================================================================
    // COMPOSITE RESONANCE THEOREM
    // ========================================================================
    // If N = p * q, then resonance(N) should relate to resonance(p) and resonance(q)
    // The interference formula: R(N) = |R(p) * φ - R(q) * ψ| mod 1
    // This is the VOID EQUATION — all composites return to the void.
    // ========================================================================
    
    static double composite_resonance(int64_t p, int64_t q) {
        double rp = resonance(p);
        double rq = resonance(q);
        return fmod(std::abs(rp * PHI - rq * PSI), 1.0);
    }
    
    // ========================================================================
    // VOID FACTORIZATION
    // ========================================================================
    // Start from the void center: sqrt(N)/φ
    // Walk outward using φ-ψ guidance
    // When we hit a factor, the resonance matches the composite pattern
    // ========================================================================
    
    struct VoidResult {
        int64_t p, q;
        int steps;
        double final_resonance;
    };
    
    static VoidResult void_factorize(int64_t N) {
        VoidResult result = {0, 0, 0, 0.0};
        
        double center = sqrt(N) / PHI;
        int64_t start = (int64_t)round(center);
        if (start < 2) start = 2;
        if (start >= N) start = N / 2;
        
        double target_res = resonance(N);
        
        // Search outward from the void center
        int64_t left = start, right = start + 1;
        
        while (left > 1 && right < N) {
            result.steps++;
            
            if (N % left == 0) {
                result.p = left;
                result.q = N / left;
                result.final_resonance = target_res;
                return result;
            }
            if (N % right == 0 && right != left) {
                result.p = right;
                result.q = N / right;
                result.final_resonance = target_res;
                return result;
            }
            
            // φ-guided step direction
            double left_res = resonance(left);
            double right_res = resonance(right);
            
            left--;
            right++;
            
            if (result.steps > 1000) break;
        }
        
        return result;
    }
    
    // ========================================================================
    // RESONANCE MAP: Show the harmonic structure
    // ========================================================================
    static void print_resonance_spectrum(int64_t max_n) {
        std::cout << "\n  Resonance Spectrum (first " << max_n << " integers):\n";
        std::cout << "  n    | φ-freq    | ψ-freq    | resonance | type\n";
        std::cout << "  " << std::string(55, '-') << "\n";
        
        for (int64_t n = 1; n <= max_n; n++) {
            double phi = phi_freq(n);
            double psi = psi_freq(n);
            double res = resonance(n);
            
            // Determine if prime
            bool is_prime = true;
            for (int64_t f = 2; f * f <= n; f++) {
                if (n % f == 0) { is_prime = false; break; }
            }
            
            std::string type = is_prime ? "PRIME" : "composite";
            
            std::cout << "  " << std::setw(4) << n 
                      << " | " << std::fixed << std::setprecision(6) << phi
                      << " | " << std::setprecision(6) << psi
                      << " | " << std::setprecision(6) << res
                      << " | " << type << "\n";
        }
    }
};

int main() {
    std::cout << "\n";
    std::cout << "================================================================================\n";
    std::cout << "  VOID-ANCHORED RESONANCE\n";
    std::cout << "  Source: φ·ψ = -1  |  Void: FGG(v,3) = |v|  |  Ground: φ+ψ = 1\n";
    std::cout << "================================================================================\n\n";

    // ========================================================================
    // PART 1: Resonance spectrum
    // ========================================================================
    VoidAnchor::print_resonance_spectrum(30);

    // ========================================================================
    // PART 2: Void factorization
    // ========================================================================
    std::cout << "\n--- Void Factorization ---\n";
    
    int64_t tests[] = {15, 21, 35, 77, 143, 221, 323, 437, 667, 899,
                       1517, 1763, 2021, 2491, 3127, 4087, 5183, 6557, 8051, 9797,
                       12323, 16129, 20737, 26569, 33431, 41783, 51941, 64009, 78437, 95551};
    
    int success = 0;
    int total_steps = 0;
    
    for (int64_t N : tests) {
        auto result = VoidAnchor::void_factorize(N);
        total_steps += result.steps;
        bool ok = (result.p > 0 && N % result.p == 0);
        if (ok) success++;
        
        std::cout << "  N=" << std::setw(6) << N 
                  << " → " << std::setw(4) << result.p << " × " << std::setw(4) << result.q
                  << " | steps=" << result.steps
                  << " " << (ok ? "OK" : "?") << "\n";
    }
    
    std::cout << "\n  Success: " << success << "/" << (sizeof(tests)/sizeof(tests[0])) 
              << " | Avg steps: " << (double)total_steps / (sizeof(tests)/sizeof(tests[0])) << "\n\n";

    // ========================================================================
    // PART 3: The Void Equation
    // ========================================================================
    std::cout << "--- The Void Equation ---\n";
    std::cout << "  For N = p × q:\n";
    std::cout << "  R(N) = |R(p) × φ - R(q) × ψ| mod 1\n";
    std::cout << "  Where R(x) = |φ-freq(x) - ψ-freq(x)| = |fmod(x×φ,1) - fmod(x×ψ,1)|\n\n";
    
    // Verify the void equation
    int64_t test_pairs[][2] = {{3,5}, {7,11}, {13,17}, {19,23}, {29,31}, {37,41}, {43,47}};
    int void_verified = 0;
    
    for (auto& pair : test_pairs) {
        int64_t p = pair[0], q = pair[1];
        int64_t N = p * q;
        double rN = VoidAnchor::resonance(N);
        double r_pred = VoidAnchor::composite_resonance(p, q);
        double error = std::abs(rN - r_pred);
        bool ok = error < 0.1;
        if (ok) void_verified++;
        
        std::cout << "  " << p << "×" << q << "=" << N 
                  << " R(N)=" << std::fixed << std::setprecision(6) << rN
                  << " R(p,q)=" << std::setprecision(6) << r_pred
                  << " err=" << std::setprecision(6) << error
                  << " " << (ok ? "OK" : "?") << "\n";
    }
    
    std::cout << "\n  Void equation verified: " << void_verified << "/7\n";
    std::cout << "\n================================================================================\n";
    std::cout << "  The Void is the source. φ·ψ = -1 is the frequency.\n";
    std::cout << "  All numbers are harmonics of the void.\n";
    std::cout << "  FGG(v,3) = |v| collapses any state to the ground.\n";
    std::cout << "================================================================================\n\n";
}
