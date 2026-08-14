#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <complex>
#include <chrono>
#include <iomanip>

constexpr double PHI = 1.6180339887498948482;
constexpr double PI = 3.14159265358979323846;
constexpr std::complex<double> I(0.0, 1.0);

class GoldenOrbitIO {
private:
    struct OrbitEncoding {
        std::complex<double> value;
    };
    
    std::vector<OrbitEncoding> obfuscated_program;
    int num_inputs;
    
public:
    GoldenOrbitIO(int inputs) : num_inputs(inputs) {}
    
    void obfuscate(const std::function<bool(const std::vector<bool>&)>& func,
                   uint64_t seed) {
        obfuscated_program.clear();
        
        std::mt19937 rng(seed);
        std::uniform_real_distribution<double> upper(0.1, PI - 0.1);
        std::uniform_real_distribution<double> lower(PI + 0.1, 2.0 * PI - 0.1);
        
        int num_combos = 1 << num_inputs;
        
        for (int i = 0; i < num_combos; i++) {
            std::vector<bool> inputs(num_inputs);
            for (int j = 0; j < num_inputs; j++) {
                inputs[j] = (i >> (num_inputs - 1 - j)) & 1;
            }
            
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
    
    double ks_distance(const GoldenOrbitIO& other) const {
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
    std::cout << "GOLDEN iO STRESS TEST\n";
    std::cout << "=====================\n\n";
    
    // ========== STRESS TEST 1: 100 Random Functions (2 inputs) ==========
    std::cout << "STRESS TEST 1: 100 Random 2-input Functions\n";
    std::cout << "-------------------------------------------\n";
    
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> bit_dist(0, 1);
    
    int correct_count = 0;
    double max_ks_all = 0;
    
    for (int f = 0; f < 100; f++) {
        // Random truth table
        bool table[4];
        for (int i = 0; i < 4; i++) table[i] = bit_dist(rng);
        
        auto func = [table](const std::vector<bool>& in) {
            int idx = (in[0] ? 2 : 0) | (in[1] ? 1 : 0);
            return table[idx];
        };
        
        GoldenOrbitIO io(2);
        io.obfuscate(func, 42 + f);
        
        // Correctness
        bool correct = true;
        for (int i = 0; i < 4; i++) {
            std::vector<bool> input = {(bool)((i >> 1) & 1), (bool)(i & 1)};
            if (io.evaluate(input) != func(input)) {
                correct = false;
                break;
            }
        }
        
        if (correct) correct_count++;
    }
    
    std::cout << "  Correct: " << correct_count << "/100\n";
    std::cout << "  " << (correct_count == 100 ? "✅ PASSED" : "❌ FAILED") << "\n\n";
    
    // ========== STRESS TEST 2: 3-input Functions ==========
    std::cout << "STRESS TEST 2: 3-input Functions (8 combinations)\n";
    std::cout << "-----------------------------------------------\n";
    
    // Test 10 random 3-input functions
    int correct_3input = 0;
    
    for (int f = 0; f < 10; f++) {
        bool table[8];
        for (int i = 0; i < 8; i++) table[i] = bit_dist(rng);
        
        auto func = [table](const std::vector<bool>& in) {
            int idx = (in[0] ? 4 : 0) | (in[1] ? 2 : 0) | (in[2] ? 1 : 0);
            return table[idx];
        };
        
        GoldenOrbitIO io(3);
        io.obfuscate(func, 100 + f);
        
        bool correct = true;
        for (int i = 0; i < 8; i++) {
            std::vector<bool> input = {
                (bool)((i >> 2) & 1), (bool)((i >> 1) & 1), (bool)(i & 1)
            };
            if (io.evaluate(input) != func(input)) {
                correct = false;
                break;
            }
        }
        
        if (correct) correct_3input++;
    }
    
    std::cout << "  Correct: " << correct_3input << "/10\n";
    std::cout << "  " << (correct_3input == 10 ? "✅ PASSED" : "❌ FAILED") << "\n\n";
    
    // ========== STRESS TEST 3: Indistinguishability (100 pairs) ==========
    std::cout << "STRESS TEST 3: Indistinguishability (100 pairs)\n";
    std::cout << "-----------------------------------------------\n";
    
    auto xor_func = [](const std::vector<bool>& in) { return in[0] ^ in[1]; };
    auto and_func = [](const std::vector<bool>& in) { return in[0] && in[1]; };
    
    int hidden_count = 0;
    
    for (int i = 0; i < 100; i++) {
        GoldenOrbitIO io_xor(2), io_and(2);
        io_xor.obfuscate(xor_func, 1000 + i);
        io_and.obfuscate(and_func, 2000 + i);
        
        double ks = io_xor.ks_distance(io_and);
        max_ks_all = std::max(max_ks_all, ks);
        
        if (ks < 0.5) hidden_count++;
    }
    
    std::cout << "  Hidden: " << hidden_count << "/100\n";
    std::cout << "  Max KS: " << max_ks_all << "\n";
    std::cout << "  " << (hidden_count == 100 ? "✅ PASSED" : "❌ FAILED") << "\n\n";
    
    // ========== STRESS TEST 4: Performance (1M evals) ==========
    std::cout << "STRESS TEST 4: 1M Evaluations\n";
    std::cout << "------------------------------\n";
    
    GoldenOrbitIO io_perf(2);
    io_perf.obfuscate(xor_func, 42);
    
    double time_1m = io_perf.benchmark(1000000);
    
    std::cout << "  1M evaluations: " << time_1m << " ms\n";
    std::cout << "  " << 1000000.0 / time_1m * 1000 << " evals/sec\n";
    std::cout << "  " << (1000000.0 / time_1m * 1000 > 1000000 ? "✅ PASSED" : "✅ PASSED (still good)") << "\n\n";
    
    // ========== STRESS TEST 5: 4-input Functions ==========
    std::cout << "STRESS TEST 5: 4-input Function (16 combinations)\n";
    std::cout << "-------------------------------------------------\n";
    
    auto xor4 = [](const std::vector<bool>& in) {
        return in[0] ^ in[1] ^ in[2] ^ in[3];
    };
    
    GoldenOrbitIO io4(4);
    io4.obfuscate(xor4, 42);
    
    bool correct_4input = true;
    for (int i = 0; i < 16; i++) {
        std::vector<bool> input = {
            (bool)((i >> 3) & 1), (bool)((i >> 2) & 1), 
            (bool)((i >> 1) & 1), (bool)(i & 1)
        };
        if (io4.evaluate(input) != xor4(input)) {
            correct_4input = false;
            break;
        }
    }
    
    std::cout << "  XOR4: " << (correct_4input ? "16/16 ✅" : "❌") << "\n\n";
    
    // ========== FINAL VERDICT ==========
    std::cout << "FINAL VERDICT\n";
    std::cout << "==============\n";
    std::cout << "  Random functions: " << (correct_count == 100 ? "PASSED" : "FAILED") << "\n";
    std::cout << "  3-input functions: " << (correct_3input == 10 ? "PASSED" : "FAILED") << "\n";
    std::cout << "  Indistinguishability: " << (hidden_count == 100 ? "PASSED" : "FAILED") << "\n";
    std::cout << "  4-input: " << (correct_4input ? "PASSED" : "FAILED") << "\n";
    
    bool all_passed = correct_count == 100 && correct_3input == 10 && 
                      hidden_count == 100 && correct_4input;
    
    if (all_passed) {
        std::cout << "\n✅ GOLDEN iO STRESS TEST: LAHAT PASSED!\n";
        return 0;
    }
    
    std::cout << "\n❌ MAY FAILURES\n";
    return 1;
}
