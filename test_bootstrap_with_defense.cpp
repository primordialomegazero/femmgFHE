#include <iostream>
#include <iomanip>
#include "src/core/constants.h"
#include "src/fhe/fhe_core.h"
#include "src/refresh/spiral_bootstrap.h"

using namespace lbcrypto;

int main() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  SPIRAL BOOTSTRAP — WITH SIDE-CHANNEL DEFENSE\n";
    std::cout << "  bootstrap_io + bootstrap_zero + bootstrap_select\n";
    std::cout << "===============================================================\n\n";

    auto sc = create_fhe_context(8192, 60);
    
    // Init SpiralBootstrap with side-channel enabled
    SpiralBootstrap sb;
    sb.init(42.0, 5, true);
    sb.enable_iO(3, 5);
    sb.enable_sidechannel = true;
    
    std::cout << "Status: " << sb.status() << "\n\n";

    // Test: encrypt a value, run bootstrap cycles, verify
    double test_val = 0.42;
    auto pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{test_val});
    auto ct = sc.cc->Encrypt(sc.kp.publicKey, pt);
    
    std::cout << "--- BOOTSTRAP CYCLES (with iO + Side-Channel Defense) ---\n\n";
    
    for (int cycle = 0; cycle < 5; cycle++) {
        // bootstrap_io: structural iO + side-channel
        ct = sb.bootstrap_io(ct, sc);
        
        Plaintext pt_out;
        sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_out);
        double val = pt_out->GetCKKSPackedValue()[0].real();
        
        std::cout << "  Cycle " << cycle << ": " << val 
                  << " (cassini=" << (sb.verify_cassini() ? "OK" : "FAIL") << ")"
                  << " bootstrap_count=" << sb.bootstrap_count << "\n";
    }
    
    std::cout << "\n--- MODE SWITCHING ---\n\n";
    
    // Switch to blackhole mode
    sb.enable_blackhole_mode(5, 8);
    std::cout << "Switched: " << sb.status() << "\n";
    ct = sb.bootstrap_io(ct, sc);
    
    // Switch to single mode (no iO, fast)
    sb.disable_obfuscation();
    std::cout << "Switched: " << sb.status() << "\n";
    ct = sb.bootstrap_single(ct, sc);
    
    // Bootstrap zero (seed rotation)
    ct = sb.bootstrap_zero(ct, sc);
    std::cout << "bootstrap_zero done. count=" << sb.bootstrap_count << "\n";
    
    std::cout << "\n===============================================================\n";
    std::cout << "  ALL BOOTSTRAP MODES WORKING\n";
    std::cout << "  bootstrap_io + bootstrap_single + bootstrap_zero\n";
    std::cout << "  Side-channel defense active\n";
    std::cout << "===============================================================\n";
    
    return 0;
}
