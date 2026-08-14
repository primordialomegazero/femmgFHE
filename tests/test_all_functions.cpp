#include <iomanip>
#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>
#include <functional>

int main() {
    std::cout << "TEST: LAHAT NG 16 BOOLEAN FUNCTIONS (2 inputs)\n\n";
    
    GoldenPrivacySystem gps(42);
    
    // Lahat ng 16 functions
    std::vector<std::pair<std::string, std::function<bool(const std::vector<bool>&)>>> functions = {
        {"FALSE", [](const std::vector<bool>& in) { return false; }},
        {"AND", [](const std::vector<bool>& in) { return in[0] && in[1]; }},
        {"A AND NOT B", [](const std::vector<bool>& in) { return in[0] && !in[1]; }},
        {"A", [](const std::vector<bool>& in) { return in[0]; }},
        {"NOT A AND B", [](const std::vector<bool>& in) { return !in[0] && in[1]; }},
        {"B", [](const std::vector<bool>& in) { return in[1]; }},
        {"XOR", [](const std::vector<bool>& in) { return in[0] ^ in[1]; }},
        {"OR", [](const std::vector<bool>& in) { return in[0] || in[1]; }},
        {"NOR", [](const std::vector<bool>& in) { return !(in[0] || in[1]); }},
        {"XNOR", [](const std::vector<bool>& in) { return !(in[0] ^ in[1]); }},
        {"NOT B", [](const std::vector<bool>& in) { return !in[1]; }},
        {"A OR NOT B", [](const std::vector<bool>& in) { return in[0] || !in[1]; }},
        {"NOT A", [](const std::vector<bool>& in) { return !in[0]; }},
        {"IMPLIES", [](const std::vector<bool>& in) { return !in[0] || in[1]; }},
        {"NAND", [](const std::vector<bool>& in) { return !(in[0] && in[1]); }},
        {"TRUE", [](const std::vector<bool>& in) { return true; }}
    };
    
    int all_passed = 0;
    
    for (auto& [name, func] : functions) {
        gps.obfuscate_program(func, 2);
        
        bool correct = true;
        for (int i = 0; i < 4; i++) {
            std::vector<bool> input = {(bool)((i >> 1) & 1), (bool)(i & 1)};
            bool result = gps.evaluate_io_public(input);
            bool expected = func(input);
            if (result != expected) correct = false;
        }
        
        if (correct) all_passed++;
        std::cout << "  " << std::left << std::setw(20) << name 
                  << (correct ? "✅" : "❌") << "\n";
    }
    
    std::cout << "\nResult: " << all_passed << "/16 functions PASSED\n";
    
    return 0;
}
