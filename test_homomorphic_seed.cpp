#include <iostream>
#include <iomanip>
#include "src/core/constants.h"
#include "src/fhe/fhe_core.h"
#include "src/refresh/homomorphic_seed_bootstrap.h"

using namespace lbcrypto;

int main() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  HOMOMORPHIC FGG + SEED ROTATION — ALL IN CIPHERTEXT DOMAIN\n";
    std::cout << "  No decrypt. No secretKey used during bootstrap.\n";
    std::cout << "===============================================================\n\n";

    auto sc = create_fhe_context(8192, 60);
    HomomorphicSeedBootstrap hsb;
    hsb.init(sc);

    double test_val = 0.42;
    auto ct = hsb.encrypt(test_val);

    // Verify initial encrypt
    Plaintext pt_init;
    sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_init);
    std::cout << "Initial: " << pt_init->GetCKKSPackedValue()[0].real() << "\n\n";

    std::cout << "--- 10 HOMOMORPHIC BOOTSTRAP CYCLES ---\n";
    std::cout << "  (All operations on ciphertexts. No decrypt.)\n\n";

    for (int i = 0; i < 10; i++) {
        ct = hsb.bootstrap(ct);
        
        Plaintext pt_out;
        sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_out);
        double val = pt_out->GetCKKSPackedValue()[0].real();
        
        std::cout << "  [" << i << "] " << val << " count=" << hsb.bootstrap_count << "\n";
    }

    std::cout << "\n===============================================================\n";
    std::cout << "  HOMOMORPHIC BOOTSTRAP DONE\n";
    std::cout << "  phi*psi = " << PHI*PSI << "\n";
    std::cout << "===============================================================\n";
    return 0;
}
