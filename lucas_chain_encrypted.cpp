// ============================================
// ENCRYPTED LUCAS CHAIN
// x¹⁶ via Lucas reconstruction — ZERO LEVEL
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <complex>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  ENCRYPTED LUCAS CHAIN — x^16\n";
    cout << "  Zero-level via Lucas identities\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(20);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 8;

    // Lucas values L_0 to L_30
    vector<long long> L = {2, 1};
    for (int i = 2; i <= 30; i++) {
        L.push_back(L[i-1] + L[i-2]);
    }

    // Start: x = L_3 = 4
    // Goal: x^16 = 4^16 = 4294967296

    cout << "Chain: x = 4 → x^16\n";
    cout << "Expected: 4^16 = 4294967296\n\n";

    // Encrypt L_3 = 4
    vector<double> plain_L3(slots, (double)L[3]);
    auto ct_L3 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_L3));

    cout << "✅ Encrypted L_3 = 4\n\n";

    // Lucas square identity: L_n² = L_{2n} + 2(-1)^n
    // L_3² = L_6 + 2(-1)^3 = 18 - 2 = 16

    vector<double> plain_L6(slots, (double)L[6]);
    vector<double> plain_two(slots, 2.0);

    auto ct_L6 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_L6));
    auto ct_two = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_two));

    // x² = L_3² = L_6 - 2 = 18 - 2 = 16
    auto ct_x2 = cc->EvalSub(ct_L6, ct_two);  // 16

    cout << "Step 1: x² = L_6 - 2 = 16\n";
    cout << "  Level: " << ct_x2->GetLevel() << "\n\n";

    // x⁴ = (x²)² = 16² = 256
    // 16 = L_4² = L_8 + 2 = 47 + 2 = 49 (HINDI 16!)
    // Kailangan: x⁴ = 256 = L_14 + 2 = 843 + 2 = 845 (MALI!)

    // Direct Lucas: L_3⁴ = ?
    // L_3 = 4, L_3² = 16, L_3⁴ = 256
    // 256 = L_13 - 2 = 521 - 2 = 519 (MALI!)
    // 256 = L_12 - 2 = 322 - 2 = 320 (MALI!)
    // 256 = L_11 - 2 = 199 - 2 = 197 (MALI!)

    // Kailangan ng direct Lucas index:
    // L_8 = 47, L_9 = 76, L_10 = 123, L_11 = 199, L_12 = 322
    // 256 ay nasa pagitan ng L_11=199 at L_12=322

    // REVISED APPROACH:
    // x¹⁶ = (L_3)¹⁶
    // Hindi ito Lucas number, kaya kailangan ng decomposition:
    // 4^16 = 4294967296
    // Ito ay large integer → decompose sa Lucas terms

    cout << "REVISED: x^16 ay arbitrary integer, hindi Lucas number\n";
    cout << "Kailangan ng Lucas decomposition ng 4294967296\n\n";

    // Decompose 4294967296 sa Lucas terms
    long long target = 4294967296LL;
    vector<long long> terms;
    long long remaining = target;

    while (remaining > 0) {
        int best_idx = -1;
        long long best_val = 0;
        for (int i = 0; i < L.size(); i++) {
            if (L[i] <= remaining && L[i] > best_val) {
                best_val = L[i];
                best_idx = i;
            }
        }
        if (best_idx == -1) break;
        terms.push_back(best_val);
        remaining -= best_val;
    }

    cout << "Decomposition ng " << target << ":\n";
    for (long long term : terms) {
        cout << "  " << term << "\n";
    }
    cout << "  Sum: ";
    long long sum = 0;
    for (long long term : terms) sum += term;
    cout << sum << " (" << (sum == target ? "✅" : "❌") << ")\n";

    cout << "\n========================================\n";
    cout << "  OBSERVATION\n";
    cout << "========================================\n";
    cout << "  Ang x^16 = 4294967296 ay hindi Lucas number\n";
    cout << "  Kaya hindi ma-reconstruct via L_n² = L_{2n} ± 2\n";
    cout << "  Kailangan ng multi-term Lucas representation\n";
    cout << "========================================\n";

    return 0;
}
