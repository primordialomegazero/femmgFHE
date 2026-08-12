#include <iostream>
#include <iomanip>
#include "src/core/constants.h"
#include "src/fhe/fhe_core.h"
#include "src/refresh/io_bootstrap.h"

using namespace lbcrypto;

int main() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  iO BOOTSTRAP — SECRET KEY INSIDE OBFUSCATED CIRCUIT\n";
    std::cout << "===============================================================\n\n";

    auto sc = create_fhe_context(8192, 60);
    iOBootstrap iob;
    iob.init(42.0, 5);
    iob.obfuscate(sc);

    double test_val = 0.42;
    auto pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{test_val});
    auto ct = sc.cc->Encrypt(sc.kp.publicKey, pt);

    std::cout << "Obfuscated: YES\n";
    std::cout << "Initial: " << test_val << "\n\n";
    std::cout << "--- 5 BOOTSTRAP CYCLES (iO-PROTECTED) ---\n\n";

    for (int i = 0; i < 5; i++) {
        ct = iob.bootstrap(ct, sc);
        
        Plaintext pt_out;
        sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_out);
        double val = pt_out->GetCKKSPackedValue()[0].real();
        
        std::cout << "  Cycle " << i << ": " << val 
                  << " cassini=" << (iob.verify_cassini() ? "OK" : "FAIL")
                  << " count=" << iob.bootstrap_count << "\n";
    }

    std::cout << "\n===============================================================\n";
    std::cout << "  iO BOOTSTRAP DONE\n";
    std::cout << "===============================================================\n";

    return 0;
}
