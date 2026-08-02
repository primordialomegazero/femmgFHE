// THEOREM 3 — Superpose Symmetry (Standalone Test)
// Verifies: superpose(A,B) = conjugate of superpose(B,A)
#include <iostream>
#include <cmath>
#include <cassert>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

void superpose(double phi_A, double psi_A, double phi_B, double psi_B, double& phi_out, double& psi_out) {
    double mp = phi_A*PHI + phi_B*PSI + psi_A*PSI + psi_B*PHI;
    double ms = psi_A*PHI + psi_B*PSI + phi_A*PSI + phi_B*PHI;
    phi_out = mp - std::floor(mp);
    psi_out = ms - std::floor(ms);
}

int main() {
    std::cout << "\n==============================================================\n";
    std::cout << "  THEOREM 3: Superpose Symmetry (Standalone)\n";
    std::cout << "==============================================================\n\n";
    
    double phi_A=0.2, psi_A=0.8, phi_B=0.3, psi_B=0.7;
    double phi_AB, psi_AB, phi_BA, psi_BA;
    
    superpose(phi_A, psi_A, phi_B, psi_B, phi_AB, psi_AB);
    superpose(phi_B, psi_B, phi_A, psi_A, phi_BA, psi_BA);
    
    std::cout << "  superpose(A,B) -> phi=" << phi_AB << " psi=" << psi_AB << "\n";
    std::cout << "  superpose(B,A) -> phi=" << phi_BA << " psi=" << psi_BA << "\n";
    std::cout << "  Symmetry: phi_AB≈psi_BA and psi_AB≈phi_BA\n";
    
    bool symmetric = (std::abs(phi_AB - psi_BA) < 1e-10) && (std::abs(psi_AB - phi_BA) < 1e-10);
    std::cout << "  " << (symmetric ? "PASS: THEOREM 3 VERIFIED" : "FAIL") << "\n\n";
    
    assert(symmetric);
    return 0;
}
