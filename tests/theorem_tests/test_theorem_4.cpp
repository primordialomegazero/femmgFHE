#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cassert>

double commutative_reconstruct(const std::vector<double>& v) {
    double n = v.size();
    double sum = 0, prod = 1, harm_sum = 0, sum_sq = 0;
    for (auto val : v) { sum += val; prod *= (val+0.0001); harm_sum += 1.0/(val+0.001); sum_sq += val*val; }
    return 0.35*sum/n + 0.25*std::pow(prod,1.0/n) + 0.25*n/harm_sum + 0.15*std::sqrt(sum_sq/n);
}

int main() {
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> dist(0, 1);
    
    int passed = 0, total = 1000;
    
    for (int t = 0; t < total; t++) {
        int n = 50 + (t % 200);
        std::vector<double> A(n), B(n);
        for (int i = 0; i < n; i++) { A[i] = dist(gen); B[i] = A[i]; }
        std::shuffle(B.begin(), B.end(), gen);
        
        double recA = commutative_reconstruct(A);
        double recB = commutative_reconstruct(B);
        
        if (std::abs(recA - recB) < 1e-10) passed++;
        else {
            std::cout << "FAIL: t=" << t << " diff=" << std::abs(recA-recB) << "\n";
            return 1;
        }
    }
    
    std::cout << passed << "/" << total << "\n";
    return 0;
}
