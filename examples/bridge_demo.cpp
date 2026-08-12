#include "src/bridge/dual_gate_bridge_fixed.h"
#include <iostream>

int main() {
    SpiralIO::DualGateFixed dg(1.0, 0.0);
    std::cout << "Bridge Demo: DualGate projection\n";
    std::cout << "  φ_val: " << dg.phi_val << "\n";
    std::cout << "  ψ_val: " << dg.psi_val << "\n";
    std::cout << "  Projection: " << dg.projection() << "\n";
    std::cout << "  Verify: " << (dg.verify() ? "PASS" : "FAIL") << "\n";
    std::cout << "  to_bool: " << dg.to_bool() << "\n";
    return 0;
}
