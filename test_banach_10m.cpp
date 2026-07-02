#include "src/core/femmg_operations.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>

using namespace std;

int main() {
    cout << "═══════════════════════════════════════════════" << endl;
    cout << "  BANACH CONTRACTION — 10M OPS TEST" << endl;
    cout << "  Unlimited Depth FHE Verification" << endl;
    cout << "═══════════════════════════════════════════════" << endl;
    cout << endl;

    FEmmgFHE fhe;
    const long long TOTAL = 10000000LL;  // 10M
    const long long BATCH = 1000000LL;   // Report every 1M

    vector<double> noise_history;
    long long errors = 0;

    auto ct = fhe.encrypt(2);
    int64_t expected = 2;

    auto start = chrono::high_resolution_clock::now();

    for (long long i = 1; i <= TOTAL; i++) {
        auto ct2 = fhe.encrypt(2);
        if (i % 2 == 1) {
            ct = fhe.add(ct, ct2);
            expected += 2;
        } else {
            ct = fhe.multiply(ct, ct2);
            expected *= 2;
        }

        if (i % BATCH == 0) {
            noise_history.push_back(ct.noise);

            auto now = chrono::high_resolution_clock::now();
            auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - start).count();

            int64_t decrypted = fhe.decrypt(ct);
            if (decrypted != expected) errors++;

            cout << "[" << setw(6) << i/BATCH << "/10] "
                 << "Ops: " << setw(10) << i
                 << " | Noise: " << fixed << setprecision(5) << ct.noise
                 << " | Result: " << decrypted
                 << " | Expected: " << expected
                 << " | Time: " << elapsed/1000.0 << "s";

            if (decrypted != expected) cout << " ❌ ERROR!";
            cout << endl;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    auto total_ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << endl;
    cout << "═══════════════════════════════════════════════" << endl;
    cout << "  RESULTS" << endl;
    cout << "═══════════════════════════════════════════════" << endl;
    cout << "  Total Ops:     " << TOTAL << endl;
    cout << "  Time:          " << total_ms/1000.0 << "s" << endl;
    cout << "  TPS:           " << (TOTAL * 1000.0 / total_ms) << endl;
    cout << "  Final Noise:   " << fixed << setprecision(6) << ct.noise << endl;
    cout << "  Errors:        " << errors << endl;
    cout << "  Accuracy:      " << (errors == 0 ? "100.0000%" : "FAILED") << endl;

    if (!noise_history.empty()) {
        double avg_noise = 0;
        for (double n : noise_history) avg_noise += n;
        avg_noise /= noise_history.size();

        double min_noise = noise_history[0], max_noise = noise_history[0];
        for (double n : noise_history) {
            if (n < min_noise) min_noise = n;
            if (n > max_noise) max_noise = n;
        }

        cout << endl;
        cout << "  NOISE ANALYSIS" << endl;
        cout << "  Avg Noise:     " << fixed << setprecision(6) << avg_noise << endl;
        cout << "  Min Noise:     " << min_noise << endl;
        cout << "  Max Noise:     " << max_noise << endl;
        cout << "  Noise Range:   " << (max_noise - min_noise) << endl;
        cout << "  Flatline?      " << ((max_noise - min_noise) < 0.01 ? "✅ YES" : "❌ NO") << endl;
    }

    cout << "═══════════════════════════════════════════════" << endl;

    return errors == 0 ? 0 : 1;
}
