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
    
    std::cout << std::fixed << std::setprecision(15);
    
    // Test with 100 values (like the main test)
    int n_vals = 100;
    
    for (int N : {2, 4, 8, 5}) {
        std::cout << "\n=== N=" << N << " with " << n_vals << " values ===\n";
        
        std::vector<double> plaintext(n_vals);
        for (int i = 0; i < n_vals; i++) plaintext[i] = val(gen);
        
        double rec_before = commutative_reconstruct(plaintext);
        
        // Compute total product
        double total_product = 1.0;
        for (int layer = 0; layer < N; layer++) {
            total_product *= (layer % 2 == 0) ? PHI : PSI;
        }
        
        // Apply N rounds
        std::vector<double> current = plaintext;
        for (int layer = 0; layer < N; layer++) {
            current = obfuscate_round(current, layer, 42 + layer * 1000);
        }
        
        // Normalize
        if (std::abs(total_product) > 1e-10) {
            for (size_t i = 0; i < current.size(); i++) {
                current[i] = current[i] / total_product;
            }
        }
        if (total_product < 0) {
            for (size_t i = 0; i < current.size(); i++) {
                if (current[i] < 0) current[i] = -current[i];
            }
        }
        
        double rec_after = commutative_reconstruct(current);
        double diff = std::abs(rec_before - rec_after);
        
        std::cout << "rec_before: " << std::setprecision(10) << rec_before << "\n";
        std::cout << "rec_after:  " << rec_after << "\n";
        std::cout << "diff:       " << std::setprecision(15) << diff << "\n";
        std::cout << "diff < 1e-10: " << (diff < 1e-10 ? "YES" : "NO") << "\n";
        
        // Check sorted values
        std::vector<double> s1=plaintext, s2=current;
        std::sort(s1.begin(),s1.end()); std::sort(s2.begin(),s2.end());
        double maxd=0;
        for(size_t i=0;i<s1.size();i++){double d=std::abs(s1[i]-s2[i]);if(d>maxd)maxd=d;}
        std::cout << "max element diff: " << maxd << "\n";
    }
    
    return 0;
}
