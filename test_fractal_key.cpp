#include <iostream>
#include "src/core/constants.h"
#include "src/fhe/fhe_core.h"
#include "src/refresh/fractal_key_bootstrap.h"

using namespace lbcrypto;

int main() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  FRACTAL KEY BOOTSTRAP — SECRET KEY SPLIT INTO FRAGMENTS\n";
    std::cout << "===============================================================\n\n";

    auto sc = create_fhe_context(8192, 60);
    FractalKeyBootstrap fkb;
    fkb.init(42.0, 5);
    fkb.fractalize(42.0);

    double test_val = 0.42;
    auto pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{test_val});
    auto ct = sc.cc->Encrypt(sc.kp.publicKey, pt);

    std::cout << "Key fragmented: YES (" << FractalKeyBootstrap::KEY_FRAGMENTS << " fragments)\n";
    std::cout << "Initial: " << test_val << "\n\n";
    std::cout << "--- 10 BOOTSTRAP CYCLES (FRACTAL KEY) ---\n\n";

    for (int i = 0; i < 10; i++) {
        ct = fkb.bootstrap(ct, sc);
        Plaintext pt_out;
        sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_out);
        double val = pt_out->GetCKKSPackedValue()[0].real();
        std::cout << "  [" << i << "] " << val 
                  << " cassini=" << (fkb.verify_cassini() ? "OK" : "FAIL")
                  << " count=" << fkb.bootstrap_count << "\n";
    }

    std::cout << "\n===============================================================\n";
    std::cout << "  FRACTAL KEY BOOTSTRAP DONE\n";
    std::cout << "===============================================================\n";
    return 0;
}
