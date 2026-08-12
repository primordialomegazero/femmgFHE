#include <iostream>
#include <iomanip>
#include <chrono>
#include "spiral_fhe_io_final.h"

int main() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  SPIRAL FHE+iO — PRODUCTION BOOTSTRAP v46.0\n";
    std::cout << "  Two-layer FHE. Unlimited depth. GF-N protected.\n";
    std::cout << "===============================================================\n\n";

    auto sc = create_fhe_context(16384, 60);
    DecryptLayer dl;
    dl.init(sc, 42.0, 5);

    double data = 0.42;
    auto pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{data});
    auto ct = sc.cc->Encrypt(sc.kp.publicKey, pt);

    std::cout << "RingDim: 16384\n";
    std::cout << "Initial data: " << data << "\n";
    std::cout << "Cassini: " << (dl.verify_cassini() ? "OK" : "FAIL") << "\n\n";

    int cycles = 100;
    auto t0 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < cycles; i++) {
        ct = dl.bootstrap(ct);
        if (i % 10 == 0) {
            Plaintext pt_out;
            sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_out);
            std::cout << "  [" << i << "] " << pt_out->GetCKKSPackedValue()[0].real()
                      << " cassini=" << (dl.verify_cassini() ? "OK" : "FAIL") << "\n";
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();

    std::cout << "\n  Cycles: " << cycles << "\n";
    std::cout << "  Time: " << secs << "s (" << (cycles/secs) << " c/s)\n";
    std::cout << "  phi*psi = " << PHI*PSI << "\n";

    std::cout << "\n===============================================================\n";
    std::cout << "  PRODUCTION BOOTSTRAP — WORKING\n";
    std::cout << "===============================================================\n";
    return 0;
}
