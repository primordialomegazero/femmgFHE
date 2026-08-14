#include <iostream>
#include <cmath>
#include <vector>
#include <set>

constexpr double PHI = 1.6180339887498948482;

int main() {
    std::cout << "Equidistributed Debug: Bakit may bias?\n\n";
    
    // Test: φ·x mod 1 para sa iba't ibang initial values
    std::cout << "Initial values at distribution:\n";
    
    std::vector<double> initial_values = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
    
    for (double init : initial_values) {
        double x = init;
        int buckets[2] = {0, 0};  // Lower half vs upper half
        
        for (int i = 0; i < 1000; i++) {
            x = std::fmod(x * PHI, 1.0);
            buckets[x >= 0.5 ? 1 : 0]++;
        }
        
        std::cout << "  init=" << init << ": lower=" << buckets[0] 
                  << " upper=" << buckets[1] << "\n";
    }
    
    std::cout << "\n";
    
    // Ang problema: φ·x mod 1 ay may PERIODIC orbits
    // Kapag ang x ay nasa certain range, na-stuck sa lower/upper half
    
    // Check: ano ang nangyayari sa specific values
    std::cout << "Orbit analysis para sa init=0.5:\n";
    double x = 0.5;
    for (int i = 0; i < 10; i++) {
        x = std::fmod(x * PHI, 1.0);
        std::cout << "  iter " << i << ": " << x << "\n";
    }
    
    std::cout << "\n";
    
    // FIX: Gumamit ng GOLDEN ANGLE (addition) hindi multiplication
    // f(x) = (x + golden_angle) mod 1
    std::cout << "FIX: Golden Angle addition (hindi multiplication):\n";
    
    double golden_angle = std::fmod(PHI, 1.0);  // φ mod 1 = 0.618034
    
    int buckets[10] = {0};
    x = 0.123456789;
    
    for (int i = 0; i < 100000; i++) {
        x = std::fmod(x + golden_angle, 1.0);
        buckets[static_cast<int>(x * 10)]++;
    }
    
    std::cout << "  100K samples (golden angle addition):\n";
    for (int i = 0; i < 10; i++) {
        std::cout << "  [" << i * 10 << "%-" << (i+1) * 10 << "%]: " << buckets[i] << "\n";
    }
    
    int min_b = buckets[0], max_b = buckets[0];
    for (int i = 1; i < 10; i++) {
        min_b = std::min(min_b, buckets[i]);
        max_b = std::max(max_b, buckets[i]);
    }
    
    double balance = (max_b - min_b) / 10000.0;
    std::cout << "  Balance: " << balance << " (0 = perfect)\n";
    std::cout << "  Status: " << (balance < 0.1 ? "EQUIDISTRIBUTED ✅" : "BIASED ❌") << "\n";
    
    return 0;
}
