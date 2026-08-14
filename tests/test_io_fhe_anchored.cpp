#include "src/io/golden_io_fhe_anchored.h"
#include <iostream>
#include <functional>

int main() {
    std::cout << "=== iO ANCHORED SA FHE ===\n\n";
    
    // Test XOR
    std::cout << "--- XOR ---\n";
    {
        using namespace GoldenIOFHEAnchored;
        
        auto xor_func = [](const std::vector<bool>& input) -> bool {
            return input[0] != input[1];
        };
        
        FHEAnchoredIO io(64);
        io.obfuscate(xor_func, 2, 42);
        
        int correct = 0;
        for (int a = 0; a < 2; a++) {
            for (int b = 0; b < 2; b++) {
                std::vector<bool> input = {(bool)a, (bool)b};
                bool expected = xor_func(input);
                bool result = io.evaluate(input);
                if (result == expected) correct++;
                
                std::cout << "  f(" << a << "," << b << ") = " 
                         << result << " (expected " << expected << ") "
                         << (result == expected ? "✅" : "❌") << "\n";
            }
        }
        std::cout << "  Result: " << correct << "/4\n\n";
    }
    
    // Test AND
    std::cout << "--- AND ---\n";
    {
        using namespace GoldenIOFHEAnchored;
        
        auto and_func = [](const std::vector<bool>& input) -> bool {
            return input[0] && input[1];
        };
        
        FHEAnchoredIO io(64);
        io.obfuscate(and_func, 2, 12345);
        
        int correct = 0;
        for (int a = 0; a < 2; a++) {
            for (int b = 0; b < 2; b++) {
                std::vector<bool> input = {(bool)a, (bool)b};
                bool expected = and_func(input);
                bool result = io.evaluate(input);
                if (result == expected) correct++;
            }
        }
        std::cout << "  Result: " << correct << "/4\n\n";
    }
    
    // Test Majority
    std::cout << "--- 3-input Majority ---\n";
    {
        using namespace GoldenIOFHEAnchored;
        
        auto majority_func = [](const std::vector<bool>& input) -> bool {
            int sum = 0;
            for (bool b : input) sum += b ? 1 : 0;
            return sum >= 2;
        };
        
        FHEAnchoredIO io(64);
        io.obfuscate(majority_func, 3, 777);
        
        int correct = 0;
        for (int a = 0; a < 2; a++) {
            for (int b = 0; b < 2; b++) {
                for (int c = 0; c < 2; c++) {
                    std::vector<bool> input = {(bool)a, (bool)b, (bool)c};
                    bool expected = majority_func(input);
                    bool result = io.evaluate(input);
                    if (result == expected) correct++;
                }
            }
        }
        std::cout << "  Result: " << correct << "/8\n\n";
    }
    
    return 0;
}
