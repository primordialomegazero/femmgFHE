#include "../src/fhe/golden_quantum_fhe.h"
#include <iostream>

int main() {
    GoldenFHE::init_ring();
    
    long golden_plain = static_cast<long>(GoldenFHE::Q / GoldenFHE::PHI);
    long wrong_value = 414125843;
    long threshold = static_cast<long>(GoldenFHE::Q / (2 * GoldenFHE::PHI));
    
    std::cout << "golden_plain: " << golden_plain << "\n";
    std::cout << "wrong_value: " << wrong_value << "\n";
    std::cout << "threshold: " << threshold << "\n";
    std::cout << "Q: " << GoldenFHE::Q << "\n\n";
    
    std::cout << "wrong_value - golden_plain: " << wrong_value - golden_plain << "\n";
    std::cout << "wrong_value + golden_plain mod Q: " << (wrong_value + golden_plain) % GoldenFHE::Q << "\n";
    std::cout << "golden_plain * 1.25: " << (long)(golden_plain * 1.25) << "\n";
    
    return 0;
}
