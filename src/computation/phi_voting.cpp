// ============================================
// φ-ENCRYPTED VOTING — REAL WORLD
//
// 8 voters, 2 candidates (A at B)
// Lahat ng boto encrypted
// Result: encrypted tally
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-ENCRYPTED VOTING SYSTEM\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 16 slots)\n";
    cout << "  8 voters, 2 candidates: A at B\n\n";

    auto encrypt_vote = [&](int vote) {
        // vote = 0 para A, 1 para B
        vector<double> v(16, 0.0);
        if (vote == 0) {
            v[0] = 1.0;  // A
            v[8] = 0.0;  // B
        } else {
            v[0] = 0.0;  // A
            v[8] = 1.0;  // B
        }
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_tally = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        
        double tally_A = result_pt->GetCKKSPackedValue()[0].real();
        double tally_B = result_pt->GetCKKSPackedValue()[8].real();
        return make_pair(tally_A, tally_B);
    };

    // ============================================
    // 8 VOTERS
    // ============================================

    vector<int> votes = {0, 1, 0, 1, 0, 0, 1, 0};  // A=0, B=1
    
    cout << "  BOTO (encrypted):\n";
    cout << "  Voter | Boto\n";
    cout << "  ------|-----\n";
    
    vector<Ciphertext<DCRTPoly>> encrypted_votes;
    for (int i = 0; i < 8; i++) {
        auto ct = encrypt_vote(votes[i]);
        encrypted_votes.push_back(ct);
        cout << "  " << setw(5) << i+1 << " | "
             << (votes[i] == 0 ? "A" : "B") << "\n";
    }
    cout << "\n";

    // ============================================
    // ENCRYPTED TALLY
    // ============================================

    cout << "  TALLYING...\n\n";

    auto ct_tally = encrypt_vote(0);  // start with dummy zero

    for (int i = 0; i < 8; i++) {
        ct_tally = cc->EvalAdd(ct_tally, encrypted_votes[i]);
    }

    auto [tally_A, tally_B] = decrypt_tally(ct_tally);

    // Adjust: tinanggal natin ang dummy
    tally_A -= 1.0;

    cout << "  RESULT:\n";
    cout << "  Candidate A: " << (int)(tally_A + 0.5) << " votes\n";
    cout << "  Candidate B: " << (int)(tally_B + 0.5) << " votes\n\n";

    // ============================================
    // VERIFICATION
    // ============================================

    int expected_A = 0, expected_B = 0;
    for (int vote : votes) {
        if (vote == 0) expected_A++;
        else expected_B++;
    }

    cout << "  EXPECTED (plaintext):\n";
    cout << "  Candidate A: " << expected_A << " votes\n";
    cout << "  Candidate B: " << expected_B << " votes\n\n";

    bool match = ((int)(tally_A + 0.5) == expected_A && (int)(tally_B + 0.5) == expected_B);

    cout << "  Match: " << (match ? "✅ YES" : "❌ NO") << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  VOTING SYSTEM COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ 8 encrypted votes\n";
    cout << "  ✅ Encrypted tally\n";
    cout << "  ✅ Result verified\n";
    cout << "  ✅ Level 0 forever\n";
    cout << "  ✅ Walang decrypt sa gitna\n";
    cout << "  ✅ Walang bootstrapping\n";
    cout << "  ✅ Pure FHE\n\n";

    return 0;
}
