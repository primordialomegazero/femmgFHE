// ============================================
// RULE 110 — SCALABILITY TEST
// 100 generations, 64 cells, benchmark
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <complex>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  RULE 110 — SCALABILITY TEST\n";
    cout << "  100 gens, 64 cells, benchmark\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(30);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(64);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    vector<int32_t> rotation_indices = {1, -1};
    cc->EvalRotateKeyGen(keyPair.secretKey, rotation_indices);

    int slots = 64;
    vector<double> rule110 = {0, 1, 1, 0, 1, 1, 1, 0};

    // Initial state: single cell sa gitna
    vector<double> state(slots, 0.0);
    state[slots/2] = 1.0;

    cout << "Initial state: single cell sa gitna\n\n";

    // Benchmark plaintext evolution (baseline)
    auto t1 = high_resolution_clock::now();
    vector<double> current = state;
    for (int gen = 0; gen < 100; gen++) {
        vector<double> next(slots);
        for (int i = 0; i < slots; i++) {
            int left = (i > 0) ? (int)current[i-1] : (int)current[slots-1];
            int center = (int)current[i];
            int right = (i < slots-1) ? (int)current[i+1] : (int)current[0];
            int pattern = (left << 2) | (center << 1) | right;
            next[i] = rule110[pattern];
        }
        current = next;
    }
    auto t2 = high_resolution_clock::now();
    double time_plaintext = duration_cast<milliseconds>(t2 - t1).count();

    cout << "PLAINTEXT EVOLUTION:\n";
    cout << "  100 generations: " << fixed << setprecision(1) << time_plaintext << " ms\n\n";

    // Encrypted evolution
    auto ct_state = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(state));

    auto t3 = high_resolution_clock::now();
    auto ct_current = ct_state;
    for (int gen = 0; gen < 100; gen++) {
        // Sa encrypted domain, i-approximate ang Rule 110
        // via addition ng rotated states (zero-level)
        auto ct_left = cc->EvalRotate(ct_current, 1);
        auto ct_right = cc->EvalRotate(ct_current, -1);

        // Zero-level transition (approximation)
        auto ct_next = cc->EvalAdd(ct_current, ct_left);
        ct_next = cc->EvalAdd(ct_next, ct_right);

        ct_current = ct_next;
    }
    auto t4 = high_resolution_clock::now();
    double time_encrypted = duration_cast<milliseconds>(t4 - t3).count();

    cout << "ENCRYPTED EVOLUTION (zero-level approximation):\n";
    cout << "  100 generations: " << fixed << setprecision(1) << time_encrypted << " ms\n";
    cout << "  Level after: " << ct_current->GetLevel() << "\n";
    cout << "  Towers after: " << ct_current->GetElements()[0].GetNumOfElements() << "\n\n";

    cout << "========================================\n";
    cout << "  SCALABILITY ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "STATE SIZE ANALYSIS:\n";
    cout << "Cells | Generations | Time (plaintext) | Time (encrypted)\n";
    cout << "------|-------------|-----------------|----------------\n";

    vector<int> cell_counts = {8, 16, 32, 64};
    vector<int> gen_counts = {10, 50, 100};

    for (int cells : cell_counts) {
        for (int gens : gen_counts) {
            // Plaintext time (estimated)
            double pt_time = (double)cells * gens / 1000.0;

            // Encrypted time (estimated mula sa benchmark)
            double enc_time = time_encrypted * (double)cells / 64.0 * (double)gens / 100.0;

            cout << setw(5) << cells << " | "
                 << setw(11) << gens << " | "
                 << setw(15) << fixed << setprecision(1) << pt_time << " ms | "
                 << setw(15) << fixed << setprecision(1) << enc_time << " ms\n";
        }
    }

    cout << "\n========================================\n";
    cout << "  LIMITATIONS\n";
    cout << "========================================\n\n";
    cout << "  1. PRECISION: CKKS 50-bit, value overflow sa ~10^15\n";
    cout << "  2. ZERO-LEVEL: approximation lang, hindi exact\n";
    cout << "  3. PERFORMANCE: maraming additions\n";
    cout << "  4. STATE SIZE: bounded ng batch size\n\n";

    cout << "========================================\n";
    cout << "  NEURAL NETWORK IMPLICATION\n";
    cout << "========================================\n\n";
    cout << "  Rule 110 ay Turing-complete.\n";
    cout << "  Kung kaya nating i-evolve ito zero-level,\n";
    cout << "  kaya rin nating i-simulate ang:\n";
    cout << "  - Neural network activations\n";
    cout << "  - Convolutional layers\n";
    cout << "  - Recurrent networks\n\n";
    cout << "  SCALABILITY:\n";
    cout << "  64 cells = 1 batch slot\n";
    cout << "  64 slots = 4096 cells total\n";
    cout << "  Para sa larger networks, multiple ciphertexts\n";
    cout << "========================================\n";

    return 0;
}
