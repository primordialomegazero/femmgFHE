#include "../src/core/banach_engine.h"
#include <iostream>
#include <iomanip>

int main() {
    banach::NDimBanachEngine engine;
    
    auto ct1 = engine.encrypt(42, 0);
    auto ct2 = engine.encrypt(43, 0);
    
    std::cout << std::setprecision(15);
    std::cout << "ct1.coordinates[0]: " << ct1.coordinates[0] << std::endl;
    std::cout << "ct2.coordinates[0]: " << ct2.coordinates[0] << std::endl;
    std::cout << "diff: " << std::abs(ct1.coordinates[0] - ct2.coordinates[0]) << std::endl;
    
    // Also print ALL coordinates
    std::cout << "\nAll coords for 42:" << std::endl;
    for (int i = 0; i < 7; i++) std::cout << "  [" << i << "]: " << ct1.coordinates[i] << std::endl;
    
    std::cout << "\nAll coords for 43:" << std::endl;
    for (int i = 0; i < 7; i++) std::cout << "  [" << i << "]: " << ct2.coordinates[i] << std::endl;
    
    std::cout << "\nlyapunov[0] 42: " << ct1.lyapunov_spectrum[0] << std::endl;
    std::cout << "lyapunov[0] 43: " << ct2.lyapunov_spectrum[0] << std::endl;
    
    return 0;
}
