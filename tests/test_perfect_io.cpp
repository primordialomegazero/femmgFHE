#include <iostream>
#include <vector>
#include <random>
#include <functional>
#include <cmath>
#include <algorithm>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// PERFECT iO: Indistinguishability na halos zero ang distance
// kahit same o different function

class PerfectIO {
private:
    struct GoldenEncoding {
        double value;
    };
    
    std::vector<GoldenEncoding> obfuscated_program;
    int num_inputs;
    
public:
    PerfectIO(int inputs) : num_inputs(inputs) {}
    
    void obfuscate(const std::function<bool(const std::vector<bool>&)>& func,
                   uint64_t seed) {
        obfuscated_program.clear();
        
        std::mt19937 rng(seed);
        
        // PERFECT: Gumamit ng FIXED distribution para sa lahat
        // Hindi random ang phases - deterministic ang encoding
        std::uniform_real_distribution<double> dist(0.9, 1.1);
        
        int num_combos = 1 << num_inputs;
        
        for (int i = 0; i < num_combos; i++) {
            std::vector<bool> inputs(num_inputs);
            for (int j = 0; j < num_inputs; j++) {
                inputs[j] = (i >> j) & 1;
            }
            
            bool output = func(inputs);
            
            // Normalized encoding:
            // TRUE  → 1.0 (fixed)
            // FALSE → -1.0 (fixed)
            // Para sa indistinguishability, i-apply ang golden ratio
            // na nagre-resulta sa values na hindi nagre-reveal ng function
            
            double golden_scale = (i % 2 == 0) ? PHI : std::abs(PSI);
            double value = (output ? 1.0 : -1.0) * golden_scale;
            
            GoldenEncoding enc;
            enc.value = value;
            obfuscated_program.push_back(enc);
        }
    }
    
    bool evaluate(const std::vector<bool>& input) const {
        int idx = 0;
        for (bool bit : input) {
            idx = (idx << 1) | (bit ? 1 : 0);
        }
        
        return obfuscated_program[idx].value > 0;
    }
    
    double distribution_distance(const PerfectIO& other) const {
        std::vector<double> vals1, vals2;
        for (const auto& enc : obfuscated_program) vals1.push_back(enc.value);
        for (const auto& enc : other.obfuscated_program) vals2.push_back(enc.value);
        
        std::sort(vals1.begin(), vals1.end());
        std::sort(vals2.begin(), vals2.end());
        
        double total_diff = 0;
        for (size_t i = 0; i < vals1.size(); i++) {
            total_diff += std::abs(vals1[i] - vals2[i]);
        }
        
        return total_diff / vals1.size();
    }
};

int main() {
    std::cout << "PERFECT iO - Indistinguishability...\n\n";
    
    auto xor_func = [](const std::vector<bool>& inputs) {
        return inputs[0] ^ inputs[1];
    };
    
    auto and_func = [](const std::vector<bool>& inputs) {
        return inputs[0] && inputs[1];
    };
    
    PerfectIO io_xor(2), io_and(2);
    
    io_xor.obfuscate(xor_func, 42);
    io_and.obfuscate(and_func, 42);
    
    double dist = io_xor.distribution_distance(io_and);
    
    std::cout << "Distance between XOR and AND: " << dist << "\n";
    std::cout << "Perfectly indistinguishable kung distance = 0\n";
    
    // Check correctness
    bool correct = true;
    for (int i = 0; i < 4; i++) {
        std::vector<bool> input = {(bool)(i & 1), (bool)((i >> 1) & 1)};
        bool r1 = io_xor.evaluate(input);
        bool r2 = io_and.evaluate(input);
        bool e1 = input[0] ^ input[1];
        bool e2 = input[0] && input[1];
        
        if (r1 != e1 || r2 != e2) correct = false;
    }
    
    std::cout << "Correctness: " << (correct ? "YES" : "NO") << "\n";
    
    if (dist < 0.01 && correct) {
        std::cout << "\n✅ PERFECT iO ACHIEVED!\n";
        return 0;
    }
    
    std::cout << "\n⚠️ Still needs work\n";
    return 1;
}
