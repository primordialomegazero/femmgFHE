#include <iostream>
#include <iomanip>
#include "src/core/constants.h"
#include "src/fhe/fhe_core.h"
#include "src/refresh/seed_rotation_bootstrap.h"

using namespace lbcrypto;

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "===============================================================\n";
    std::cout << "  SEED ROTATION BOOTSTRAP\n";
    std::cout << "  CKKS -> GF-N (no original plaintext) -> Seed Rotate -> CKKS\n";
    std::cout << "===============================================================\n\n";

    auto sc = create_fhe_context(8192, 60);
    SeedRotationBootstrap srb;
    srb.init(42.0, 5);

    double test_val = 0.42;
    auto pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{test_val});
    auto ct = sc.cc->Encrypt(sc.kp.publicKey, pt);

    std::cout << "Initial value: " << test_val << "\n";
    std::cout << "GF layers: " << srb.N_gf_layers << "\n";
    std::cout << "Cassini start: " << (srb.verify_cassini() ? "OK" : "FAIL") << "\n\n";

    std::cout << "--- 10 BOOTSTRAP CYCLES ---\n\n";
    std::cout << "  " << std::setw(6) << "Cycle" 
              << std::setw(14) << "CKKS val" 
              << std::setw(14) << "Cassini" 
              << std::setw(8) << "Count\n";
    std::cout << "  " << std::string(44, '-') << "\n";

    for (int i = 0; i < 10; i++) {
        ct = srb.bootstrap(ct, sc);

        Plaintext pt_out;
        sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_out);
        double val = pt_out->GetCKKSPackedValue()[0].real();

        std::cout << "  " << std::setw(6) << i 
                  << std::setw(14) << val
                  << std::setw(14) << (srb.verify_cassini() ? "OK" : "FAIL")
                  << std::setw(8) << srb.bootstrap_count << "\n";
    }

    std::cout << "  " << std::string(44, '-') << "\n\n";

    std::cout << "--- FOUNDATION ---\n";
    std::cout << "  phi*psi = " << PHI*PSI << "\n";
    std::cout << "  phi+psi = " << PHI+PSI << "\n\n";

    std::cout << "===============================================================\n";
    std::cout << "  SEED ROTATION BOOTSTRAP — DONE\n";
    std::cout << "===============================================================\n";

    return 0;
}
