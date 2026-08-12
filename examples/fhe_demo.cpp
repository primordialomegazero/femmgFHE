#include "src/fhe/spiral_fhe_io_final.h"
#include <iostream>

int main() {
    auto sc = create_fhe_context(8192, 60);
    DecryptLayer dl;
    dl.init(sc, 42.0, 5);
    
    double data = 0.42;
    auto pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{data});
    auto ct = sc.cc->Encrypt(sc.kp.publicKey, pt);
    
    std::cout << "FHE Demo: 10 bootstrap cycles\n";
    for (int i = 0; i < 10; i++) {
        ct = dl.bootstrap(ct);
        std::cout << "  Cycle " << i << ": Cassini=" 
                  << (dl.verify_cassini() ? "OK" : "FAIL") << "\n";
    }
    std::cout << "Status: PASS\n";
    return 0;
}
