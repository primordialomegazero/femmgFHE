#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "===============================================================\n";
    std::cout << "  SEED EVOLUTION DATA GATHERING\n";
    std::cout << "  Testing: How should seed evolve under squaring?\n";
    std::cout << "===============================================================\n\n";

    // Test 1: Different seed evolution formulas
    std::cout << "--- SEED EVOLUTION FORMULAS ---\n\n";
    
    double seeds[] = {0.1, 0.25, 0.5, 0.618, 0.75, 0.9};
    int n_seeds = 6;
    
    struct Formula {
        std::string name;
        double (*f)(double);
    };
    
    Formula formulas[] = {
        {"seed * 2 * φ", [](double s) { return fmod(s * 2.0 * PHI, 1.0); }},
        {"seed * φ²", [](double s) { return fmod(s * PHI * PHI, 1.0); }},
        {"seed * φ + ψ", [](double s) { return fmod(s * PHI + PSI, 1.0); }},
        {"seed * (1+φ)", [](double s) { return fmod(s * (1.0 + PHI), 1.0); }},
        {"seed * φ mod 0.5", [](double s) { return fmod(s * PHI, 0.5); }},
        {"(seed + 1) / φ", [](double s) { return fmod((s + 1.0) / PHI, 1.0); }},
    };
    int n_formulas = 6;
    
    for (int f = 0; f < n_formulas; f++) {
        std::cout << "  " << formulas[f].name << ":\n";
        std::cout << "    ";
        for (int s = 0; s < n_seeds; s++) {
            double val = seeds[s];
            std::cout << std::setw(10) << val;
        }
        std::cout << "\n    ";
        for (int s = 0; s < n_seeds; s++) {
            double val = formulas[f].f(seeds[s]);
            std::cout << std::setw(10) << val;
        }
        std::cout << "\n    ";
        // Check if any seed goes to 0
        for (int s = 0; s < n_seeds; s++) {
            double val = formulas[f].f(seeds[s]);
            std::cout << std::setw(10) << (val < 0.001 ? "ZERO!" : (val > 0.999 ? "ONE!" : "ok"));
        }
        std::cout << "\n\n";
    }

    // Test 2: Tracking squaring with seed
    std::cout << "--- TRACKING m² VIA SEED ---\n\n";
    std::cout << "  Goal: y1 = m * φ^seed\n";
    std::cout << "  After FGG: y1' = y1² = m² * φ^(2*seed)\n";
    std::cout << "  Need: seed' such that y1' = m² * φ^seed'\n";
    std::cout << "  So: φ^(2*seed) = φ^seed' → seed' = 2*seed\n\n";
    
    std::cout << "  " << std::setw(10) << "m" << std::setw(10) << "seed"
              << std::setw(14) << "y1=m*φ^seed" << std::setw(14) << "y1²"
              << std::setw(14) << "seed'=2*seed" << std::setw(14) << "m²*φ^seed'\n";
    std::cout << "  " << std::string(76, '-') << "\n";
    
    double test_seeds[] = {0.1, 0.25, 0.5, 0.618, 0.75};
    double m = 0.42;
    
    for (double s : test_seeds) {
        double phi_pow = pow(PHI, s * 10);
        double y1 = fmod(m * phi_pow, 1.0);
        double y1_sq = fmod(y1 * y1, 1.0);
        double seed_prime = fmod(s * 2.0, 1.0);
        double phi_pow_prime = pow(PHI, seed_prime * 10);
        double expected = fmod(m * m * phi_pow_prime, 1.0);
        
        std::cout << "  " << std::setw(10) << m << std::setw(10) << s
                  << std::setw(14) << y1 << std::setw(14) << y1_sq
                  << std::setw(14) << seed_prime << std::setw(14) << expected
                  << "  " << (fabs(y1_sq - expected) < 0.01 ? "MATCH" : "MISMATCH") << "\n";
    }

    // Test 3: Seed behavior over multiple squarings
    std::cout << "\n--- SEED OVER MULTIPLE SQUARINGS ---\n\n";
    
    for (double s : test_seeds) {
        std::cout << "  seed₀ = " << s << ": ";
        double seed = s;
        bool alive = true;
        for (int i = 0; i < 10 && alive; i++) {
            seed = fmod(seed * 2.0, 1.0);
            std::cout << std::setw(10) << seed;
            if (seed < 0.001 || seed > 0.999) {
                std::cout << " (DEAD)";
                alive = false;
            }
        }
        std::cout << "\n";
    }

    // Test 4: Alternative — additive instead of multiplicative
    std::cout << "\n--- ADDITIVE SEED EVOLUTION ---\n\n";
    std::cout << "  seed' = fmod(seed + φ, 1.0) — never goes to 0\n\n";
    
    for (double s : test_seeds) {
        std::cout << "  seed₀ = " << s << ": ";
        double seed = s;
        for (int i = 0; i < 10; i++) {
            seed = fmod(seed + PHI, 1.0);
            std::cout << std::setw(10) << seed;
        }
        std::cout << "\n";
    }

    // Test 5: The winning approach
    std::cout << "\n--- WINNER: MULTIPLICATIVE + ADDITIVE ---\n\n";
    std::cout << "  seed' = fmod(seed * 2 + φ, 1.0) — avoids 0, tracks squaring\n\n";
    
    for (double s : test_seeds) {
        std::cout << "  seed₀ = " << s << ": ";
        double seed = s;
        for (int i = 0; i < 10; i++) {
            seed = fmod(seed * 2.0 + PHI, 1.0);
            std::cout << std::setw(10) << seed;
        }
        std::cout << "\n";
    }

    std::cout << "\n===============================================================\n";
    std::cout << "  SEED EVOLUTION DATA COMPLETE\n";
    std::cout << "===============================================================\n";
    return 0;
}
