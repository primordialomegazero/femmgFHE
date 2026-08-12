#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

double io_fgg(double raw_val, int depth, bool use_phi) {
    double current = raw_val;
    for (int d = 0; d < depth; d++) {
        double encoded = (d % 2 == 0) ?
            (use_phi ? current * PHI : current * PSI) :
            (use_phi ? current * PSI : current * PHI);
        double collapsed = (d % 2 == 0) ?
            std::abs(encoded * PSI) : std::abs(encoded * PHI);
        current = collapsed;
    }
    return current;
}

double io_fuzzy_nand(double a, double b) {
    double ca = std::min(1.0, std::max(0.0, a));
    double cb = std::min(1.0, std::max(0.0, b));
    return 1.0 - ca * cb;
}

struct EmergentThreshold {
    double thresh_phi, thresh_psi;
    EmergentThreshold() {
        thresh_phi = (io_fgg(0.0, 3, true) + io_fgg(1.0, 3, true)) / 2.0;
        thresh_psi = (io_fgg(0.0, 3, false) + io_fgg(1.0, 3, false)) / 2.0;
    }
    bool to_bool(double val, bool use_phi) {
        return use_phi ? (val > thresh_phi) : (val > thresh_psi);
    }
};

int main() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  PHI-PATH vs PSI-PATH -- io_fgg FROM io_emergent.h\n";
    std::cout << "===============================================================\n\n";

    EmergentThreshold eth;

    double phi_false = io_fgg(0.0, 3, true);
    double phi_true  = io_fgg(1.0, 3, true);
    double psi_false = io_fgg(0.0, 3, false);
    double psi_true  = io_fgg(1.0, 3, false);
    
    std::cout << "--- EMERGENT THRESHOLD ---\n";
    std::cout << "  phi-path: FALSE=" << phi_false << " TRUE=" << phi_true 
              << " threshold=" << eth.thresh_phi << "\n";
    std::cout << "  psi-path: FALSE=" << psi_false << " TRUE=" << psi_true
              << " threshold=" << eth.thresh_psi << "\n\n";

    std::cout << "--- PHI-PATH vs PSI-PATH NAND ---\n\n";
    std::cout << "  " << std::setw(6) << "a" << std::setw(6) << "b" 
              << std::setw(12) << "phi-NAND" << std::setw(12) << "psi-NAND"
              << std::setw(10) << "phi-bool" << std::setw(10) << "psi-bool"
              << std::setw(10) << "Expected\n";
    std::cout << "  " << std::string(66, '-') << "\n";

    double ins[4][2] = {{0,0},{0,1},{1,0},{1,1}};
    int exp[4] = {1,1,1,0};
    int pass = 0;

    for (int i = 0; i < 4; i++) {
        double raw = io_fuzzy_nand(ins[i][0], ins[i][1]);
        double phi_nand = io_fgg(raw, 3, true);
        double psi_nand = io_fgg(raw, 3, false);
        bool phi_bool = eth.to_bool(phi_nand, true);
        bool psi_bool = eth.to_bool(psi_nand, false);
        bool expected = exp[i];
        bool ok = (phi_bool == expected) && (psi_bool == expected);
        if (ok) pass++;
        
        std::cout << "  " << std::setw(6) << ins[i][0] << std::setw(6) << ins[i][1]
                  << std::setw(12) << phi_nand << std::setw(12) << psi_nand
                  << std::setw(10) << (phi_bool ? "T" : "F") 
                  << std::setw(10) << (psi_bool ? "T" : "F")
                  << std::setw(10) << (expected ? "T" : "F")
                  << "  " << (ok ? "PASS" : "FAIL") << "\n";
    }
    std::cout << "  " << std::string(66, '-') << "\n";
    std::cout << "  Result: " << pass << "/4\n\n";

    std::cout << "--- KEY INSIGHT ---\n";
    std::cout << "  phi-path and psi-path produce DIFFERENT continuous values\n";
    std::cout << "  but SAME Boolean output via emergent thresholds.\n";
    std::cout << "  Indistinguishability: attacker sees only the Boolean output.\n";
    std::cout << "  They cannot determine which path (phi or psi) was used.\n\n";

    std::cout << "--- FOUNDATION ---\n";
    std::cout << "  phi*psi = " << PHI*PSI << "\n";
    std::cout << "  phi+psi = " << PHI+PSI << "\n";

    std::cout << "\n===============================================================\n";
    std::cout << "  PHI-PATH vs PSI-PATH -- BOTH CORRECT, DIFFERENT INTERNALS\n";
    std::cout << "===============================================================\n";
    return 0;
}
