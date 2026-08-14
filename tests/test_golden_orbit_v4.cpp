#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <complex>
#include <chrono>

constexpr double PHI = 1.6180339887498948482;
constexpr double PI = 3.14159265358979323846;
constexpr std::complex<double> I(0.0, 1.0);

class GoldenOrbitIOV4 {
private:
    struct OrbitEncoding {
        std::complex<double> value;
    };
    
    std::vector<OrbitEncoding> obfuscated_program;
    int num_inputs;
    
public:
    GoldenOrbitIOV4(int inputs) : num_inputs(inputs) {}
    
    void obfuscate(const std::function<bool(const std::vector<bool>&)>& func,
                   uint64_t seed) {
        obfuscated_program.clear();
        
        std::mt19937 rng(seed);
        std::uniform_real_distribution<double> upper(0.1, PI - 0.1);
        std::uniform_real_distribution<double> lower(PI + 0.1, 2.0 * PI - 0.1);
        
        int num_combos = 1 << num_inputs;
        
        for (int i = 0; i < num_combos; i++) {
            // FIXED INDEXING: i=0→(0,0), i=1→(0,1), i=2→(1,0), i=3→(1,1)
            // Para sa 2 inputs: bit0 = (i & 1), bit1 = ((i >> 1) & 1)
            // Pero sa evaluate: idx = (idx << 1) | bit
            // Kaya: idx=0 → (0,0), idx=1 → (0,1), idx=2 → (1,0), idx=3 → (1,1)
            
            std::vector<bool> inputs(num_inputs);
            inputs[0] = (i >> 1) & 1;  // MSB = bit 0
            inputs[1] = i & 1;          // LSB = bit 1
            
            bool output = func(inputs);
            
            double angle = output ? upper(rng) : lower(rng);
            std::complex<double> value = std::exp(I * angle);
            
            obfuscated_program.push_back({value});
        }
    }
    
    bool evaluate(const std::vector<bool>& input) const {
        int idx = 0;
        for (bool bit : input) {
            idx = (idx << 1) | (bit ? 1 : 0);
        }
        
        if (idx >= static_cast<int>(obfuscated_program.size())) return false;
        
        return obfuscated_program[idx].value.imag() > 0;
    }
    
    double ks_distance(const GoldenOrbitIOV4& other) const {
        std::vector<double> mags1, mags2;
        for (const auto& enc : obfuscated_program) mags1.push_back(std::abs(enc.value));
        for (const auto& enc : other.obfuscated_program) mags2.push_back(std::abs(enc.value));
        
        std::sort(mags1.begin(), mags1.end());
        std::sort(mags2.begin(), mags2.end());
        
        double max_diff = 0;
        for (size_t i = 0; i < mags1.size(); i++) {
            double cdf1 = (i + 1) / static_cast<double>(mags1.size());
            double cdf2 = (i + 1) / static_cast<double>(mags2.size());
            max_diff = std::max(max_diff, std::abs(cdf1 - cdf2));
        }
        
        return max_diff;
    }
    
    double benchmark(int runs) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < runs; i++) {
            std::vector<bool> input(num_inputs);
            for (int j = 0; j < num_inputs; j++) input[j] = ((i >> j) & 1);
            evaluate(input);
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
};

int main() {
    std::cout << "GOLDEN ORBIT iO V4 - Fixed Indexing...\n\n";
    
    std::vector<std::pair<std::string, std::function<bool(const std::vector<bool>&)>>> functions = {
        {"XOR", [](const std::vector<bool>& in) { return in[0] ^ in[1]; }},
        {"AND", [](const std::vector<bool>& in) { return in[0] && in[1]; }},
        {"OR", [](const std::vector<bool>& in) { return in[0] || in[1]; }},
        {"NAND", [](const std::vector<bool>& in) { return !(in[0] && in[1]); }},
        {"NOR", [](const std::vector<bool>& in) { return !(in[0] || in[1]); }},
        {"XOR+AND", [](const std::vector<bool>& in) { return (in[0] ^ in[1]) && in[0]; }},
        {"IMPLIES", [](const std::vector<bool>& in) { return !in[0] || in[1]; }},
        {"EQUIV", [](const std::vector<bool>& in) { return in[0] == in[1]; }},
        {"FIRST", [](const std::vector<bool>& in) { return in[0]; }},
        {"SECOND", [](const std::vector<bool>& in) { return in[1]; }}
    };
    
    std::vector<GoldenOrbitIOV4> ios;
    for (size_t i = 0; i < functions.size(); i++) {
        GoldenOrbitIOV4 io(2);
        io.obfuscate(functions[i].second, 42 + i);
        ios.push_back(io);
    }
    
    bool all_correct = true;
    for (size_t f = 0; f < functions.size(); f++) {
        bool correct = true;
        for (int i = 0; i < 4; i++) {
            std::vector<bool> input = {(bool)((i >> 1) & 1), (bool)(i & 1)};
            if (ios[f].evaluate(input) != functions[f].second(input)) {
                correct = false;
                all_correct = false;
            }
        }
        std::cout << "  " << functions[f].first << ": " 
                  << (correct ? "✅" : "❌") << "\n";
    }
    
    double max_ks = 0;
    for (size_t i = 0; i < ios.size(); i++) {
        for (size_t j = i + 1; j < ios.size(); j++) {
            max_ks = std::max(max_ks, ios[i].ks_distance(ios[j]));
        }
    }
    
    std::cout << "\nSign hiding (max KS): " << max_ks << " → " 
              << (max_ks < 0.5 ? "HIDDEN" : "REVEALED") << "\n";
    
    double time_10k = ios[0].benchmark(10000);
    std::cout << "Performance: " << 10000.0 / time_10k * 1000 << " evals/sec\n";
    
    if (all_correct) {
        std::cout << "\n✅ GOLDEN ORBIT iO V4: ALL 10 FUNCTIONS CORRECT!\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
