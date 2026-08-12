#include <iostream>
#include "src/core/constants.h"
#include "src/fhe/split_context.h"
#include "src/refresh/oracle_bootstrap.h"

using namespace lbcrypto;

int main() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  ORACLE BOOTSTRAP — SERVER HAS NO SECRET KEY\n";
    std::cout << "===============================================================\n\n";

    auto sc = create_fhe_context(8192, 60);
    
    PublicContext pub;
    BootstrapContext boot;
    split_context(sc, pub, boot);
    
    // Server only has pub
    std::cout << "Server has secretKey: " << (boot.secretKey ? "YES (ERROR!)" : "NO") << "\n";
    std::cout << "Server has publicKey: " << (pub.publicKey ? "YES" : "NO") << "\n\n";

    OracleBootstrap ob;
    ob.init(42.0, 5);

    double test_val = 0.42;
    auto ct = pub.encrypt(test_val);

    std::cout << "--- 10 BOOTSTRAP CYCLES (ORACLE) ---\n\n";
    for (int i = 0; i < 10; i++) {
        ct = ob.bootstrap(ct, boot);
        std::cout << "  [" << i << "] cassini=" << (ob.verify_cassini() ? "OK" : "FAIL")
                  << " count=" << ob.bootstrap_count << "\n";
    }

    std::cout << "\n===============================================================\n";
    std::cout << "  ORACLE BOOTSTRAP DONE\n";
    std::cout << "  Server never held the secret key\n";
    std::cout << "===============================================================\n";
    return 0;
}
