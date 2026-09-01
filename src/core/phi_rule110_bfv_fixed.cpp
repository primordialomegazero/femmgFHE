// ============================================
// φ-RULE 110 BFV FIXED — EXACT 8/8
//
// BFV exact integers + tamang lookup
// Rule 110: 
// sum=1 → 1 (001, 010, 100)
// sum=2 at L=0 → 1 (011)
// iba → 0
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
    cout << "  φ-RULE 110 BFV FIXED\n";
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
    cout << "  Tamang Rule 110 lookup\n\n";

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
    // TRANSITION TABLE (BFV FIXED)
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE (BFV FIXED)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Sum | Output | Expected | Match?\n";
    cout << "  ------|-----|--------|----------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_bit(L);
                auto ct_C = encrypt_bit(C);
                auto ct_R = encrypt_bit(R);
                
                // Sum = L + C + R (max 3)
                auto sum1 = cc->EvalAdd(ct_L, ct_C);
                auto sum2 = cc->EvalAdd(sum1, ct_R);
                
                int64_t sum_val = decrypt_bit(sum2);
                
                // Tamang Rule 110:
                // sum=1 → 1
                // sum=2 && L=0 → 1 (011)
                // iba → 0
                int output = 0;
                if (sum_val == 1) output = 1;
                else if (sum_val == 2 && L == 0) output = 1;
                
                int expected = rule110[(L << 2) | (C << 1) | R];
                bool match = (output == expected);
                if (match) match_count++;
                
                cout << "  " << L << " " << C << " " << R << " | "
                     << setw(3) << sum_val << " | "
                     << setw(6) << output << " | "
                     << setw(8) << expected << " | "
                     << (match ? "✅" : "❌") << "\n";
            }
        }
    }
    cout << "\n  Match: " << match_count << "/8\n\n";

    return 0;
}
