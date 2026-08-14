#include <iostream>
#include <vector>

int main() {
    // Sa V3 class, ang evaluate ay:
    // int idx = 0;
    // for (bool bit : input) {
    //     idx = (idx << 1) | (bit ? 1 : 0);
    // }
    
    // Test indexing para sa 2 inputs
    std::vector<std::vector<bool>> inputs = {
        {false, false},  // 00
        {true, false},   // 10
        {false, true},   // 01
        {true, true}     // 11
    };
    
    std::cout << "Evaluation indexing:\n";
    for (auto& input : inputs) {
        int idx = 0;
        for (bool bit : input) {
            idx = (idx << 1) | (bit ? 1 : 0);
        }
        std::cout << "  input(" << input[0] << "," << input[1] << ") → idx=" << idx << "\n";
    }
    
    // Sa obfuscate, ang indexing ay:
    // inputs[j] = (i >> j) & 1; → j=0: LSB, j=1: bit 1
    
    std::cout << "\nObfuscate indexing:\n";
    for (int i = 0; i < 4; i++) {
        std::vector<bool> inputs(2);
        for (int j = 0; j < 2; j++) {
            inputs[j] = (i >> j) & 1;
        }
        std::cout << "  i=" << i << " → inputs(" << inputs[0] << "," << inputs[1] << ")\n";
    }
    
    // ANG PROBLEMA: Sa obfuscate, ang input order ay (i>>0, i>>1)
    // Sa evaluate, ang input order ay (bit0, bit1)
    // Kaya may MISMATCH kapag ang input ay {true, false} vs {false, true}
    
    return 0;
}
