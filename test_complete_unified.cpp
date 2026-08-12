#include "spiral_fhe_io_complete_unified.h"

int main() {
    std::cout << "======================================================================\n";
    std::cout << "  SPIRAL FHE+iO COMPLETE UNIFIED v41.0\n";
    std::cout << "  Multidimensional iO + Fibonacci FHE + Anti-Matter + Cold Fusion\n";
    std::cout << "  phi*psi = -1 -> 1+1=2 -> ALL OUT\n";
    std::cout << "======================================================================\n\n";

    auto system = SpiralCompleteNS::SpiralComplete::create()
        .withRingDim(16384)
        .withTotalCycles(500)
        .withLogInterval(50)
        .build();
    
    system.init();
    system.run();
    
    std::cout << "\nLog: spiral_complete.log\n";
    return 0;
}
