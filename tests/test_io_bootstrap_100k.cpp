#include "src/io/golden_io_bootstrap.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "=== UNLIMITED iO BOOTSTRAP 100K ===\n\n";

    auto xor_func = [](const std::vector<bool>& in) {
        return in[0] ^ in[1];
    };

    GoldenIOBootstrap::UnlimitedIO io;
    io.obfuscate(xor_func, 2, 42);

    const int TARGET = 100000;
    int correct = 0;
    auto t0 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TARGET; i++) {
        bool a = i % 2;
        bool b = (i / 2) % 2;
        std::vector<bool> input = {a, b};
        bool out = io.evaluate_unlimited(input);
        bool exp = a ^ b;
        if (out == exp) correct++;

        if ((i + 1) % 10000 == 0) {
            auto t1 = std::chrono::high_resolution_clock::now();
            double secs = std::chrono::duration<double>(t1 - t0).count();
            std::cout << "  [" << (i+1) << "/" << TARGET << "] correct=" << correct
                      << " rate=" << ((i+1) / secs) << " ops/s\n";
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();

    std::cout << "\nCorrect: " << correct << "/" << TARGET << "\n";
    std::cout << "Time: " << secs << "s\n";
    std::cout << "Rate: " << (TARGET / secs) << " ops/s\n";
    std::cout << "Bootstrap phases: " << io.get_bootstrap_phase() << "\n";
    std::cout << (correct == TARGET ? "UNLIMITED iO PASS ✅\n" : "FAIL ❌\n");

    return 0;
}
