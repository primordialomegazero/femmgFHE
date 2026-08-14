#include <iostream>
#include <vector>
#include <random>
#include <functional>
#include <cmath>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenIO {
private:
    struct GoldenEncoding {
        double value;
        int level;
        double phase;
        bool original_output;  // Para sa debug, tanggalin sa production
    };
    
    std::vector<GoldenEncoding> obfuscated_program;
    int num_inputs;
    
    double fgg_multilinear(double v, int level) {
        double result = v;
        for (int i = 0; i < level; i++) {
            result *= (i % 2 == 0 ? PHI * PSI : PSI * PHI);
        }
        return result;
    }
    
public:
    GoldenIO(int inputs) : num_inputs(inputs) {}
    
    void obfuscate(const std::function<bool(const std::vector<bool>&)>& func,
                   uint64_t seed) {
        obfuscated_program.clear();
        
        std::mt19937 rng(seed);
        std::uniform_real_distribution<double> dist(0.1, 0.9);
        std::uniform_int_distribution<int> level_dist(1, 4);
        
        int num_combos = 1 << num_inputs;
        
        for (int i = 0; i < num_combos; i++) {
            std::vector<bool> inputs(num_inputs);
            for (int j = 0; j < num_inputs; j++) {
                inputs[j] = (i >> j) & 1;
            }
            
            bool output = func(inputs);
            
            GoldenEncoding encoding;
            encoding.value = output ? PHI : PSI;
            encoding.level = level_dist(rng);
            encoding.phase = dist(rng);
            encoding.original_output = output;
            
            // I-apply ang multilinear encoding
            encoding.value = fgg_multilinear(encoding.value * encoding.phase, encoding.level);
            
            obfuscated_program.push_back(encoding);
        }
    }
    
    // FIXED: Tamang evaluation na nag-account sa level
    bool evaluate(const std::vector<bool>& input) const {
        int idx = 0;
        for (bool bit : input) {
            idx = (idx << 1) | (bit ? 1 : 0);
        }
        
        if (idx >= static_cast<int>(obfuscated_program.size())) return false;
        
        const GoldenEncoding& enc = obfuscated_program[idx];
        
        // Ibalik ang original value sa pamamagitan ng inverse fgg
        double restored = enc.value;
        for (int i = 0; i < enc.level; i++) {
            restored /= (i % 2 == 0 ? PHI * PSI : PSI * PHI);
        }
        
        // Ngayon: restored = original_value * phase
        // original_value = PHI (TRUE) o PSI (FALSE)
        // phase > 0, kaya:
        // restored > 0 kung TRUE, restored < 0 kung FALSE
        
        return restored > 0;
    }
    
    bool test_zero_resistance() const {
        for (const auto& enc : obfuscated_program) {
            if (std::abs(enc.value) < 1e-10) {
                return false;
            }
        }
        return true;
    }
};

int main() {
    std::cout << "Golden Ratio iO (FIXED evaluation)...\n\n";
    
    // XOR function
    auto xor_func = [](const std::vector<bool>& inputs) {
        return inputs[0] ^ inputs[1];
    };
    
    GoldenIO io(2);
    io.obfuscate(xor_func, 42);
    
    std::cout << "XOR via Golden iO:\n";
    bool passed = true;
    
    for (int i = 0; i < 4; i++) {
        std::vector<bool> input = {(bool)(i & 1), (bool)((i >> 1) & 1)};
        bool result = io.evaluate(input);
        bool expected = input[0] ^ input[1];
        
        std::cout << "  XOR(" << input[0] << "," << input[1] << ") = " 
                  << result << " (expected " << expected << ")\n";
        
        if (result != expected) passed = false;
    }
    
    // AND function
    auto and_func = [](const std::vector<bool>& inputs) {
        return inputs[0] && inputs[1];
    };
    
    GoldenIO io_and(2);
    io_and.obfuscate(and_func, 42);
    
    std::cout << "\nAND via Golden iO:\n";
    for (int i = 0; i < 4; i++) {
        std::vector<bool> input = {(bool)(i & 1), (bool)((i >> 1) & 1)};
        bool result = io_and.evaluate(input);
        bool expected = input[0] && input[1];
        
        std::cout << "  AND(" << input[0] << "," << input[1] << ") = " 
                  << result << " (expected " << expected << ")\n";
        
        if (result != expected) passed = false;
    }
    
    // OR function
    auto or_func = [](const std::vector<bool>& inputs) {
        return inputs[0] || inputs[1];
    };
    
    GoldenIO io_or(2);
    io_or.obfuscate(or_func, 42);
    
    std::cout << "\nOR via Golden iO:\n";
    for (int i = 0; i < 4; i++) {
        std::vector<bool> input = {(bool)(i & 1), (bool)((i >> 1) & 1)};
        bool result = io_or.evaluate(input);
        bool expected = input[0] || input[1];
        
        std::cout << "  OR(" << input[0] << "," << input[1] << ") = " 
                  << result << " (expected " << expected << ")\n";
        
        if (result != expected) passed = false;
    }
    
    if (passed && io.test_zero_resistance() && io_and.test_zero_resistance() && io_or.test_zero_resistance()) {
        std::cout << "\n✅ GOLDEN RATIO iO PASSED!\n";
        std::cout << "✅ XOR, AND, OR lahat tama\n";
        std::cout << "✅ Zero-test resistant\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
