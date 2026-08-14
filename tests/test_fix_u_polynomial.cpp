#include <iostream>
#include <set>
#include <random>

int main() {
    std::cout << "U Polynomial Distribution Fix\n\n";
    
    // Current: state % 3 - 1 → {-1, 0, 1}
    // Problem: 3 lang na values
    
    // FIX 1: Gumamit ng mas malawak na range
    // state % 7 - 3 → {-3, -2, -1, 0, 1, 2, 3}
    
    std::set<long> current_vals, fixed_vals;
    
    uint64_t state = 42;
    
    for (int i = 0; i < 1000; i++) {
        state ^= (state << 13);
        state ^= (state >> 7);
        state ^= (state << 17);
        
        long current = (state % 3) - 1;
        long fixed = (state % 7) - 3;
        
        current_vals.insert(current);
        fixed_vals.insert(fixed);
    }
    
    std::cout << "Current u coefficients (state % 3 - 1):\n";
    std::cout << "  Values: ";
    for (long v : current_vals) std::cout << v << " ";
    std::cout << "\n  Count: " << current_vals.size() << "\n\n";
    
    std::cout << "Fixed u coefficients (state % 7 - 3):\n";
    std::cout << "  Values: ";
    for (long v : fixed_vals) std::cout << v << " ";
    std::cout << "\n  Count: " << fixed_vals.size() << "\n\n";
    
    // FIX 2: Gumamit ng mt19937 para sa u
    std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<int> dist(-1, 1);
    
    std::set<int> mt_vals;
    for (int i = 0; i < 1000; i++) {
        mt_vals.insert(dist(rng));
    }
    
    std::cout << "mt19937 u coefficients (uniform -1 to 1):\n";
    std::cout << "  Values: ";
    for (int v : mt_vals) std::cout << v << " ";
    std::cout << "\n  Count: " << mt_vals.size() << "\n";
    
    return 0;
}
