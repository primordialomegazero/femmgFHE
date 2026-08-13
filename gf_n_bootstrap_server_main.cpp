#include "src/bridge/gf_n_fhe_bootstrap.h"

int main() {
    SpiralIO::GFNBootstrapServer server;
    if (!server.start()) {
        std::cerr << "Failed to start GF-N bootstrap server\n";
        return 1;
    }
    std::cout << "GF-N Bootstrap Server running...\n";
    server.serve();
    return 0;
}
