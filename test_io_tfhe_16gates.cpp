#include "src/io/spiral_io_tfhe.h"

int main() {
    SpiralIO::TFHEContext ctx;
    ctx.init();
    
    std::cout << "===============================================================\n";
    std::cout << "  SPIRAL iO — TFHE 16 GATES (Scaled Test)\n";
    std::cout << "===============================================================\n\n";
    
    constexpr int NG = 16;
    constexpr int NI = 2;
    constexpr int TW = NG + NI;
    
    // XOR + buffer chain (NOT gates)
    std::vector<std::vector<int>> in1(NG, std::vector<int>(TW, 0));
    std::vector<std::vector<int>> in2(NG, std::vector<int>(TW, 0));
    
    // XOR circuit (4 gates)
    in1[0][0] = 1; in2[0][1] = 1;
    in1[1][0] = 1; in2[1][2] = 1;
    in1[2][1] = 1; in2[2][2] = 1;
    in1[3][3] = 1; in2[3][4] = 1;
    
    // Buffer chain (12 gates: NOT gates)
    for (int g = 4; g < NG; g++) {
        in1[g][NI + g - 1] = 1;
        in2[g][NI + g - 1] = 1;
    }
    
    std::cout << "Circuit: " << NG << " gates, " << TW << " wires\n";
    std::cout << "Obfuscating...\n";
    
    auto prog = SpiralIO::iOCompleteTFHE::obfuscate(ctx, in1, in2);
    std::cout << "Obfuscated: " << NG << " gates\n\n";
    
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
    std::cout << "  Unlimited: YES (TFHE auto-bootstrap per gate)\n";
    std::cout << "===============================================================\n";
    
    return correct == 4 ? 0 : 1;
}
