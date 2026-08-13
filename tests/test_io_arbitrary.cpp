#include "src/golden_lwe/golden_io_arbitrary.h"
#include <iostream>

int main() {
    std::cout << "=== ARBITRARY FUNCTION iO + QUANTUM ===\n\n";

    // Arbitrary function: XOR ng 2 inputs
    auto xor_func = [](const std::vector<bool>& in) {
        return in[0] ^ in[1];
    };

    GoldenIOArbitrary::ArbitraryFunctionIO io(2, xor_func);
    io.obfuscate(42);

    std::cout << "--- Arbitrary XOR Function ---\n";
    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            std::vector<bool> input = {bool(a), bool(b)};
            bool out = io.evaluate(input);
            bool exp = a ^ b;
            std::cout << "evaluate(" << a << "," << b << ") = " << out
                      << " exp " << exp << "\n";
        }
    }

    // Quantum IO test
    auto and_func = [](const std::vector<bool>& in) {
        return in[0] && in[1];
    };

    GoldenIOArbitrary::QuantumIO qio(2, and_func);
    qio.obfuscate(43);

    std::cout << "\n--- Quantum AND Function ---\n";
    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            std::vector<bool> input = {bool(a), bool(b)};
            bool out = qio.evaluate(input);
            bool exp = a && b;
            std::cout << "evaluate(" << a << "," << b << ") = " << out
                      << " exp " << exp << "\n";
        }
    }

    return 0;
}
