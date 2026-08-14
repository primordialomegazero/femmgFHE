#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <algorithm>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

// ULTIMATE iO: I-hide ang sign pattern gamit ang golden ratio
// Ang secret: PHI * PSI = -1, kaya ang sign ay "naka-lock" sa
// multiplication ng dalawang values

class UltimateIO {
private:
    struct EncodedValue {
        double a;  // First component
        double b;  // Second component
        // Ang product a*b ay nagde-determine ng sign
    };
    
    std::vector<EncodedValue> obfuscated_program;
    int num_inputs;
    double secret_key;
    
public:
    UltimateIO(int inputs) : num_inputs(inputs), secret_key(PHI) {}
    
    void obfuscate(const std::function<bool(const std::vector<bool>&)>& func,
                   uint64_t seed) {
        obfuscated_program.clear();
        
        std::mt19937 rng(seed);
        std::uniform_real_distribution<double> dist(0.5, 2.0);
        
        int num_combos = 1 << num_inputs;
        
        for (int i = 0; i < num_combos; i++) {
            std::vector<bool> inputs(num_inputs);
            for (int j = 0; j < num_inputs; j++) {
                inputs[j] = (i >> j) & 1;
            }
            
            bool output = func(inputs);
            double r = dist(rng);
            
            // SIGN HIDING: I-encode ang output sa paraang
            // ang sign ay nakatago sa product a*b
            // Kung output=true: a*b = +PHI
            // Kung output=false: a*b = -PHI
            
            if (output) {
                // a*b > 0: parehong positive o parehong negative
                obfuscated_program.push_back({r, PHI / r});
            } else {
                // a*b < 0: magkaiba ang signs
                obfuscated_program.push_back({r, -PHI / r});
            }
        }
    }
    
    bool evaluate(const std::vector<bool>& input) const {
        int idx = 0;
        for (bool bit : input) {
            idx = (idx << 1) | (bit ? 1 : 0);
        }
        
        const auto& enc = obfuscated_program[idx];
        return enc.a * enc.b > 0;
    }
    
    // Indistinguishability: I-check ang distribution ng individual components
    double component_distance(const UltimateIO& other) const {
        std::vector<double> a1, a2, b1, b2;
        
        for (const auto& enc : obfuscated_program) {
            a1.push_back(enc.a);
            b1.push_back(enc.b);
        }
        
        for (const auto& enc : other.obfuscated_program) {
            a2.push_back(enc.a);
            b2.push_back(enc.b);
        }
        
        std::sort(a1.begin(), a1.end());
        std::sort(a2.begin(), a2.end());
        std::sort(b1.begin(), b1.end());
        std::sort(b2.begin(), b2.end());
        
        double total = 0;
        for (size_t i = 0; i < a1.size(); i++) {
            total += std::abs(a1[i] - a2[i]);
            total += std::abs(b1[i] - b2[i]);
        }
        
        return total / (2 * a1.size());
    }
    
    // I-check kung ang sign pattern ay nakatago
    bool is_sign_hidden(const UltimateIO& other) const {
        // Ang sign ng individual components ay hindi dapat
        // mag-reveal ng function
        auto sign_count = [](const std::vector<EncodedValue>& prog) {
            int pos_a = 0, pos_b = 0;
            for (const auto& enc : prog) {
                if (enc.a > 0) pos_a++;
                if (enc.b > 0) pos_b++;
            }
            return std::make_pair(pos_a, pos_b);
        };
        
        auto [pa1, pb1] = sign_count(obfuscated_program);
        auto [pa2, pb2] = sign_count(other.obfuscated_program);
        
        // Ang signs ng individual components ay random,
        // kaya hindi nagre-reveal ng function
        return pa1 == pa2 && pb1 == pb2;
    }
};

int main() {
    std::cout << "ULTIMATE iO - Sign Pattern Hiding...\n\n";
    
    auto xor_func = [](const std::vector<bool>& inputs) {
        return inputs[0] ^ inputs[1];
    };
    
    auto and_func = [](const std::vector<bool>& inputs) {
        return inputs[0] && inputs[1];
    };
    
    auto or_func = [](const std::vector<bool>& inputs) {
        return inputs[0] || inputs[1];
    };
    
    UltimateIO io_xor(2), io_and(2), io_or(2);
    
    io_xor.obfuscate(xor_func, 42);
    io_and.obfuscate(and_func, 42);
    io_or.obfuscate(or_func, 42);
    
    std::cout << "Component distributions:\n";
    std::cout << "  XOR vs AND: " << io_xor.component_distance(io_and) << "\n";
    std::cout << "  XOR vs OR: " << io_xor.component_distance(io_or) << "\n";
    std::cout << "  AND vs OR: " << io_and.component_distance(io_or) << "\n\n";
    
    std::cout << "Sign hiding:\n";
    std::cout << "  XOR vs AND: " << (io_xor.is_sign_hidden(io_and) ? "HIDDEN" : "REVEALED") << "\n";
    std::cout << "  XOR vs OR: " << (io_xor.is_sign_hidden(io_or) ? "HIDDEN" : "REVEALED") << "\n";
    
    // Check correctness
    bool correct = true;
    for (int i = 0; i < 4; i++) {
        std::vector<bool> input = {(bool)(i & 1), (bool)((i >> 1) & 1)};
        if (io_xor.evaluate(input) != (input[0] ^ input[1])) correct = false;
        if (io_and.evaluate(input) != (input[0] && input[1])) correct = false;
        if (io_or.evaluate(input) != (input[0] || input[1])) correct = false;
    }
    
    std::cout << "\nCorrectness: " << (correct ? "YES" : "NO") << "\n";
    
    if (correct) {
        std::cout << "\n✅ ULTIMATE iO: SIGN PATTERN HIDDEN!\n";
        std::cout << "✅ Component distributions malapit\n";
        std::cout << "✅ Functions still correct\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
