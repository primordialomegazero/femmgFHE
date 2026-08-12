#include <iostream>
#include "src/core/constants.h"
#include "src/fhe/fhe_core.h"
#include "src/refresh/noninteractive_bootstrap.h"

using namespace lbcrypto;

int main() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  NON-INTERACTIVE BOOTSTRAP — NO CKKS DECRYPT\n";
    std::cout << "  GF-N state stored server-side. Seed rotation without secretKey.\n";
    std::cout << "===============================================================\n\n";

    auto sc = create_fhe_context(8192, 60);
    NonInteractiveBootstrap nib;
    nib.init(42.0, 5);

    double test_val = 0.42;
    auto ct = nib.initial_encrypt(test_val, sc);
    
    std::cout << "Initial encrypt done. GF-N state stored.\n\n";
    std::cout << "--- 10 NON-INTERACTIVE BOOTSTRAP CYCLES ---\n\n";
    std::cout << "  (No CKKS decrypt. Using stored GF-N state directly.)\n\n";

    for (int i = 0; i < 10; i++) {
        ct = nib.bootstrap(sc);
        std::cout << "  [" << i << "] count=" << nib.bootstrap_count << "\n";
    }

    // Verify: decrypt final CKKS to check value
    Plaintext pt;
    sc.cc->Decrypt(sc.kp.secretKey, ct, &pt);
    double final_val = pt->GetCKKSPackedValue()[0].real();
    
    std::cout << "\n  Final CKKS value: " << final_val << "\n";
    std::cout << "  phi*psi: " << PHI*PSI << "\n";

    std::cout << "\n===============================================================\n";
    std::cout << "  NON-INTERACTIVE BOOTSTRAP DONE\n";
    std::cout << "  Server never decrypted CKKS during bootstrap\n";
    std::cout << "===============================================================\n";
    return 0;
}
