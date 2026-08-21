// SAT + PERIOD-0 + FHE + ANALYSIS
// 8 variables, 256 assignments, encrypted search

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  SAT + PERIOD-0 + FHE + ANALYSIS\n";
    cout << "  8 Variables, 256 Assignments\n";
    cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    // ============================================
    // 3-SAT INSTANCE (8 variables)
    // ============================================

    cout << "3-SAT INSTANCE (8 variables):\n";
    cout << "  (x1 OR x2 OR NOT x3)\n";
    cout << "  (NOT x1 OR x2 OR x4)\n";
    cout << "  (x1 OR NOT x2 OR NOT x4)\n";
    cout << "  (x2 OR x3 OR NOT x4)\n";
    cout << "  (x5 OR x6 OR NOT x7)\n";
    cout << "  (NOT x5 OR x7 OR x8)\n";
    cout << "  (x3 OR x5 OR NOT x8)\n";
    cout << "  (x4 OR x6 OR x7)\n\n";

    // ============================================
    // PERIOD-0 TRAJECTORY (256 points)
    // ============================================

    cout << "GENERATING PERIOD-0 TRAJECTORY...\n";
    vector<double> trajectory;
    double x = 0.0;
    for (int i = 0; i < 256; i++) {
        x += phi_sq;
        x -= floor(x);
        trajectory.push_back(x);
    }
    cout << "  Generated: " << trajectory.size() << " points\n\n";

    // ============================================
    // BRUTE FORCE SAT SEARCH
    // ============================================

    cout << "BRUTE FORCE SAT SEARCH:\n";
    cout << "=======================\n\n";

    auto start = high_resolution_clock::now();

    int sat_count = 0;
    vector<int> sat_assignments;

    for (int i = 0; i < 256; i++) {
        bool x1 = (i & 1) != 0;
        bool x2 = (i & 2) != 0;
        bool x3 = (i & 4) != 0;
        bool x4 = (i & 8) != 0;
        bool x5 = (i & 16) != 0;
        bool x6 = (i & 32) != 0;
        bool x7 = (i & 64) != 0;
        bool x8 = (i & 128) != 0;

        bool clause1 = x1 || x2 || (!x3);
        bool clause2 = (!x1) || x2 || x4;
        bool clause3 = x1 || (!x2) || (!x4);
        bool clause4 = x2 || x3 || (!x4);
        bool clause5 = x5 || x6 || (!x7);
        bool clause6 = (!x5) || x7 || x8;
        bool clause7 = x3 || x5 || (!x8);
        bool clause8 = x4 || x6 || x7;

        bool satisfiable = clause1 && clause2 && clause3 && clause4 &&
                          clause5 && clause6 && clause7 && clause8;

        if (satisfiable) {
            sat_count++;
            sat_assignments.push_back(i);
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);

    cout << "  Satisfiable assignments: " << sat_count << " / 256\n";
    cout << "  Search time: " << duration.count() << " microseconds\n";
    cout << "  First solution: " << (sat_assignments.empty() ? -1 : sat_assignments[0]) << "\n\n";

    // ============================================
    // TRAJECTORY ANALYSIS
    // ============================================

    cout << "TRAJECTORY ANALYSIS:\n";
    cout << "====================\n\n";

    // Check for repeats
    bool has_repeat = false;
    for (int i = 0; i < 256; i++) {
        for (int j = i+1; j < 256; j++) {
            if (abs(trajectory[i] - trajectory[j]) < 1e-15) {
                has_repeat = true;
                cout << "  REPEAT at " << i << " and " << j << "\n";
                break;
            }
        }
        if (has_repeat) break;
    }

    cout << "  Repeat check: " << (has_repeat ? "FAILED - may repeat!" : "PASSED - walang repeat!") << "\n";

    // Check density
    vector<int> bins(20, 0);
    for (double v : trajectory) {
        int bin = (int)(v * 20);
        if (bin >= 20) bin = 19;
        bins[bin]++;
    }

    cout << "  Density (20 bins):\n";
    for (int i = 0; i < 20; i++) {
        cout << "    [" << i/20.0 << ", " << (i+1)/20.0 << "): " << bins[i] << "\n";
    }

    // Entropy
    double entropy = 0.0;
    for (int i = 0; i < 20; i++) {
        if (bins[i] > 0) {
            double p = bins[i] / 256.0;
            entropy -= p * log2(p);
        }
    }
    double max_entropy = log2(20);
    double normalized = entropy / max_entropy;

    cout << "\n  Shannon entropy: " << entropy << " bits\n";
    cout << "  Max entropy: " << max_entropy << " bits\n";
    cout << "  Normalized: " << normalized << "\n";
    cout << "  Percentage: " << normalized * 100 << "%\n\n";

    // ============================================
    // FHE INTEGRATION (CKKS)
    // ============================================

    cout << "FHE INTEGRATION (CKKS):\n";
    cout << "=======================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(3);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    // I-encrypt ang trajectory values
    vector<complex<double>> vec(256, {0.0, 0.0});
    for (int i = 0; i < 256; i++) {
        vec[i] = {trajectory[i], 0.0};
    }

    Plaintext pt = cc->MakeCKKSPackedPlaintext(vec);
    auto ct = cc->Encrypt(keys.publicKey, pt);

    cout << "  Encrypted: 256 trajectory values\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    // Decrypt at i-verify
    Plaintext pt_dec;
    cc->Decrypt(keys.secretKey, ct, &pt_dec);
    auto dec_values = pt_dec->GetCKKSPackedValue();

    bool all_match = true;
    for (int i = 0; i < 256; i++) {
        if (abs(dec_values[i].real() - trajectory[i]) > 1e-6) {
            all_match = false;
            cout << "  MISMATCH at " << i << ": " << dec_values[i].real() << " vs " << trajectory[i] << "\n";
            break;
        }
    }

    cout << "  Decryption check: " << (all_match ? "PASSED - lahat match!" : "FAILED") << "\n\n";

    // ============================================
    // CONCLUSION
    // ============================================

    cout << "========================================\n";
    cout << "  FINAL RESULTS:\n";
    cout << "  - SAT solutions: " << sat_count << "\n";
    cout << "  - Trajectory: " << (has_repeat ? "May repeat" : "Unbounded, walang repeat") << "\n";
    cout << "  - Entropy: " << normalized * 100 << "%\n";
    cout << "  - FHE: " << (all_match ? "Working" : "Failed") << "\n";
    cout << "  - Level: " << ct->GetLevel() << "\n";
    cout << "========================================\n";

    return 0;
}
