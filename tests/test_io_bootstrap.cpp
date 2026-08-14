#include "../src/io/golden_io_bootstrap.h"
#include <iostream>
#include <vector>
#include <cassert>

using namespace GoldenIOBootstrap;

int main() {
    std::cout << "iO BOOTSTRAP TEST (FIXED)\n\n";
    
    // Test 1: fgg_multilinear na walang abs()
    std::cout << "1. fgg_multilinear (walang abs):\n";
    double v1 = fgg_multilinear(PHI, 1);
    double v2 = fgg_multilinear(PHI, 2);
    double v3 = fgg_multilinear(PHI, 3);
    
    std::cout << "  fgg(PHI, 1) = " << v1 << " (negative kasi PHI*PSI=-1)\n";
    std::cout << "  fgg(PHI, 2) = " << v2 << " (positive ulit)\n";
    std::cout << "  fgg(PHI, 3) = " << v3 << " (negative ulit)\n";
    
    // Test 2: UnlimitedIO with XOR
    std::cout << "\n2. UnlimitedIO XOR:\n";
    UnlimitedIO io;
    
    auto xor_func = [](const std::vector<bool>& in) {
        return in[0] ^ in[1];
    };
    
    io.obfuscate(xor_func, 2, 42);
    
    bool correct = true;
    for (int i = 0; i < 4; i++) {
        std::vector<bool> input = {(bool)((i >> 1) & 1), (bool)(i & 1)};
        bool result = io.evaluate_with_bootstrap(input);
        bool expected = input[0] ^ input[1];
        
        if (result != expected) correct = false;
    }
    
    std::cout << "  XOR: " << (correct ? "4/4 PASSED ✅" : "FAILED ❌") << "\n";
    
    // Test 3: Unlimited evaluation (depth testing)
    std::cout << "\n3. Unlimited evaluation:\n";
    UnlimitedIO io_unlimited;
    io_unlimited.obfuscate(xor_func, 2, 99);
    
    int total = 0;
    for (int i = 0; i < 100; i++) {
        std::vector<bool> input = {(bool)((i >> 1) & 1), (bool)(i & 1)};
        bool result = io_unlimited.evaluate_unlimited(input);
        if (result == (input[0] ^ input[1])) total++;
    }
    
    std::cout << "  100 evaluations: " << total << "/100 correct\n";
    std::cout << "  Depth used: " << io_unlimited.get_depth_used() << "\n";
    std::cout << "  Bootstrap phase: " << io_unlimited.get_bootstrap_phase() << "\n";
    
    // Test 4: QuantumUnlimitedIO
    std::cout << "\n4. QuantumUnlimitedIO:\n";
    QuantumUnlimitedIO qio;
    qio.obfuscate(xor_func, 2, 123);
    
    int qtotal = 0;
    for (int i = 0; i < 20; i++) {
        std::vector<bool> input = {(bool)((i >> 1) & 1), (bool)(i & 1)};
        bool result = qio.evaluate_unlimited(input);
        if (result == (input[0] ^ input[1])) qtotal++;
    }
    
    std::cout << "  20 evaluations: " << qtotal << "/20 correct\n";
    
    std::cout << "\n✅ iO BOOTSTRAP TEST COMPLETE!\n";
    
    return 0;
}
