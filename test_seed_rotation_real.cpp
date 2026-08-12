#include <iostream>
#include <iomanip>
#include <chrono>
#include "src/core/constants.h"
#include "src/fhe/fhe_core.h"
#include "src/refresh/seed_rotation_bootstrap.h"

using namespace lbcrypto;

int main() {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "===============================================================\n";
    std::cout << "  SEED ROTATION BOOTSTRAP — OPENFHE 16K RINGDIM\n";
    std::cout << "  100 cycles. Real measurements. No bullshit.\n";
    std::cout << "===============================================================\n\n";

    auto sc = create_fhe_context(16384, 60, 1024);
    SeedRotationBootstrap srb;
    srb.init(42.0, 5);

    double test_val = 0.42;
    auto pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{test_val});
    auto ct = sc.cc->Encrypt(sc.kp.publicKey, pt);

    Plaintext pt_init;
    sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_init);
    double init_val = pt_init->GetCKKSPackedValue()[0].real();

    std::cout << "Config:\n";
    std::cout << "  RingDim: 16384\n";
    std::cout << "  BatchSize: 1024\n";
    std::cout << "  GF layers: " << srb.N_gf_layers << "\n";
    std::cout << "  Initial value: " << test_val << "\n";
    std::cout << "  CKKS decrypt of initial: " << init_val << "\n";
    std::cout << "  Cassini: " << (srb.verify_cassini() ? "OK" : "FAIL") << "\n\n";

    int cycles = 100;
    int cassini_fails = 0;
    double total_time = 0;
    double min_val = 1e10, max_val = -1e10;

    std::cout << "--- RUNNING " << cycles << " CYCLES ---\n\n";

    auto t_start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < cycles; i++) {
        auto t0 = std::chrono::high_resolution_clock::now();
        ct = srb.bootstrap(ct, sc);
        auto t1 = std::chrono::high_resolution_clock::now();
        double cycle_time = std::chrono::duration<double, std::milli>(t1 - t0).count();
        total_time += cycle_time;

        if (!srb.verify_cassini()) cassini_fails++;

        Plaintext pt_out;
        sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_out);
        double val = pt_out->GetCKKSPackedValue()[0].real();
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;

        if (i % 10 == 0 || i == cycles - 1) {
            std::cout << "  [" << std::setw(3) << i << "] val=" << std::setw(12) << val
                      << " time=" << std::setw(8) << std::setprecision(2) << cycle_time << "ms"
                      << " cassini=" << (srb.verify_cassini() ? "OK" : "FAIL")
                      << " count=" << srb.bootstrap_count << "\n";
        }
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    double total_sec = std::chrono::duration<double>(t_end - t_start).count();

    std::cout << "\n--- RESULTS ---\n\n";
    std::cout << "  Total cycles:       " << cycles << "\n";
    std::cout << "  Bootstrap count:    " << srb.bootstrap_count << "\n";
    std::cout << "  Cassini fails:      " << cassini_fails << "\n";
    std::cout << "  Total time:         " << std::setprecision(2) << total_sec << "s\n";
    std::cout << "  Avg time/cycle:     " << (total_time / cycles) << "ms\n";
    std::cout << "  Speed:              " << (cycles / total_sec) << " cycles/s\n";
    std::cout << "  CKKS value range:   [" << min_val << ", " << max_val << "]\n";
    std::cout << "  phi*psi:            " << PHI*PSI << "\n";
    std::cout << "  phi+psi:            " << PHI+PSI << "\n";

    std::cout << "\n===============================================================\n";
    std::cout << "  SEED ROTATION BOOTSTRAP — REAL OPENFHE TEST COMPLETE\n";
    std::cout << "===============================================================\n";

    return 0;
}
