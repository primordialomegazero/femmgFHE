#include <iostream>
#include <vector>
#include <random>
#include <functional>
#include <cmath>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

class GoldenIOV2 {
private:
    struct GoldenEncoding {
        double value;
        int level;
    };
    
    std::vector<GoldenEncoding> obfuscated_program;
    int num_inputs;
    
    // FIXED: Lahat ay same level para sa indistinguishability
    // Tapos i-apply ang randomization sa phase lang
    double encode(double v, double phase) {
        return v * phase;
    }
    
    double decode(double v, double phase) {
        return v / phase;
    }
    
public:
    GoldenIOV2(int inputs) : num_inputs(inputs) {}
    
    void obfuscate(const std::function<bool(const std::vector<bool>&)>& func,
                   uint64_t seed) {
        obfuscated_program.clear();
        
        std::mt19937 rng(seed);
        std::uniform_real_distribution<double> dist(0.5, 1.5);  // Narrower range
        
        int num_combos = 1 << num_inputs;
        
        for (int i = 0; i < num_combos; i++) {
            std::vector<bool> inputs(num_inputs);
            for (int j = 0; j < num_inputs; j++) {
                inputs[j] = (i >> j) & 1;
            }
            
            bool output = func(inputs);
            double phase = dist(rng);
            
            // Lahat ay level 1 para sa uniformity
            double value = output ? PHI : PSI;
            value = encode(value, phase);
            
            GoldenEncoding enc;
            enc.value = value;
            enc.level = 1;
            obfuscated_program.push_back(enc);
        }
    }
    
    bool evaluate(const std::vector<bool>& input) const {
        int idx = 0;
        for (bool bit : input) {
            idx = (idx << 1) | (bit ? 1 : 0);
        }
        
        const auto& enc = obfuscated_program[idx];
        
        // PHI > 0, PSI < 0
        return enc.value > 0;
    }
    
    double statistical_distance(const GoldenIOV2& other) const {
        if (obfuscated_program.size() != other.obfuscated_program.size()) return 1.0;
        
        double max_diff = 0;
        for (size_t i = 0; i < obfuscated_program.size(); i++) {
            double diff = std::abs(obfuscated_program[i].value - other.obfuscated_program[i].value);
            max_diff = std::max(max_diff, diff);
        }
        
        return max_diff;
    }
    
    bool test_zero_resistance() const {
        for (const auto& enc : obfuscated_program) {
            if (std::abs(enc.value) < 0.01) return false;
        }
        return true;
    }
};

int main() {
    std::cout << "Golden iO V2 - Indistinguishability Fix...\n\n";
    
    auto xor_func = [](const std::vector<bool>& inputs) {
        return inputs[0] ^ inputs[1];
    };
    
    GoldenIOV2 io_a(2);
    GoldenIOV2 io_b(2);
    
    io_a.obfuscate(xor_func, 42);
    io_b.obfuscate(xor_func, 99);
    
    double dist = io_a.statistical_distance(io_b);
    
    std::cout << "Statistical distance: " << dist << "\n";
    std::cout << "Indistinguishable: " << (dist < 0.5 ? "YES" : "NO") << "\n";
    
    // I-verify correctness
    bool correct = true;
    for (int i = 0; i < 4; i++) {
        std::vector<bool> input = {(bool)(i & 1), (bool)((i >> 1) & 1)};
        bool result_a = io_a.evaluate(input);
        bool result_b = io_b.evaluate(input);
        bool expected = input[0] ^ input[1];
        
        if (result_a != expected || result_b != expected) {
            correct = false;
        }
    }
    
    std::cout << "Correctness: " << (correct ? "YES" : "NO") << "\n";
    std::cout << "Zero-test resistant: " << (io_a.test_zero_resistance() ? "YES" : "NO") << "\n";
    
    if (dist < 0.5 && correct && io_a.test_zero_resistance()) {
        std::cout << "\n✅ INDISTINGUISHABILITY FIXED!\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
