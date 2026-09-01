// ============================================
// φ-RULE 110 BFV — EXACT INTEGER SPACE
//
// BFV ay exact integer arithmetic—walang approximation.
// State: 0 → 0, 1 → 1 (plain integers)
// Transition: exact integer operations
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
    cout << "  φ-RULE 110 BFV — EXACT INTEGER\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(0);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    cout << "  ✅ BFV initialized (exact integers!)\n";
    cout << "  State: 0 → 0, 1 → 1\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto encrypt_bit = [&](int bit) {
        vector<int64_t> v(1, bit);
        Plaintext pt = cc->MakePackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bit = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return result_pt->GetPackedValue()[0];
    };

    // ============================================
    // TRANSITION TABLE (BFV)
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE (BFV)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Output | Expected | Match?\n";
    cout << "  ------|--------|----------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_bit(L);
                auto ct_C = encrypt_bit(C);
                auto ct_R = encrypt_bit(R);
                
                // Direct sum: L + C + R
                auto sum1 = cc->EvalAdd(ct_L, ct_C);
                auto sum2 = cc->EvalAdd(sum1, ct_R);
                
                int64_t sum_val = decrypt_bit(sum2);
                int output = (sum_val == 1 || sum_val == 2) ? 1 : 0;
                
                // For Rule 110: 
                // 000→0, 001→1, 010→1, 011→0, 100→1, 101→1, 110→1, 111→0
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (output == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    return 0;
}
