// ================================================================
// SPIRAL iO — SCALED TEST: 100 GATES
// ================================================================
#include "src/io/spiral_io_final_complete.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace SpiralIO;

int main() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  SPIRAL iO — SCALED TEST (100 GATES)\n";
    std::cout << "===============================================================\n\n";

    auto t0 = std::chrono::high_resolution_clock::now();

    FHEContext fhe;
    fhe.init(16384, 200);
    std::cout << "FHE initialized (RingDim=16384, Depth=200, Batch=1024)\n";

    constexpr int NI = 2;
    constexpr int NG = 100;
    constexpr int TW = NI + NG;

    std::vector<std::vector<double>> in1(NG, std::vector<double>(TW, 0.0));
    std::vector<std::vector<double>> in2(NG, std::vector<double>(TW, 0.0));

    // XOR circuit (4 gates)
    in1[0][0] = 1.0; in2[0][1] = 1.0;    // NAND(x, y)
    in1[1][0] = 1.0; in2[1][2] = 1.0;    // NAND(x, gate0)
    in1[2][1] = 1.0; in2[2][2] = 1.0;    // NAND(y, gate0)
    in1[3][3] = 1.0; in2[3][4] = 1.0;    // NAND(gate1, gate2)

    // Gates 4-99: Redundant chain
    for (int g = 4; g < NG; g++) {
        in1[g][NI + g - 1] = 1.0;
        in2[g][NI + g - 1] = 1.0;
    }

    std::cout << "Circuit built: " << NG << " gates, " << TW << " wires\n";

    auto prog = iOComplete::obfuscate(fhe, NI, NG, in1, in2);
    std::cout << "Obfuscated: " << prog.num_gates << " gates, coefficients encrypted\n\n";

    std::cout << "Truth table:\n";
    std::cout << "  x y | Out  XOR\n";
    std::cout << "  " << std::string(16, '-') << "\n";

    int output_slot = NI + NG - 1;
    int correct = 0;

    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            auto cx = fhe.enc_all((double)x);
            auto cy = fhe.enc_all((double)y);
            auto ct_out = iOComplete::evaluate(fhe, prog, {cx, cy});
            double out = fhe.dec_slot(ct_out, output_slot);
            bool exp = (x != y);
            bool result = (out > 0.5);
            if (result == exp) correct++;
            std::cout << "  " << x << " " << y << " | "
                      << std::setw(8) << out << "  " << exp
                      << "  " << (result == exp ? "✅" : "❌") << "\n";
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();

    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Total time: " << secs << "s\n";

    if (correct == 4) {
        std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║  ✅ iO SCALED TO 100 GATES — WORKING ✅                    ║\n";
        std::cout << "║  Encrypted coefficients, EvalSum routing, correct output.  ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    } else {
        std::cout << "\n  ❌ FAILED: " << correct << "/4 correct\n";
    }

    std::cout << "\n===============================================================\n";
    return 0;
}
