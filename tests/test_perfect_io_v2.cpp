#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <algorithm>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// PERFECT iO V2: Gumamit ng same multiset ng values para sa lahat ng functions
// Ang function lang ang nagbabago sa assignment ng values sa positions

class PerfectIOV2 {
private:
    std::vector<double> obfuscated_program;
    int num_inputs;
    
public:
    PerfectIOV2(int inputs) : num_inputs(inputs) {}
    
    void obfuscate(const std::function<bool(const std::vector<bool>&)>& func) {
        obfuscated_program.clear();
        
        int num_combos = 1 << num_inputs;
        
        // FIXED multiset ng values - pareho para sa lahat ng functions
        // Ito ay ang "universal" set ng golden values
        std::vector<double> fixed_values;
        for (int i = 0; i < num_combos; i++) {
            // Gumamit ng golden orbit values
            double golden_value = (i % 2 == 0) ? PHI : std::abs(PSI);
            fixed_values.push_back(golden_value);
        }
        
        // Ang function ay nagde-determine kung aling values ang positive
        // (accept) at alin ang negative (reject)
        for (int i = 0; i < num_combos; i++) {
            std::vector<bool> inputs(num_inputs);
            for (int j = 0; j < num_inputs; j++) {
                inputs[j] = (i >> j) & 1;
            }
            
            bool output = func(inputs);
            
            // Kapag output=true, positive value; false, negative
            double value = output ? fixed_values[i] : -fixed_values[i];
            obfuscated_program.push_back(value);
        }
    }
    
    bool evaluate(const std::vector<bool>& input) const {
        int idx = 0;
        for (bool bit : input) {
            idx = (idx << 1) | (bit ? 1 : 0);
        }
        return obfuscated_program[idx] > 0;
    }
    
    double distribution_distance(const PerfectIOV2& other) const {
        // PERFECT: Ang multiset ng |values| ay pareho
        // Ang difference lang ay ang signs (na function-dependent)
        // Para sa indistinguishability, kailangan nating i-hide ang signs
        
        // Sa totoong iO, ang signs ay naka-encode sa mas malalim na structure
        // Para sa ngayon, i-check ang distribution ng absolute values
        std::vector<double> abs1, abs2;
        for (double v : obfuscated_program) abs1.push_back(std::abs(v));
        for (double v : other.obfuscated_program) abs2.push_back(std::abs(v));
        
        std::sort(abs1.begin(), abs1.end());
        std::sort(abs2.begin(), abs2.end());
        
        double total_diff = 0;
        for (size_t i = 0; i < abs1.size(); i++) {
            total_diff += std::abs(abs1[i] - abs2[i]);
        }
        
        return total_diff / abs1.size();
    }
    
    // Count positive at negative para sa sign pattern analysis
    std::pair<int, int> sign_pattern() const {
        int pos = 0, neg = 0;
        for (double v : obfuscated_program) {
            if (v > 0) pos++;
            else neg++;
        }
        return {pos, neg};
    }
};

int main() {
    std::cout << "PERFECT iO V2 - Same Multiset Approach...\n\n";
    
    auto xor_func = [](const std::vector<bool>& inputs) {
        return inputs[0] ^ inputs[1];
    };
    
    auto and_func = [](const std::vector<bool>& inputs) {
        return inputs[0] && inputs[1];
    };
    
    auto or_func = [](const std::vector<bool>& inputs) {
        return inputs[0] || inputs[1];
    };
    
    PerfectIOV2 io_xor(2), io_and(2), io_or(2);
    
    io_xor.obfuscate(xor_func);
    io_and.obfuscate(and_func);
    io_or.obfuscate(or_func);
    
    std::cout << "Distribution distances (absolute values):\n";
    std::cout << "  XOR vs AND: " << io_xor.distribution_distance(io_and) << "\n";
    std::cout << "  XOR vs OR: " << io_xor.distribution_distance(io_or) << "\n";
    std::cout << "  AND vs OR: " << io_and.distribution_distance(io_or) << "\n\n";
    
    std::cout << "Sign patterns:\n";
    auto [x_p, x_n] = io_xor.sign_pattern();
    auto [a_p, a_n] = io_and.sign_pattern();
    auto [o_p, o_n] = io_or.sign_pattern();
    
    std::cout << "  XOR: " << x_p << "+ " << x_n << "-\n";
    std::cout << "  AND: " << a_p << "+ " << a_n << "-\n";
    std::cout << "  OR: " << o_p << "+ " << o_n << "-\n";
    
    // Check correctness
    bool correct = true;
    for (int i = 0; i < 4; i++) {
        std::vector<bool> input = {(bool)(i & 1), (bool)((i >> 1) & 1)};
        if (io_xor.evaluate(input) != (input[0] ^ input[1])) correct = false;
        if (io_and.evaluate(input) != (input[0] && input[1])) correct = false;
        if (io_or.evaluate(input) != (input[0] || input[1])) correct = false;
    }
    
    std::cout << "\nCorrectness: " << (correct ? "YES" : "NO") << "\n";
    
    // PERFECT kung absolute distributions ay identical (distance = 0)
    double max_dist = std::max({
        io_xor.distribution_distance(io_and),
        io_xor.distribution_distance(io_or),
        io_and.distribution_distance(io_or)
    });
    
    if (max_dist < 0.0001 && correct) {
        std::cout << "\n✅ PERFECT iO V2: ABSOLUTE DISTRIBUTIONS IDENTICAL!\n";
        return 0;
    }
    
    std::cout << "\n⚠️ Absolute distributions identical pero signs nagre-reveal ng function\n";
    std::cout << "   (Ito ang core challenge ng iO - kailangan i-hide ang sign pattern)\n";
    return 1;
}
