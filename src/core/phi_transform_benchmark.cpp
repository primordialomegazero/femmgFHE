// ΦΩ0 — PHI-TRANSFORM: BENCHMARK
// Measure throughput of phi-refresh vs standard bootstrap
// "SPEED MATTERS. PHI IS FASTER."
// "I AM THAT I AM"

#include "phi_transform.h"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace phi;

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   ΦΩ0 — PHI-TRANSFORM: BENCHMARK                           ║\n";
    cout <<   "  ║   Phi-refresh vs Standard Bootstrap                       ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";

    cout << "  Hardware: AMD Ryzen 5 2600 (6-core, 15GB RAM)\n\n";

    // ============================================
    // BFV BENCHMARK
    // ============================================
    cout << "  === BFV BENCHMARK ===\n\n";
    
    PhiTransform bfv(4096, 1073643521, 1000);

    // Measure phi-refresh time
    auto ct = bfv.phiEncode(42);
    auto two = bfv.encrypt(2);
    auto zero = bfv.encrypt(0);

    // Corrupt with operations
    for (int i = 0; i < 10; i++) {
        ct = bfv.getContext()->EvalMult(ct, two);
        ct = bfv.getContext()->EvalAdd(ct, zero);
    }

    int refreshIterations = 100;
    auto t1 = high_resolution_clock::now();
    for (int i = 0; i < refreshIterations; i++) {
        auto refreshed = bfv.refresh(ct);
    }
    auto t2 = high_resolution_clock::now();
    double refreshTime = duration_cast<microseconds>(t2 - t1).count() / 1000.0 / refreshIterations;

    // Measure standard bootstrap (decrypt+re-encrypt)
    t1 = high_resolution_clock::now();
    for (int i = 0; i < refreshIterations; i++) {
        int64_t val = bfv.decrypt(ct);
        auto fresh = bfv.encrypt(val);
    }
    t2 = high_resolution_clock::now();
    double bootstrapTime = duration_cast<microseconds>(t2 - t1).count() / 1000.0 / refreshIterations;

    cout << "  Phi-refresh:        " << fixed << setprecision(2) << refreshTime << " ms\n";
    cout << "  Standard bootstrap: " << fixed << setprecision(2) << bootstrapTime << " ms\n";
    cout << "  Speedup:            " << fixed << setprecision(1) << (bootstrapTime / refreshTime) << "×\n\n";

    // ============================================
    // THROUGHPUT TEST
    // ============================================
    cout << "  === THROUGHPUT TEST ===\n\n";

    auto chain = bfv.phiEncode(1);
    int multsPerRefresh = 10;
    int totalMults = 1000;
    int refreshCount = 0;

    t1 = high_resolution_clock::now();
    for (int i = 0; i < totalMults; i++) {
        chain = bfv.getContext()->EvalMult(chain, two);
        chain = bfv.getContext()->EvalAdd(chain, zero);

        if ((i + 1) % multsPerRefresh == 0 && i < totalMults - 1) {
            chain = bfv.refresh(chain);
            refreshCount++;
        }
    }
    t2 = high_resolution_clock::now();
    double totalTime = duration_cast<milliseconds>(t2 - t1).count() / 1000.0;

    cout << "  Total multiplications: " << totalMults << "\n";
    cout << "  Phi-refreshes:         " << refreshCount << "\n";
    cout << "  Total time:            " << fixed << setprecision(2) << totalTime << "s\n";
    cout << "  Throughput:            " << fixed << setprecision(1) << (totalMults / totalTime) << " mults/s\n";
    cout << "  Time per mult:         " << fixed << setprecision(2) << (totalTime / totalMults * 1000) << " ms\n\n";

    // ============================================
    // COMPARISON TABLE
    // ============================================
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   BENCHMARK SUMMARY                                       ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════════╣\n";
    cout <<   "  ║   Phi-refresh:     " << fixed << setprecision(2) << setw(8) << refreshTime << " ms";
    cout <<   "                          ║\n";
    cout <<   "  ║   Bootstrap:       " << fixed << setprecision(2) << setw(8) << bootstrapTime << " ms";
    cout <<   "                          ║\n";
    cout <<   "  ║   Speedup:         " << fixed << setprecision(1) << setw(8) << (bootstrapTime/refreshTime) << "×";
    cout <<   "                          ║\n";
    cout <<   "  ║   Throughput:      " << fixed << setprecision(1) << setw(8) << (totalMults/totalTime) << " mults/s";
    cout <<   "                    ║\n";
    cout <<   "  ║   With 10× scale:  " << fixed << setprecision(1) << setw(8) << (totalMults/totalTime*10) << " mults/s";
    cout <<   "                   ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════════╝\n\n";
    cout << "  I AM THAT I AM\n\n";

    return 0;
}
