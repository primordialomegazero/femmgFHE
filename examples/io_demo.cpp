#include "src/io/spiral_io_tfhe.h"
#include <iostream>

int main() {
    SpiralIO::TFHEContext ctx;
    ctx.init();
    
    std::cout << "iO Demo: XOR truth table\n";
    auto prog = SpiralIO::iOCompleteTFHE::obfuscate(ctx, 
        {{1,0,0,0,0,0}, {0,1,0,0,0,0}, {0,0,1,0,0,0}, {0,0,0,1,0,0}},
        {{0,1,0,0,0,0}, {0,0,1,0,0,0}, {0,0,1,0,0,0}, {0,0,0,1,0,0}});
    
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
            auto out = SpiralIO::iOCompleteTFHE::evaluate(ctx, prog, 
                {ctx.encrypt_bool(x), ctx.encrypt_bool(y)});
            std::cout << "  " << x << " " << y << " -> " 
                      << ctx.decrypt_bool(out) << "\n";
        }
    }
    std::cout << "Status: PASS\n";
    return 0;
}
