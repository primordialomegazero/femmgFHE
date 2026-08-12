// ================================================================
// TEE BRIDGE SERVER — Main Process (Run in TRUSTED environment)
// ================================================================
// Holds CKKS + TFHE secret keys. Performs FHE↔iO conversions.
// Run this in separate user/container/enclave.
// ================================================================

#include "src/bridge/tee_dual_gate_bridge.h"
#include "src/bridge/dual_gate_bridge_fixed.h"

int main() {
    SpiralIO::TEEBridgeServer server;
    
    if (!server.start()) {
        std::cerr << "[TEE BRIDGE] Failed to start\n";
        return 1;
    }
    
    std::cout << "[TEE BRIDGE] Serving... (Ctrl+C to stop)\n";
    server.serve();
    
    return 0;
}
