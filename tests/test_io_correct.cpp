#include "src/io/golden_io_arbitrary.h"
#include "src/io/golden_io_bootstrap.h"
#include <iostream>
#include <functional>

int main() {
    std::cout << "=== TAMANG PAGGAMIT NG iO ===\n\n";
    
    // Test 1: ArbitraryFunctionIO para sa XOR
    std::cout << "--- ArbitraryFunctionIO: XOR ---\n";
    {
        using namespace GoldenIOArbitrary;
        
        auto xor_func = [](const std::vector<bool>& input) -> bool {
            return input[0] != input[1];
        };
        
        ArbitraryFunctionIO io(2, xor_func);
        io.obfuscate(42);
        
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
    
    // Test 2: ArbitraryFunctionIO para sa AND
    std::cout << "--- ArbitraryFunctionIO: AND ---\n";
    {
        using namespace GoldenIOArbitrary;
        
        auto and_func = [](const std::vector<bool>& input) -> bool {
            return input[0] && input[1];
        };
        
        ArbitraryFunctionIO io(2, and_func);
        io.obfuscate(12345);
        
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
    
    // Test 3: ArbitraryFunctionIO para sa Majority
    std::cout << "--- ArbitraryFunctionIO: 3-input Majority ---\n";
    {
        using namespace GoldenIOArbitrary;
        
        auto majority_func = [](const std::vector<bool>& input) -> bool {
            int sum = 0;
            for (bool b : input) sum += b ? 1 : 0;
            return sum >= 2;
        };
        
        ArbitraryFunctionIO io(3, majority_func);
        io.obfuscate(777);
        
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
    
    // Test 4: UnlimitedIO para sa OR
    std::cout << "--- UnlimitedIO: OR (with bootstrapping) ---\n";
    {
        using namespace GoldenIOBootstrap;
        
        auto or_func = [](const std::vector<bool>& input) -> bool {
            return input[0] || input[1];
        };
        
        UnlimitedIO io(64);
        io.obfuscate(or_func, 2, 999);
        
        int correct = 0;
        for (int a = 0; a < 2; a++) {
            for (int b = 0; b < 2; b++) {
                std::vector<bool> input = {(bool)a, (bool)b};
                bool expected = or_func(input);
                bool result = io.evaluate_unlimited(input);
                if (result == expected) correct++;
            }
        }
        std::cout << "  Result: " << correct << "/4\n\n";
    }
    
    // Test 5: QuantumIO para sa NOT
    std::cout << "--- QuantumIO: NOT (with Hadamard) ---\n";
    {
        using namespace GoldenIOArbitrary;
        
        auto not_func = [](const std::vector<bool>& input) -> bool {
            return !input[0];
        };
        
        QuantumIO io(1, not_func);
        io.obfuscate(555);
        
        // Apply Hadamard para sa quantum superposition
        io.apply_hadamard();
        
        int correct = 0;
        for (int a = 0; a < 2; a++) {
            std::vector<bool> input = {(bool)a};
            bool expected = not_func(input);
            bool result = io.evaluate(input);
            if (result == expected) correct++;
            
            std::cout << "  f(" << a << ") = " 
                     << result << " (expected " << expected << ") "
                     << (result == expected ? "✅" : "❌") << "\n";
        }
        std::cout << "  Result: " << correct << "/2\n";
    }
    
    return 0;
}
