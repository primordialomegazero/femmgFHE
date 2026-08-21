// RULE 110 CELLULAR AUTOMATON — 0-LEVEL TURING COMPLETE
// Ang holy grail: computation via local rules, hindi NAND
// Lahat operations: AddSub + Rotate + EvalSum = 0 LEVELS

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 CELLULAR AUTOMATON\n";
    std::cout << "  0-LEVEL TURING COMPLETE ATTEMPT\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double phi = 1.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalSumKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    // Helper functions
    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto make_vector_ct = [&](std::vector<double> vals) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (size_t i = 0; i < vals.size() && i < slots; i++) {
            vec[i] = {vals[i], 0.0};
        }
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct, size_t idx = 0) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[idx].real();
    };

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);

    // ============================================
    // RULE 110 TRUTH TABLE
    // Pattern: L C R → Next
    // 111 → 0, 110 → 1, 101 → 1, 100 → 0
    // 011 → 1, 010 → 1, 001 → 1, 000 → 0
    // ============================================

    std::cout << "RULE 110 TRUTH TABLE:\n";
    std::cout << "=====================\n";
    std::cout << "  Pattern LCR → Next\n";
    std::cout << "  111 → 0\n";
    std::cout << "  110 → 1\n";
    std::cout << "  101 → 1\n";
    std::cout << "  100 → 0\n";
    std::cout << "  011 → 1\n";
    std::cout << "  010 → 1\n";
    std::cout << "  001 → 1\n";
    std::cout << "  000 → 0\n\n";

    // ============================================
    // 0-LEVEL IMPLEMENTATION ATTEMPT
    // ============================================

    std::cout << "0-LEVEL RULE 110 STEP:\n";
    std::cout << "======================\n";

    // Initialize with some pattern
    // Use a simple pattern: single 1 in middle
    std::vector<double> init_pattern(slots, 0.0);
    init_pattern[128] = phi_sq;  // Single "1" in middle

    auto current = make_vector_ct(init_pattern);

    std::cout << "  Initial pattern: 1 at position 128\n";
    std::cout << "  Running 10 steps...\n\n";

    int errors = 0;
    auto start = high_resolution_clock::now();

    for (int step = 0; step < 10; step++) {
        // Step 1: Get left and right neighbors via rotation (0 levels)
        auto left = cc->EvalRotate(current, 1);    // shift right, left neighbor
        auto right = cc->EvalRotate(current, -1);  // shift left, right neighbor

        // Step 2: Sum = left + current + right (0 levels)
        auto sum = cc->EvalAdd(left, current);
        sum = cc->EvalAdd(sum, right);

        // Step 3: Threshold — kailangan natin ng trick
        // Rule 110: next = 1 kung sum ∈ {φ², 2φ²}
        //           next = 0 kung sum ∈ {0, 3φ²}
        //
        // OBSERVATION: 
        // sum = 0 → next = 0
        // sum = φ² → next = φ²
        // sum = 2φ² → next = φ²
        // sum = 3φ² → next = 0
        //
        // Ito ay PARANG period-2: 0 → 0 → φ² → φ² → 0 → 0 → ...
        // Actually: next = φ² - (sum - φ²)²/φ²
        // PERO may multiplication... Kailangan ng ibang trick.

        // ALTERNATIVE: Use EvalSum for "soft thresholding"
        // If we average neighboring cells, we get smooth transitions
        auto soft_next = cc->EvalSub(ct_phi_sq, sum);
        // This gives: 3φ² → -2φ², 2φ² → -φ², φ² → 0, 0 → φ²
        // Not quite right, but let's see the pattern

        current = soft_next;

        // Check some positions
        if (step < 3 || step == 9) {
            std::cout << "  Step " << step << ": ";
            for (int pos = 125; pos <= 131; pos++) {
                double v = decrypt_val(current, pos);
                int bit = (std::abs(v) > phi_sq / 2.0) ? 1 : 0;
                std::cout << bit;
            }
            std::cout << " (level=" << current->GetLevel() << ")\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 10\n";
    std::cout << "  Time: " << duration.count() << "ms\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  (Kung 0 level, may potential!)\n";
    std::cout << "========================================\n";

    return 0;
}
