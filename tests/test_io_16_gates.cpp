#include "src/io/spiral_io_final_complete.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace SpiralIO;

int main() {
    install_signal_handlers();
    
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  SPIRAL iO — 16 GATES (Error Handled)\n";
    std::cout << "===============================================================\n\n";

    FHEContext fhe;
    IOError init_err = fhe.init(8192, 60);
    if (init_err != IOError::None) {
        std::cerr << "FHE init failed: " << error_string(init_err) << "\n";
        return 1;
    }
    std::cout << "FHE initialized (RingDim=8192, Depth=60, Batch=512)\n";

    constexpr int NI = 2;
    constexpr int NG = 16;
    constexpr int TW = NI + NG;

    std::vector<std::vector<double>> in1(NG, std::vector<double>(TW, 0.0));
    std::vector<std::vector<double>> in2(NG, std::vector<double>(TW, 0.0));

    // XOR circuit (first 4 gates)
    in1[0][0] = 1.0; in2[0][1] = 1.0;
    in1[1][0] = 1.0; in2[1][2] = 1.0;
    in1[2][1] = 1.0; in2[2][2] = 1.0;
    in1[3][3] = 1.0; in2[3][4] = 1.0;

    // Gates 4-15: Buffer (NOT chains)
    for (int g = 4; g < NG; g++) {
        in1[g][NI + g - 1] = 1.0;
        in2[g][NI + g - 1] = 1.0;
    }

    std::cout << "Circuit built: " << NG << " gates, " << TW << " wires\n";
    std::cout << "Obfuscating...\n";

    auto t0 = std::chrono::high_resolution_clock::now();
    auto result = iOComplete::obfuscate(fhe, NI, NG, in1, in2);
    auto t1 = std::chrono::high_resolution_clock::now();

    if (!result.ok()) {
        std::cerr << "Obfuscation failed: " << error_string(result.error) << "\n";
        return 1;
    }

    std::cout << "Obfuscated: " << result.program.num_gates << " gates in "
              << std::chrono::duration<double>(t1 - t0).count() << "s\n\n";

    std::cout << "Evaluating...\n";
    std::cout << "  x y | Out  XOR\n";
    std::cout << "  " << std::string(16, '-') << "\n";

    int correct = 0;
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            if (!g_running) {
                std::cout << "\n[SHUTDOWN] Interrupted at x=" << x << " y=" << y << "\n";
                return 0;
            }

            auto cx = fhe.enc_all((double)x);
            auto cy = fhe.enc_all((double)y);
            int out_int = -1;
            auto eval_result = iOComplete::evaluate(fhe, result.program, {cx, cy}, out_int);

            if (!eval_result.ok()) {
                std::cerr << "Eval failed at x=" << x << " y=" << y 
                          << ": " << error_string(eval_result.error) << "\n";
                return 1;
            }

            bool exp = (x != y);
            if (out_int == (exp ? 1 : 0)) correct++;
            std::cout << "  " << x << " " << y << " | "
                      << std::setw(4) << out_int << "  " << exp
                      << "  " << (out_int == (exp ? 1 : 0) ? "OK" : "FAIL") << "\n";
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Total time: " << std::chrono::duration<double>(t2 - t0).count() << "s\n";
    std::cout << "  Status: " << (correct == 4 ? "PASS" : "FAIL") << "\n";
    
    return correct == 4 ? 0 : 1;
}
