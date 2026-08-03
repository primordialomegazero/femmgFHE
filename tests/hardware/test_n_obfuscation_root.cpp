#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <vector>
#include <algorithm>

const double PHI = 1.6180339887498948482;

double commutative_reconstruct(const std::vector<double>& v) {
    double n = v.size();
    double sum = 0, prod = 1, harm_sum = 0, sum_sq = 0;
    for (auto val : v) { 
        sum += val; prod *= (val+0.0001); 
        harm_sum += 1.0/(val+0.001); sum_sq += val*val; 
    }
    return 0.35*sum/n + 0.25*std::pow(prod,1.0/n) + 0.25*n/harm_sum + 0.15*std::sqrt(sum_sq/n);
}

int main() {
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> val(0.1, 0.9);
    
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "=== ROOT CAUSE DEBUG ===\n\n";
    
    // Create test data
    std::vector<double> data(5);
    for (int i = 0; i < 5; i++) data[i] = val(gen);
    
    std::cout << "Original values: ";
    for (auto v : data) std::cout << v << " ";
    std::cout << "\n";
    
    double rec_orig = commutative_reconstruct(data);
    std::cout << "Original reconstruction: " << rec_orig << "\n\n";
    
    // Now simulate obfuscation: 4-fold group shuffle
    // This should PRESERVE the multiset exactly
    std::vector<double> obfuscated;
    for (double v : data) {
        double part = v / 4.0;
        obfuscated.push_back(part);
        obfuscated.push_back(part);
        obfuscated.push_back(part);
        obfuscated.push_back(part);
    }
    // Shuffle groups of 4
    std::vector<std::vector<double>> groups(5);
    for (int i = 0; i < 5; i++) {
        groups[i] = {obfuscated[i*4], obfuscated[i*4+1], obfuscated[i*4+2], obfuscated[i*4+3]};
    }
    std::shuffle(groups.begin(), groups.end(), gen);
    std::vector<double> reconstructed(5);
    for (int i = 0; i < 5; i++) {
        reconstructed[i] = groups[i][0] + groups[i][1] + groups[i][2] + groups[i][3];
    }
    
    std::cout << "After obfuscation: ";
    for (auto v : reconstructed) std::cout << v << " ";
    std::cout << "\n";
    
    double rec_obf = commutative_reconstruct(reconstructed);
    std::cout << "Obfuscated reconstruction: " << rec_obf << "\n";
    std::cout << "DIFFERENCE: " << std::abs(rec_orig - rec_obf) << "\n\n";
    
    // Sort both to check multiset equality
    std::vector<double> s1 = data, s2 = reconstructed;
    std::sort(s1.begin(), s1.end());
    std::sort(s2.begin(), s2.end());
    std::cout << "Sorted original: ";
    for (auto v : s1) std::cout << v << " ";
    std::cout << "\n";
    std::cout << "Sorted obfuscated: ";
    for (auto v : s2) std::cout << v << " ";
    std::cout << "\n";
    
    double max_diff = 0;
    for (size_t i = 0; i < s1.size(); i++) {
        double d = std::abs(s1[i] - s2[i]);
        if (d > max_diff) max_diff = d;
    }
    std::cout << "Max element diff: " << max_diff;
    if (max_diff < 1e-10) std::cout << " (IDENTICAL)";
    else std::cout << " (DIFFERENT!)";
    std::cout << "\n\n";
    
    std::cout << "=== THE ISSUE ===\n";
    std::cout << "If multisets are IDENTICAL, commutative_reconstruct MUST be identical.\n";
    std::cout << "Commutative means: f(shuffle(v)) = f(v).\n";
    std::cout << "So rec_orig MUST equal rec_obf.\n";
    std::cout << "The test is: abs(rec_orig - rec_obf) < 1e-10.\n";
    std::cout << "But we got: " << std::abs(rec_orig - rec_obf) << "\n\n";
    
    // Check: is commutative_reconstruct truly commutative?
    std::cout << "=== IS COMMUTATIVE_RECONSTRUCT TRULY COMMUTATIVE? ===\n";
    std::vector<double> test1 = {0.1, 0.2, 0.3, 0.4, 0.5};
    std::vector<double> test2 = {0.5, 0.1, 0.3, 0.4, 0.2}; // Shuffled
    double r1 = commutative_reconstruct(test1);
    double r2 = commutative_reconstruct(test2);
    std::cout << "rec(0.1,0.2,0.3,0.4,0.5) = " << r1 << "\n";
    std::cout << "rec(0.5,0.1,0.3,0.4,0.2) = " << r2 << "\n";
    std::cout << "Diff: " << std::abs(r1-r2) << "\n";
    if (std::abs(r1-r2) < 1e-10) std::cout << "✓ TRULY COMMUTATIVE\n";
    else std::cout << "✗ NOT COMMUTATIVE — BUG IN RECONSTRUCTION!\n";
    
    return 0;
}
