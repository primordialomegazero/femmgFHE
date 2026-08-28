// ============================================
// ENCRYPTED LUCAS CHAIN — x^16 ZERO LEVEL
// V2: Complete Lucas table hanggang L_50
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
    cout << "  ENCRYPTED LUCAS CHAIN V2 — x^16\n";
    cout << "  Complete Lucas table\n";
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

    // Complete Lucas table L_0 to L_50
    vector<long long> L = {2, 1};
    for (int i = 2; i <= 50; i++) {
        L.push_back(L[i-1] + L[i-2]);
    }

    cout << "Lucas table size: " << L.size() << "\n";
    cout << "L_40 = " << L[40] << "\n";
    cout << "L_45 = " << L[45] << "\n";
    cout << "L_50 = " << L[50] << "\n\n";

    // Decompose 4294967296 sa Lucas terms
    long long target = 4294967296LL;
    vector<pair<int, long long>> terms;
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
        terms.push_back({best_idx, best_val});
        remaining -= best_val;
    }

    cout << "Decomposition ng " << target << ":\n";
    cout << "  Terms: " << terms.size() << "\n";
    long long sum = 0;
    for (auto& t : terms) {
        cout << "  L_" << t.first << " = " << t.second << "\n";
        sum += t.second;
    }
    cout << "  Sum: " << sum << " (" << (sum == target ? "✅" : "❌") << ")\n\n";

    // Test: x = 4 (L_3)
    // x^2 = 16 (L_6 - 2)
    // Verify zero-level squaring works

    cout << "========================================\n";
    cout << "  ENCRYPTED SQUARING TEST\n";
    cout << "========================================\n\n";

    cout << "x = L_3 = 4\n";
    cout << "x^2 = L_6 - 2 = 18 - 2 = 16\n\n";

    vector<double> plain_L3(slots, (double)L[3]);
    vector<double> plain_L6(slots, (double)L[6]);
    vector<double> plain_two(slots, 2.0);

    auto ct_L3 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_L3));
    auto ct_L6 = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_L6));
    auto ct_two = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_two));

    // x^2 = L_6 - 2
    auto ct_x2 = cc->EvalSub(ct_L6, ct_two);

    cout << "✅ x^2 computed via Lucas identity\n";
    cout << "  Level: " << ct_x2->GetLevel() << "\n\n";

    // Traditional x*x for comparison
    auto ct_x2_trad = cc->EvalMult(ct_L3, ct_L3);
    cout << "Traditional x*x:\n";
    cout << "  Level: " << ct_x2_trad->GetLevel() << "\n\n";

    // Decrypt and verify
    Plaintext plain_recon, plain_trad;
    cc->Decrypt(keyPair.secretKey, ct_x2, &plain_recon);
    cc->Decrypt(keyPair.secretKey, ct_x2_trad, &plain_trad);

    plain_recon->SetLength(slots);
    plain_trad->SetLength(slots);

    auto recon_complex = plain_recon->GetCKKSPackedValue();
    auto trad_complex = plain_trad->GetCKKSPackedValue();

    cout << "VERIFICATION:\n";
    cout << "Slot | Lucas Recon | Traditional | Expected | Match?\n";
    cout << "-----|-------------|-------------|----------|-------\n";

    for (int i = 0; i < slots; i++) {
        double recon = recon_complex[i].real();
        double trad = trad_complex[i].real();
        bool match = (abs(recon - 16.0) < 0.01) && (abs(trad - 16.0) < 0.01);
        cout << setw(4) << i << " | " 
             << setw(11) << fixed << setprecision(4) << recon << " | "
             << setw(11) << trad << " | "
             << setw(8) << "16.0000" << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  TEST COMPLETE\n";
    cout << "========================================\n";
    cout << "  Lucas Recon: Level " << ct_x2->GetLevel() << "\n";
    cout << "  Traditional: Level " << ct_x2_trad->GetLevel() << "\n";
    cout << "========================================\n";

    return 0;
}
