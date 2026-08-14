#include "../src/io/golden_io_bootstrap.h"
#include <iostream>
#include <vector>

using namespace GoldenIOBootstrap;

int main() {
    std::cout << "QUANTUM BOOTSTRAP DEBUG\n\n";
    
    QuantumUnlimitedIO qio;
    
    // I-check ang quantum interference
    for (int i = 0; i < 10; i++) {
        std::vector<bool> input = {(bool)((i >> 1) & 1), (bool)(i & 1)};
        bool result = qio.evaluate_unlimited(input);
        bool expected = input[0] ^ input[1];
        
        std::cout << "  Eval " << i << ": input(" << input[0] << "," << input[1] 
                  << ") → " << result << " (expected " << expected << ")\n";
    }
    
    return 0;
}
