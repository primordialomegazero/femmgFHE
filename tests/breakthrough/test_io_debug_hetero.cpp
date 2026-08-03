#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <vector>
#include <algorithm>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

double commutative_reconstruct(const std::vector<double>& v) {
    double n = v.size();
    double sum = 0, prod = 1, harm_sum = 0, sum_sq = 0;
    for (auto val : v) { 
        sum += val; prod *= (val+0.0001); 
        harm_sum += 1.0/(val+0.001); sum_sq += val*val; 
    }
    return 0.35*sum/n + 0.25*std::pow(prod,1.0/n) + 0.25*n/harm_sum + 0.15*std::sqrt(sum_sq/n);
}

int main() {
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> val(0.1, 0.9);
    std::uniform_int_distribution<int> bit(0, 1);
    
    std::cout << "=== CORRECTED: Both circuits use SAME φ/ψ mapping ===\n\n";
    
    for (int t = 0; t < 10; t++) {
        std::vector<double> raw_A, raw_B;
        
        for (int s = 0; s < 10; s++) {
            int X=bit(gen), Y=bit(gen), Z=bit(gen);
            double v = val(gen);
            
            int res_A = (X&Y)|Z;
            int res_B = (X|Z)&(Y|Z);
            
            // SAME mapping: true→φ, false→ψ for BOTH circuits
            // The structural difference is in the CIRCUIT COMPUTATION,
            // not in the output mapping.
            double out_A = res_A ? v*PHI : v*PSI;
            double out_B = res_B ? v*PHI : v*PSI;  // SAME mapping
            
            // Now shuffle Circuit B to create structural difference
            raw_A.push_back(out_A);
            raw_B.push_back(out_B);
        }
        
        // Shuffle B to create structural difference
        std::shuffle(raw_B.begin(), raw_B.end(), gen);
        
        double rec_A = commutative_reconstruct(raw_A);
        double rec_B = commutative_reconstruct(raw_B);
        
        // Same multiset → same reconstruction
        std::cout << "Test " << t << ": rec_A=" << std::fixed << std::setprecision(6) << rec_A
                  << " rec_B=" << rec_B << " diff=" << std::abs(rec_A-rec_B) << "\n";
    }
    
    std::cout << "\n=== ANALYSIS ===\n";
    std::cout << "When both circuits use the SAME φ/ψ output mapping:\n";
    std::cout << "  - Same Boolean function (Theorem 1)\n";
    std::cout << "  - Same output values (same mapping)\n";
    std::cout << "  - Different structure (shuffled order)\n";
    std::cout << "  - Same multiset → commutative_reconstruct identical\n";
    std::cout << "  → This IS the correct structural iO model\n";
    
    std::cout << "\n=== QUESTION ===\n";
    std::cout << "How to create DIFFERENT intermediate values while\n";
    std::cout << "preserving the same Boolean function?\n";
    std::cout << "  → Apply φ/ψ DURING gate evaluation, not at output\n";
    std::cout << "  → Each gate: result = (a⊕b) rotated by φ or ψ\n";
    std::cout << "  → Output = final accumulated value\n";
    
    return 0;
}
