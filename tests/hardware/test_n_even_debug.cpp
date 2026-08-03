#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <vector>
#include <algorithm>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

double commutative_reconstruct(const std::vector<double>& v) {
    double n = v.size();
    double sum = 0, prod = 1, harm_sum = 0, sum_sq = 0;
    for (auto val : v) { 
        sum += val; prod *= (val+0.0001); 
        harm_sum += 1.0/(val+0.001); sum_sq += val*val; 
    }
    return 0.35*sum/n + 0.25*std::pow(prod,1.0/n) + 0.25*n/harm_sum + 0.15*std::sqrt(sum_sq/n);
}

// Exact copy of the current obfuscate_round
std::vector<double> obfuscate_round(const std::vector<double>& input, int layer, uint64_t seed) {
    size_t n = input.size();
    std::vector<std::vector<double>> groups(n);
    for (size_t i = 0; i < n; i++) {
        double part = input[i] / 4.0;
        groups[i] = {part, part, part, part};
    }
    double scale = (layer % 2 == 0) ? PHI : PSI;
    for (size_t i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            groups[i][j] *= scale;
        }
    }
    std::mt19937 gen(seed + layer * 1000);
    std::shuffle(groups.begin(), groups.end(), gen);
    std::vector<double> output(n);
    for (size_t i = 0; i < n; i++) {
        output[i] = groups[i][0] + groups[i][1] + groups[i][2] + groups[i][3];
    }
    return output;
}

int main() {
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> val(0.1, 0.9);
    
    std::cout << std::fixed << std::setprecision(10);
    
    for (int N : {2, 4, 8}) {
        std::cout << "\n=== N=" << N << " ===\n";
        
        // Generate plaintext
        std::vector<double> plaintext(5);
        for (int i = 0; i < 5; i++) plaintext[i] = val(gen);
        
        std::cout << "Plaintext: ";
        for (auto v : plaintext) std::cout << v << " ";
        std::cout << "\n";
        
        double rec_before = commutative_reconstruct(plaintext);
        std::cout << "rec_before: " << rec_before << "\n";
        
        // Compute total product
        double total_product = 1.0;
        for (int layer = 0; layer < N; layer++) {
            total_product *= (layer % 2 == 0) ? PHI : PSI;
        }
        std::cout << "total_product: " << total_product << "\n";
        
        // Apply N rounds
        std::vector<double> current = plaintext;
        for (int layer = 0; layer < N; layer++) {
            current = obfuscate_round(current, layer, 42 + layer * 1000);
        }
        
        std::cout << "After " << N << " rounds (before normalize): ";
        for (auto v : current) std::cout << v << " ";
        std::cout << "\n";
        
        // Normalize
        std::vector<double> normalized = current;
        if (std::abs(total_product) > 1e-10) {
            for (size_t i = 0; i < normalized.size(); i++) {
                normalized[i] = normalized[i] / total_product;
            }
        }
        if (total_product < 0) {
            for (size_t i = 0; i < normalized.size(); i++) {
                if (normalized[i] < 0) normalized[i] = -normalized[i];
            }
        }
        
        std::cout << "After normalize: ";
        for (auto v : normalized) std::cout << v << " ";
        std::cout << "\n";
        
        double rec_after = commutative_reconstruct(normalized);
        std::cout << "rec_after: " << rec_after << "\n";
        std::cout << "diff: " << std::abs(rec_before - rec_after) << "\n";
        
        // Check: are multisets identical?
        std::vector<double> s1 = plaintext, s2 = normalized;
        std::sort(s1.begin(), s1.end());
        std::sort(s2.begin(), s2.end());
        std::cout << "Sorted plaintext: ";
        for (auto v : s1) std::cout << v << " ";
        std::cout << "\n";
        std::cout << "Sorted normalized: ";
        for (auto v : s2) std::cout << v << " ";
        std::cout << "\n";
        double maxd = 0;
        for (size_t i = 0; i < s1.size(); i++) {
            double d = std::abs(s1[i] - s2[i]);
            if (d > maxd) maxd = d;
        }
        std::cout << "Max element diff: " << maxd;
        if (maxd < 1e-10) std::cout << " (IDENTICAL)";
        else std::cout << " (DIFFERENT!)";
        std::cout << "\n";
        
        // Check: does obfuscate_round preserve values exactly?
        std::cout << "\n=== Check round 0 ===\n";
        std::vector<double> round0 = obfuscate_round(plaintext, 0, 42);
        std::cout << "After round 0 (scaled by PHI): ";
        for (auto v : round0) std::cout << v << " ";
        std::cout << "\n";
        std::cout << "Expected (plaintext * PHI): ";
        for (auto v : plaintext) std::cout << (v * PHI) << " ";
        std::cout << "\n";
        std::sort(round0.begin(), round0.end());
        std::vector<double> expected;
        for (auto v : plaintext) expected.push_back(v * PHI);
        std::sort(expected.begin(), expected.end());
        double rd=0;
        for (size_t i=0;i<5;i++){double d=std::abs(round0[i]-expected[i]);if(d>rd)rd=d;}
        std::cout << "Max diff: " << rd;
        if (rd < 1e-10) std::cout << " (OK)";
        else std::cout << " (ROUND ERROR!)";
        std::cout << "\n";
    }
    
    return 0;
}
