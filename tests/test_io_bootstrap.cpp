#include "src/io/golden_io_bootstrap.h"
#include <iostream>

int main() {
    std::cout << "=== UNLIMITED iO BOOTSTRAP ===\n\n";

    auto xor_func = [](const std::vector<bool>& in) {
        return in[0] ^ in[1];
    };

    GoldenIOBootstrap::UnlimitedIO io;
    io.obfuscate(xor_func, 2, 42);

    std::cout << "--- Depth Test (1000 evaluations) ---\n";
    int correct = 0;
    for (int i = 0; i < 1000; i++) {
        bool a = i % 2;
        bool b = (i / 2) % 2;
        std::vector<bool> input = {a, b};
        bool out = io.evaluate_unlimited(input);
        bool exp = a ^ b;
        if (out == exp) correct++;
    }

    std::cout << "Correct: " << correct << "/1000\n";
    std::cout << "Bootstrap phases used: " << io.get_bootstrap_phase() << "\n";
    std::cout << (correct == 1000 ? "UNLIMITED iO PASS ✅\n" : "FAIL ❌\n");

    return 0;
}
