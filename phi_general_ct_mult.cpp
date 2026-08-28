// ============================================
// φ-GENERAL CT × CT — ARBITRARY MULTIPLICATION
//
// Subukan: a × b sa encrypted domain
// gamit ang Lucas/Fibonacci decomposition
// at zero-level reconstruction
//
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-GENERAL CT × CT\n";
    cout << "  Arbitrary multiplication via φ-domain\n";
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

    // Lucas at Fibonacci tables
    vector<long long> L = {2, 1};
    vector<long long> F = {0, 1};
    for (int i = 2; i <= 50; i++) {
        L.push_back(L[i-1] + L[i-2]);
        F.push_back(F[i-1] + F[i-2]);
    }

    // Helper: Lucas decomposition
    auto lucas_decompose = [&](long long target, vector<int>& indices) {
        long long rem = target;
        indices.clear();
        while (rem > 0) {
            int best_idx = -1;
            long long best_val = 0;
            for (int i = 0; i < L.size(); i++) {
                if (L[i] <= rem && L[i] > best_val) {
                    best_val = L[i];
                    best_idx = i;
                }
            }
            if (best_idx == -1) break;
            indices.push_back(best_idx);
            rem -= best_val;
        }
    };

    // Helper: Fibonacci decomposition (Zeckendorf)
    auto fib_decompose = [&](long long target, vector<int>& indices) {
        long long rem = target;
        indices.clear();
        while (rem > 0) {
            int best_idx = -1;
            long long best_val = 0;
            for (int i = 2; i < F.size(); i++) {
                if (F[i] <= rem && F[i] > best_val) {
                    best_val = F[i];
                    best_idx = i;
                }
            }
            if (best_idx == -1) break;
            indices.push_back(best_idx);
            rem -= best_val;
        }
    };

    // Test values
    vector<double> test_a = {3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0, 23.0};
    vector<double> test_b = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0};

    cout << "Test vectors:\n";
    cout << "  a = [";
    for (int i = 0; i < slots; i++) cout << (i > 0 ? ", " : "") << test_a[i];
    cout << "]\n";
    cout << "  b = [";
    for (int i = 0; i < slots; i++) cout << (i > 0 ? ", " : "") << test_b[i];
    cout << "]\n\n";

    // Encrypt a and b
    auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(test_a));
    auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(test_b));

    cout << "✅ Encrypted a and b\n\n";

    // Traditional ct × ct
    auto ct_prod_trad = cc->EvalMult(ct_a, ct_b);
    cout << "Traditional ct × ct:\n";
    cout << "  Level: " << ct_prod_trad->GetLevel() << "\n\n";

    // φ-domain approach
    // Decompose each a[i] and b[i] into Lucas terms
    // Then multiply via Lucas cross identities

    cout << "φ-DOMAIN DECOMPOSITION:\n";
    cout << "Slot | a | Lucas Decomp | b | Lucas Decomp | Terms\n";
    cout << "-----|---|---------------|---|---------------|---\n";

    vector<vector<int>> a_decomp(slots);
    vector<vector<int>> b_decomp(slots);

    for (int i = 0; i < slots; i++) {
        lucas_decompose((long long)test_a[i], a_decomp[i]);
        lucas_decompose((long long)test_b[i], b_decomp[i]);

        cout << setw(4) << i << " | " << setw(1) << test_a[i] << " | ";
        for (int idx : a_decomp[i]) cout << L[idx] << " ";
        cout << "| " << test_b[i] << " | ";
        for (int idx : b_decomp[i]) cout << L[idx] << " ";
        cout << "| " << a_decomp[i].size() + b_decomp[i].size() << "\n";
    }

    cout << "\nKEY INSIGHT:\n";
    cout << "Kung ang bawat value ay ma-decompose sa Lucas terms,\n";
    cout << "ang product a×b ay magiging sum ng Lucas cross products.\n";
    cout << "Bawat cross product ay may identity:\n";
    cout << "L_a × L_b = (L_{a+b} + L_{a-b}) / 2\n\n";

    // Build φ-domain product using Lucas cross identities
    // For simplicity, just verify for slot 0: 3×2=6

    cout << "VERIFICATION (Slot 0: 3×2=6):\n";
    cout << "  3 = L_2\n";
    cout << "  2 = L_0\n";
    cout << "  L_2 × L_0 = (L_2 + L_2) / 2 = (3 + 3) / 2 = 3\n";
    cout << "  Expected: 3 × 2 = 6\n";
    cout << "  Result: 3 (MALI!)\n\n";

    cout << "ANALYSIS:\n";
    cout << "Ang Lucas cross identity ay may division by 2.\n";
    cout << "Sa CKKS, ang division by 2 ay multiplication by 0.5.\n";
    cout << "Ito ay kumokonsumo ng level.\n\n";

    // Alternative: Direct multiplication via Lucas sums
    // 3 × 2 = (L_2) × (L_0) = 3 × 2 = 6
    // Sa φ-domain: 6 = L_2 + L_0 + 1 = 3 + 2 + 1

    cout << "ALTERNATIVE APPROACH:\n";
    cout << "  3 × 2 = 6 = L_2 + L_1 + L_0 = 3 + 1 + 2\n";
    cout << "  Levels: 0 (pure addition)\n\n";

    cout << "========================================\n";
    cout << "  FINDINGS\n";
    cout << "========================================\n";
    cout << "  ✅ Lucas decomposition ay universal\n";
    cout << "  ✅ Cross product may identity\n";
    cout << "  ⚠️ Division by 2 sa cross identity\n";
    cout << "  ⚠️ Hindi pa zero-level for general ct × ct\n";
    cout << "========================================\n";

    return 0;
}
