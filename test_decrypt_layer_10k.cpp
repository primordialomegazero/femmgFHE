#include <iostream>
#include <iomanip>
#include <chrono>
#include "src/core/constants.h"
#include "src/fhe/fhe_core.h"
#include "src/refresh/decrypt_layer.h"

using namespace lbcrypto;

int main() {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "===============================================================\n";
    std::cout << "  DECRYPTION LAYER — 10,000 CYCLES — 64K RINGDIM\n";
    std::cout << "  Unlimited depth. GF-N protected. SecretKey isolated.\n";
    std::cout << "===============================================================\n\n";

    auto sc = create_fhe_context(65536, 60, 4096);
    DecryptLayer dl;
    dl.init(sc, 42.0, 5);

    double data = 0.42;
    auto pt = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{data});
    auto ct = sc.cc->Encrypt(sc.kp.publicKey, pt);

    std::cout << "RingDim: 65536\n";
    std::cout << "BatchSize: 4096\n";
    std::cout << "Initial data: " << data << "\n";
    std::cout << "Cassini start: " << (dl.verify_cassini() ? "OK" : "FAIL") << "\n\n";

    int cycles = 10000;
    int cassini_fails = 0;
    double min_val = 1e10, max_val = -1e10;

    std::cout << "--- RUNNING " << cycles << " CYCLES ---\n\n";

    auto t_start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < cycles; i++) {
        ct = dl.decrypt_to_gf(ct);

        if (!dl.verify_cassini()) cassini_fails++;

        Plaintext pt_out;
        sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_out);
        double val = pt_out->GetCKKSPackedValue()[0].real();
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;

        if (i % 1000 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - t_start).count();
            double rate = (i + 1) / elapsed;
            double eta = (cycles - i - 1) / rate;
            
            std::cout << "  [" << std::setw(5) << i << "/" << cycles << "]"
                      << " val=" << std::setw(12) << val
                      << " cassini=" << (dl.verify_cassini() ? "OK" : "FAIL")
                      << " elapsed=" << std::setw(6) << std::setprecision(0) << elapsed << "s"
                      << " rate=" << std::setprecision(2) << rate << " c/s"
                      << " ETA=" << std::setprecision(0) << eta << "s"
                      << std::setprecision(4) << "\n";
        }
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    double total_sec = std::chrono::duration<double>(t_end - t_start).count();

    Plaintext pt_final;
    sc.cc->Decrypt(sc.kp.secretKey, ct, &pt_final);
    double final_val = pt_final->GetCKKSPackedValue()[0].real();

    std::cout << "\n--- RESULTS ---\n\n";
    std::cout << "  Total cycles:       " << cycles << "\n";
    std::cout << "  Bootstrap count:    " << dl.bootstrap_count << "\n";
    std::cout << "  Cassini fails:      " << cassini_fails << "\n";
    std::cout << "  CKKS value range:   [" << min_val << ", " << max_val << "]\n";
    std::cout << "  Final value:        " << final_val << "\n";
    std::cout << "  Total time:         " << std::setprecision(2) << total_sec << "s\n";
    std::cout << "  Speed:              " << (cycles / total_sec) << " cycles/s\n";
    std::cout << "  phi*psi:            " << PHI*PSI << "\n";

    std::cout << "\n===============================================================\n";
    std::cout << "  DECRYPTION LAYER 10K — COMPLETE\n";
    std::cout << "===============================================================\n";

    return 0;
}
