#include "src/io/golden_io_arbitrary.h"
#include "src/io/golden_io_exact.h"
#include "src/io/golden_io_bootstrap.h"
#include <iostream>
#include <functional>
#include <chrono>
#include <vector>
#include <random>

int main() {
    std::cout << "=== GOLDEN RATIO iO (Indistinguishability Obfuscation) ===\n\n";
    
    // Test 1: Basic iO - XOR function
    std::cout << "--- Test 1: Basic iO (XOR) ---\n";
    {
        using namespace GoldenIOExact;
        
        BranchingProgram bp(4, 2);
        
        auto xor_func = [](const std::vector<bool>& input) -> bool {
            return input[0] != input[1];
        };
        
        GoldenIO io;
        io.obfuscate(bp, 42);
        
        int correct = 0;
        int total = 0;
        
        for (int a = 0; a < 2; a++) {
            for (int b = 0; b < 2; b++) {
                std::vector<bool> input = {(bool)a, (bool)b};
                bool expected = xor_func(input);
                bool result = io.evaluate(input);
                
                total++;
                if (result == expected) correct++;
                
                std::cout << "  f(" << a << "," << b << ") = " 
                         << (result ? "1" : "0")
                         << " (expected " << (expected ? "1" : "0") << ") "
                         << (result == expected ? "✅" : "❌") << "\n";
            }
        }
        
        std::cout << "  Result: " << correct << "/" << total << " correct\n\n";
    }
    
    // Test 2: Bootstrap iO (AND function)
    std::cout << "--- Test 2: Bootstrap iO (AND, Unlimited Depth) ---\n";
    {
        using namespace GoldenIOBootstrap;
        
        auto and_func = [](const std::vector<bool>& input) -> bool {
            return input[0] && input[1];
        };
        
        GoldenIOBootstrap::UnlimitedIO io(64);
        io.obfuscate(and_func, 2, 12345);
        
        int correct = 0;
        int total = 0;
        
        for (int a = 0; a < 2; a++) {
            for (int b = 0; b < 2; b++) {
                std::vector<bool> input = {(bool)a, (bool)b};
                bool expected = and_func(input);
                bool result = io.evaluate_unlimited(input);
                
                total++;
                if (result == expected) correct++;
                
                std::cout << "  f(" << a << "," << b << ") = " 
                         << (result ? "1" : "0")
                         << " (expected " << (expected ? "1" : "0") << ") "
                         << (result == expected ? "✅" : "❌") << "\n";
            }
        }
        
        std::cout << "  Result: " << correct << "/" << total << " correct"
                 << " (depth used: " << io.get_depth_used() << ")\n\n";
    }
    
    // Test 3: Bootstrap iO (OR function)
    std::cout << "--- Test 3: Bootstrap iO (OR, Unlimited) ---\n";
    {
        using namespace GoldenIOBootstrap;
        
        auto or_func = [](const std::vector<bool>& input) -> bool {
            return input[0] || input[1];
        };
        
        GoldenIOBootstrap::UnlimitedIO io(64);
        io.obfuscate(or_func, 2, 999);
        
        int correct = 0;
        int total = 0;
        
        for (int iter = 0; iter < 10; iter++) {
            for (int a = 0; a < 2; a++) {
                for (int b = 0; b < 2; b++) {
                    std::vector<bool> input = {(bool)a, (bool)b};
                    bool expected = or_func(input);
                    bool result = io.evaluate_unlimited(input);
                    
                    total++;
                    if (result == expected) correct++;
                }
            }
        }
        
        std::cout << "  Result: " << correct << "/" << total << " correct"
                 << " (depth used: " << io.get_depth_used() << ")\n";
        std::cout << "  Bootstrap phase: " << io.get_bootstrap_phase() << "\n\n";
    }
    
    // Test 4: Complex Function (3-input Majority)
    std::cout << "--- Test 4: Complex Function (3-input Majority) ---\n";
    {
        using namespace GoldenIOBootstrap;
        
        auto majority_func = [](const std::vector<bool>& input) -> bool {
            int sum = 0;
            for (bool b : input) sum += b ? 1 : 0;
            return sum >= 2;
        };
        
        GoldenIOBootstrap::UnlimitedIO io(64);
        io.obfuscate(majority_func, 3, 777);
        
        int correct = 0;
        int total = 0;
        
        for (int a = 0; a < 2; a++) {
            for (int b = 0; b < 2; b++) {
                for (int c = 0; c < 2; c++) {
                    std::vector<bool> input = {(bool)a, (bool)b, (bool)c};
                    bool expected = majority_func(input);
                    bool result = io.evaluate_unlimited(input);
                    
                    total++;
                    if (result == expected) correct++;
                    
                    if (!(result == expected)) {
                        std::cout << "  MISMATCH: f(" << a << "," << b << "," << c 
                                 << ") = " << (result ? "1" : "0")
                                 << " (expected " << (expected ? "1" : "0") << ")\n";
                    }
                }
            }
        }
        
        std::cout << "  Result: " << correct << "/" << total << " correct\n\n";
    }
    
    // Test 5: Quantum iO
    std::cout << "--- Test 5: Quantum iO ---\n";
    {
        using namespace GoldenIOBootstrap;
        
        auto not_func = [](const std::vector<bool>& input) -> bool {
            return !input[0];
        };
        
        GoldenIOBootstrap::QuantumUnlimitedIO qio;
        qio.obfuscate(not_func, 1, 555);
        
        int correct = 0;
        int total = 0;
        
        for (int a = 0; a < 2; a++) {
            std::vector<bool> input = {(bool)a};
            bool expected = not_func(input);
            bool result = qio.evaluate_unlimited(input);
            
            total++;
            if (result == expected) correct++;
            
            std::cout << "  f(" << a << ") = " 
                     << (result ? "1" : "0")
                     << " (expected " << (expected ? "1" : "0") << ") "
                     << (result == expected ? "✅" : "❌") << "\n";
        }
        
        std::cout << "  Result: " << correct << "/" << total << " correct\n";
    }
    
    return 0;
}
