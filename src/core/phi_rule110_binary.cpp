// ============================================
// φ-RULE 110 BINARY — DIRECT BIT ENCODING
//
// Direct binary: 0 → 0, 1 → 1
// Walang φ, walang log, walang approximation
// Pure integer arithmetic
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
    cout << "  φ-RULE 110 BINARY\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(2);
    parameters.SetMultiplicativeDepth(0);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    cout << "  ✅ BFV initialized (mod 2!)\n";
    cout << "  Pure binary: mod 2 arithmetic\n\n";

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
    // TRANSITION TABLE (BINARY)
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE (BINARY)\n";
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
                
                // Binary sum: L + C + R (mod 2)
                auto sum1 = cc->EvalAdd(ct_L, ct_C);
                auto sum2 = cc->EvalAdd(sum1, ct_R);
                
                int64_t sum_val = decrypt_bit(sum2);
                int output = (sum_val == 1) ? 1 : 0;
                
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
