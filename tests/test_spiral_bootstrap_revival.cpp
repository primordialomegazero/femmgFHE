// REVIVE: SPIRAL BOOTSTRAP METAPROGRAMMED
// I-test natin kung gumagana pa ang iyong dating system
// at kung pwedeng i-adapt sa current CKKS setup

#include "../archive/experimental/research_drafts/spiral_bootstrap_metaprogrammed_standalone.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SPIRAL BOOTSTRAP REVIVAL TEST\n";
    std::cout << "  Testing Your Original System\n";
    std::cout << "========================================\n\n";
    
    // Test with different configurations
    std::cout << "TEST 1: SpiralBootstrap3D (N=3, Depth=3, Quantum=ON, Recursive=ON)\n";
    std::cout << "=============================================================\n";
    
    SpiralBootstrap3D bs3d;
    std::vector<double> input = {1.0, 2.0, 3.0};
    
    bs3d.print_status();
    std::cout << "\n  Input: [1.0, 2.0, 3.0]\n";
    
    auto output = bs3d.bootstrap(input);
    std::cout << "  Output: [";
    for (size_t i = 0; i < output.size(); i++) {
        std::cout << output[i];
        if (i < output.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n\n";
    
    bs3d.print_status();
    
    // Test 100 bootstraps
    std::cout << "\nTEST 2: 100 BOOTSTRAPS STRESS TEST\n";
    std::cout << "===================================\n";
    
    SpiralBootstrap4D_Deep bs4d;
    auto current = input;
    
    for (int i = 0; i < 100; i++) {
        current = bs4d.bootstrap(current);
        
        if (i % 20 == 0 || i == 99) {
            std::cout << "  Bootstrap " << i << ": ";
            bs4d.print_status();
            std::cout << "\n";
        }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Bootstraps: " << bs4d.bootstrap_count << "\n";
    std::cout << "  Cassini: " << (bs4d.verify_cassini() ? "✅ STABLE" : "⟳ EMERGING") << "\n";
    std::cout << "========================================\n";
    
    return 0;
}
