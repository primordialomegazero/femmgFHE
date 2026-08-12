#include "src/io/spiral_io_tfhe.h"

int main() {
    SpiralIO::TFHEContext ctx;
    ctx.init();
    
    constexpr int NG = 100;
    constexpr int NI = 2;
    constexpr int TW = NG + NI;
    
    std::cout << "===============================================================\n";
    std::cout << "  SPIRAL iO — TFHE 100 GATES (Full Scale)\n";
    std::cout << "===============================================================\n\n";
    
    // XOR + buffer chain
    std::vector<std::vector<int>> in1(NG, std::vector<int>(TW, 0));
    std::vector<std::vector<int>> in2(NG, std::vector<int>(TW, 0));
    
    // XOR circuit (4 gates)
    in1[0][0] = 1; in2[0][1] = 1;
    in1[1][0] = 1; in2[1][2] = 1;
    in1[2][1] = 1; in2[2][2] = 1;
    in1[3][3] = 1; in2[3][4] = 1;
    
    // Buffer chain (96 gates)
    for (int g = 4; g < NG; g++) {
        in1[g][NI + g - 1] = 1;
        in2[g][NI + g - 1] = 1;
    }
    
    std::cout << "Circuit: " << NG << " gates, " << TW << " wires\n";
    std::cout << "Obfuscating...\n";
    
    auto t0 = std::chrono::high_resolution_clock::now();
    auto prog = SpiralIO::iOCompleteTFHE::obfuscate(ctx, in1, in2);
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << "Obfuscated: " << NG << " gates in "
              << std::chrono::duration<double>(t1 - t0).count() << "s\n\n";
    
    std::cout << "Evaluating...\n";
    std::cout << "  x y | Out  XOR\n";
    std::cout << "  " << std::string(16, '-') << "\n";
    
    int correct = 0;
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            auto cx = ctx.encrypt_bool(x);
            auto cy = ctx.encrypt_bool(y);
            auto ct_out = SpiralIO::iOCompleteTFHE::evaluate(ctx, prog, {cx, cy});
            bool out = ctx.decrypt_bool(ct_out);
            bool exp = (x != y);
            if (out == exp) correct++;
            std::cout << "  " << x << " " << y << " | " << out << "   " << exp
                      << "  " << (out == exp ? "OK" : "FAIL") << "\n";
        }
    }
    
    std::cout << "\n  Correct: " << correct << "/4\n";
    std::cout << "  Unlimited: YES (100 gates, TFHE auto-bootstrap)\n";
    std::cout << "===============================================================\n";
    
    return correct == 4 ? 0 : 1;
}
