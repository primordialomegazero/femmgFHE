#include <iostream>
#include <random>
#include <set>

int main() {
    std::cout << "PRNG Distribution Analysis\n\n";
    
    // Test: xorshift PRNG sa iba't ibang seeds
    auto xorshift = [](uint64_t& state) {
        state ^= (state << 13);
        state ^= (state >> 7);
        state ^= (state << 17);
        return state;
    };
    
    // I-check kung ilang unique values ang na-produce
    std::set<long> unique_values;
    
    for (int i = 0; i < 100; i++) {
        uint64_t state = 1000000 + i;
        xorshift(state);
        long val = state % 1000000000;
        unique_values.insert(val);
    }
    
    std::cout << "xorshift with 100 consecutive nonces:\n";
    std::cout << "  Unique values: " << unique_values.size() << "/100\n\n";
    
    // Test: gamitin ang random_device
    std::random_device rd;
    std::set<long> unique_rd;
    
    for (int i = 0; i < 100; i++) {
        uint64_t val = rd();
        unique_rd.insert(val % 1000000000);
    }
    
    std::cout << "random_device with 100 samples:\n";
    std::cout << "  Unique values: " << unique_rd.size() << "/100\n\n";
    
    // Ang issue: xorshift ay deterministic sa seed
    // Kapag malapit ang nonces, malapit din ang states
    // Kaya ang u polynomial ay nagre-repeat
    
    // FIX: Gumamit ng separate mt19937_64 na naka-seed sa random_device
    std::mt19937_64 rng(rd());
    std::set<long> unique_mt;
    
    for (int i = 0; i < 100; i++) {
        uint64_t val = rng();
        unique_mt.insert(val % 1000000000);
    }
    
    std::cout << "mt19937_64 with random_device seed:\n";
    std::cout << "  Unique values: " << unique_mt.size() << "/100\n";
    
    return 0;
}
