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
    std::cout << "  SPIRAL iO — SCALED TEST (8 GATES, FIXED DEPTH)\n";
    std::cout << "===============================================================\n\n";

    FHEContext fhe;
    fhe.init(8192, 60);  // Increased: RingDim=8192, Depth=60
    std::cout << "FHE initialized (RingDim=8192, Depth=60, Batch=512)\n";

    constexpr int NI = 2;
    constexpr int NG = 8;
    constexpr int TW = NI + NG;

    std::vector<std::vector<double>> in1(NG, std::vector<double>(TW, 0.0));
    std::vector<std::vector<double>> in2(NG, std::vector<double>(TW, 0.0));

    in1[0][0] = 1.0; in2[0][1] = 1.0;
    in1[1][0] = 1.0; in2[1][2] = 1.0;
    in1[2][1] = 1.0; in2[2][2] = 1.0;
    in1[3][3] = 1.0; in2[3][4] = 1.0;

    for (int g = 4; g < NG; g++) {
        in1[g][NI + g - 1] = 1.0;
        in2[g][NI + g - 1] = 1.0;
    }

    std::cout << "Circuit built: " << NG << " gates, " << TW << " wires\n";
    std::cout << "Obfuscating...\n";

    auto prog = iOComplete::obfuscate(fhe, NI, NG, in1, in2);
    std::cout << "Obfuscated: " << prog.num_gates << " gates\n\n";

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
            if ((out > 0.5) == exp) correct++;
            std::cout << "  " << x << " " << y << " | "
                      << std::setw(4) << out << "  " << exp
                      << "  " << ((out > 0.5) == exp ? "OK" : "FAIL") << "\n";
        }
    }

    std::cout << "\n  Correct: " << correct << "/4\n";
    return 0;
}
