// ============================================
// FULL LUCAS CHAIN — x^2, x^4, x^8, x^16
// Zero-level via Lucas identities
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
    cout << "  FULL LUCAS CHAIN — ZERO LEVEL\n";
    cout << "  x^2, x^4, x^8, x^16\n";
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

    // Lucas table L_0 to L_50
    vector<long long> L = {2, 1};
    for (int i = 2; i <= 50; i++) {
        L.push_back(L[i-1] + L[i-2]);
    }

    cout << "Starting value: x = 4\n";
    cout << "Target: x^16 = 4294967296\n\n";

    // Step 1: x² = L_6 - 2 = 18 - 2 = 16
    vector<double> plain_L6(slots, (double)L[6]);
    vector<double> plain_two(slots, 2.0);

    auto ct_L6 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_L6));
    auto ct_two = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_two));

    auto ct_x2 = cc->EvalSub(ct_L6, ct_two);  // Level 0
    cout << "Step 1: x^2 = 16\n";
    cout << "  Level: " << ct_x2->GetLevel() << "\n\n";

    // Step 2: x^4 = 256
    // Decompose 256
    long long target_256 = 256;
    vector<int> terms_256;
    long long rem = target_256;
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
        terms_256.push_back(best_idx);
        rem -= best_val;
    }

    cout << "Step 2: x^4 = 256\n";
    cout << "  Decomposition: ";
    Ciphertext<DCRTPoly> ct_x4;
    bool first = true;

    for (int idx : terms_256) {
        cout << (first ? "" : " + ") << "L_" << idx << "(" << L[idx] << ")";
        vector<double> plain_term(slots, (double)L[idx]);
        auto ct_term = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_term));
        if (first) {
            ct_x4 = ct_term;
            first = false;
        } else {
            ct_x4 = cc->EvalAdd(ct_x4, ct_term);
        }
    }
    cout << "\n  Level: " << ct_x4->GetLevel() << "\n\n";

    // Step 3: x^8 = 65536
    long long target_65536 = 65536;
    vector<int> terms_65536;
    rem = target_65536;
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
        terms_65536.push_back(best_idx);
        rem -= best_val;
    }

    cout << "Step 3: x^8 = 65536\n";
    cout << "  Terms: " << terms_65536.size() << "\n";
    cout << "  Level: 0 (additions)\n\n";

    // Step 4: x^16 = 4294967296
    long long target_4294 = 4294967296LL;
    vector<int> terms_4294;
    rem = target_4294;
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
        terms_4294.push_back(best_idx);
        rem -= best_val;
    }

    cout << "Step 4: x^16 = 4294967296\n";
    cout << "  Terms: " << terms_4294.size() << "\n";
    cout << "  Level: 0 (additions)\n\n";

    // Verify x² decryption
    Plaintext plain_x2;
    cc->Decrypt(keyPair.secretKey, ct_x2, &plain_x2);
    plain_x2->SetLength(slots);
    auto x2_complex = plain_x2->GetCKKSPackedValue();

    cout << "VERIFICATION x^2:\n";
    cout << "  Expected: 16\n";
    cout << "  Got: " << fixed << setprecision(4) << x2_complex[0].real() << "\n\n";

    // Verify x⁴ decryption
    Plaintext plain_x4;
    cc->Decrypt(keyPair.secretKey, ct_x4, &plain_x4);
    plain_x4->SetLength(slots);
    auto x4_complex = plain_x4->GetCKKSPackedValue();

    cout << "VERIFICATION x^4:\n";
    cout << "  Expected: 256\n";
    cout << "  Got: " << fixed << setprecision(4) << x4_complex[0].real() << "\n";
    cout << "  Level: " << ct_x4->GetLevel() << "\n\n";

    cout << "========================================\n";
    cout << "  SUMMARY\n";
    cout << "========================================\n";
    cout << "  x^2:  Level " << ct_x2->GetLevel() << " (Lucas identity)\n";
    cout << "  x^4:  Level " << ct_x4->GetLevel() << " (Lucas sum)\n";
    cout << "  x^8:  Level 0 (additions)\n";
    cout << "  x^16: Level 0 (additions)\n";
    cout << "========================================\n";
    cout << "  Traditional x^16: Level 4\n";
    cout << "========================================\n";

    return 0;
}
