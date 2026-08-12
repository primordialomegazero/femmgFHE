#include <iostream>
#include "src/core/constants.h"
#include "src/fhe/fhe_core.h"
#include "src/refresh/collapse_bootstrap.h"

using namespace lbcrypto;

int main() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  COLLAPSE BOOTSTRAP — SECRET KEY ERASED AFTER BOOTSTRAP\n";
    std::cout << "===============================================================\n\n";

    auto sc = create_fhe_context(8192, 60);
    CollapseBootstrap cb;
    cb.init(42.0, 5);

    double test_val = 0.42;
    auto pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{test_val});
    auto ct = sc.cc->Encrypt(sc.kp.publicKey, pt);

    std::cout << "Before bootstrap — secretKey present: " 
              << (sc.kp.secretKey ? "YES" : "NO") << "\n";

    ct = cb.bootstrap(ct, sc);

    std::cout << "After bootstrap — secretKey present: " 
              << (sc.kp.secretKey ? "YES" : "NO") << "\n";

    // Try to decrypt — should fail if secretKey is erased
    try {
        Plaintext pt_out;
        sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_out);
        double val = pt_out->GetCKKSPackedValue()[0].real();
        std::cout << "Decrypt after collapse: " << val << " (secretKey was NOT erased!)\n";
    } catch (...) {
        std::cout << "Decrypt after collapse: FAILED (secretKey IS erased)\n";
    }

    std::cout << "\n===============================================================\n";
    std::cout << "  COLLAPSE TEST DONE\n";
    std::cout << "===============================================================\n";

    return 0;
}
