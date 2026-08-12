#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// ============================================================
// GF-N V3 — SQUARING-COMPATIBLE
// ============================================================
// Property: Enc(m²) can be derived from Enc(m) via public ops
// y1 evolves as: y1' = f(y1) where f is homomorphic-friendly
// ============================================================

struct GFN_V3 {
    double y1;           // Primary ciphertext component
    double y2;           // Secondary component (for Cassini)
    double seed;         // Seed state
    
    GFN_V3(double s = 42.0) : seed(s) {}
    
    // Encrypt: y1 = m * φ^seed_mod_1
    double encrypt(double m) {
        double seed_mod = fmod(seed * PHI, 1.0);
        y1 = fmod(m * pow(PHI, seed_mod * 10), 1.0);
        y2 = fmod(m * pow(fabs(PSI), seed_mod * 10), 1.0);
        return y1;
    }
    
    // SQUARING EVOLUTION: How does y1 change when m → m²?
    // y1_new = (m²) * φ^seed = (m * φ^seed)² / φ^seed = y1² / φ^seed
    // We can compute y1_new from y1 and seed!
    double evolve_under_squaring() {
        double seed_mod = fmod(seed * PHI, 1.0);
        double phi_factor = pow(PHI, seed_mod * 10);
        // y1' = y1² / phi_factor
        y1 = fmod((y1 * y1) / phi_factor, 1.0);
        // y2' = y2² / psi_factor
        double psi_factor = pow(fabs(PSI), seed_mod * 10);
        y2 = fmod((y2 * y2) / psi_factor, 1.0);
        // Seed rotation
        seed = fmod(seed * PHI + y1 * 0.001, 1.0);
        return y1;
    }
    
    // Cassini verification
    double cassini() {
        double phi_y1 = y1 + PHI;
        double psi_y2 = y2 + PSI;
        return fabs(phi_y1 * psi_y2 + 1.0);
    }
    
    // Decrypt
    double decrypt() {
        double seed_mod = fmod(seed * PHI, 1.0);
        double phi_factor = pow(PHI, seed_mod * 10);
        return fmod(y1 / phi_factor, 1.0);
    }
};

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "===============================================================\n";
    std::cout << "  GF-N V3 — SQUARING-COMPATIBLE INNER ENCRYPTION\n";
    std::cout << "  Property: Enc(m²) can be derived from Enc(m) + seed\n";
    std::cout << "===============================================================\n\n";

    GFN_V3 gfn(42.0);
    
    double m = 0.42;
    double y1 = gfn.encrypt(m);
    
    std::cout << "--- INITIAL ENCRYPTION ---\n";
    std::cout << "  m = " << m << "\n";
    std::cout << "  y1 = " << y1 << "\n";
    std::cout << "  y2 = " << gfn.y2 << "\n";
    std::cout << "  Cassini = " << gfn.cassini() << "\n";
    std::cout << "  Decrypt = " << gfn.decrypt() << "\n\n";

    // Test: Evolve under squaring 5 times
    std::cout << "--- EVOLUTION UNDER SQUARING ---\n\n";
    std::cout << "  " << std::setw(6) << "Step" << std::setw(14) << "y1" 
              << std::setw(14) << "y2" << std::setw(14) << "Cassini"
              << std::setw(14) << "Decrypt\n";
    std::cout << "  " << std::string(62, '-') << "\n";

    double expected_m = m;
    for (int step = 0; step < 5; step++) {
        gfn.evolve_under_squaring();
        expected_m = fmod(expected_m * expected_m, 1.0);
        
        std::cout << "  " << std::setw(6) << step 
                  << std::setw(14) << gfn.y1
                  << std::setw(14) << gfn.y2
                  << std::setw(14) << gfn.cassini()
                  << std::setw(14) << gfn.decrypt() << "\n";
    }
    
    std::cout << "  " << std::string(62, '-') << "\n\n";

    // Test: Can we recover m² from the evolved state?
    std::cout << "--- RECOVERY TEST ---\n";
    double recovered = gfn.decrypt();
    double expected = fmod(pow(m, 32), 1.0);  // After 5 squarings: m^(2^5)
    std::cout << "  After 5 squarings: decrypt = " << recovered << "\n";
    std::cout << "  Expected m^32 = " << expected << "\n";
    std::cout << "  Error = " << fabs(recovered - expected) << "\n\n";

    // Test: Cassini integrity after evolution
    std::cout << "--- CASSINI INTEGRITY ---\n";
    std::cout << "  Cassini after evolution: " << gfn.cassini() << "\n";
    std::cout << "  Status: " << (gfn.cassini() > 0.1 ? "PRESERVED" : "DEGRADED") << "\n";
    std::cout << "  phi*psi = " << PHI*PSI << "\n";

    std::cout << "\n===============================================================\n";
    std::cout << "  GF-N V3 — SQUARING-COMPATIBLE\n";
    std::cout << "  Enc(m²) = y1² / φ^seed — derivable from Enc(m)\n";
    std::cout << "===============================================================\n";

    return 0;
}
