#include "src/io/spiral_io_tfhe.h"
#include "src/io/tfhe_shared_keys.h"
#include "src/crypto/golden_prng.h"

int main() {
    // Use fixed master seed to generate deterministic context
    SpiralIO::TFHEContext ctx;
    ctx.init();  // uses default seed? We'll modify later.

    SpiralIO::TFHESharedKeys::save_keys("/tmp/fhe_io_keys.bin", ctx);
    std::cout << "Shared TFHE keys saved to /tmp/fhe_io_keys.bin\n";
    return 0;
}
