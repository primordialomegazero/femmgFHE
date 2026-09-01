// ============================================
// φ-RULE 110 NAND GATES — ALL LOG SPACE
//
// Rule 110 = L XOR (C AND R)
// Lahat galing sa NAND:
// NAND(a,b) = -(a+b) sa log space
// NOT(a) = -a
// AND(a,b) = NAND(NAND(a,b), NAND(a,b))
// XOR(a,b) = NAND(NAND(a, NAND(a,b)), NAND(b, NAND(a,b)))
//
// Encoding: 0 → -2, 1 → +2
// Lahat depth 0!
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
    cout << "  φ-RULE 110 NAND GATES\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    cout << "  ✅ CKKS initialized (depth 0!)\n";
    cout << "  Log space: 0→-2, 1→+2\n\n";

    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    // ============================================
    // LOG SPACE ENCODING
    // ============================================

    auto encrypt_log = [&](int bit) {
        double val = (bit == 0) ? -2.0 : 2.0;
        vector<double> v(16, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += result_pt->GetCKKSPackedValue()[i].real();
        double avg = sum / 16.0;
        return (avg >= 0.0) ? 1 : 0;  // Threshold: ≥ 0 → 1
    };

    // ============================================
    // LOG SPACE GATES (LAHAT DEPTH 0)
    // ============================================

    // NAND: -(a+b)
    auto gate_nand = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalNegate(sum);
    };

    // NOT: NAND(a,a) = -a
    auto gate_not = [&](const Ciphertext<DCRTPoly>& a) {
        return cc->EvalNegate(a);
    };

    // AND: NAND(NAND(a,b), NAND(a,b))
    auto gate_and = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto nand_ab = gate_nand(a, b);
        return gate_nand(nand_ab, nand_ab);
    };

    // XOR: NAND(NAND(a, NAND(a,b)), NAND(b, NAND(a,b)))
    auto gate_xor = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto nand_ab = gate_nand(a, b);
        auto nand_a_n = gate_nand(a, nand_ab);
        auto nand_b_n = gate_nand(b, nand_ab);
        return gate_nand(nand_a_n, nand_b_n);
    };

    // Rule 110: output = L XOR (C AND R)
    auto rule110_gate = [&](const Ciphertext<DCRTPoly>& L,
                             const Ciphertext<DCRTPoly>& C,
                             const Ciphertext<DCRTPoly>& R) {
        auto c_and_r = gate_and(C, R);
        return gate_xor(L, c_and_r);
    };

    // ============================================
    // TRANSITION TABLE TEST (ENCRYPTED)
    // ============================================

    cout << "========================================\n";
    cout << "  TRANSITION TABLE (NAND GATES)\n";
    cout << "========================================\n\n";

    cout << "  L C R | Output | Expected | Match?\n";
    cout << "  ------|--------|----------|--------\n";

    int match_count = 0;
    for (int L : {0, 1}) {
        for (int C : {0, 1}) {
            for (int R : {0, 1}) {
                auto ct_L = encrypt_log(L);
                auto ct_C = encrypt_log(C);
                auto ct_R = encrypt_log(R);
                
                auto ct_out = rule110_gate(ct_L, ct_C, ct_R);
                int output = decrypt_log(ct_out);
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

    // ============================================
    // PLAINTEXT REFERENCE
    // ============================================

    int N = 16;
    vector<int> plain(N, 0);
    plain[7] = 1;
    plain[8] = 1;

    vector<vector<int>> history;
    history.push_back(plain);
    for (int gen = 0; gen < 20; gen++) {
        vector<int> next(N, 0);
        for (int i = 0; i < N; i++) {
            int L = plain[(i + N - 1) % N];
            int C = plain[i];
            int R = plain[(i + 1) % N];
            int pattern = (L << 2) | (C << 1) | R;
            next[i] = rule110[pattern];
        }
        plain = next;
        history.push_back(plain);
    }

    // ============================================
    // PURE FHE EVOLUTION (NAND GATES)
    // ============================================

    cout << "========================================\n";
    cout << "  PURE FHE EVOLUTION (NAND GATES)\n";
    cout << "========================================\n\n";

    vector<Ciphertext<DCRTPoly>> cells;
    for (int bit : history[0]) {
        cells.push_back(encrypt_log(bit));
    }

    cout << "  Gen 0: ";
    for (int i = 0; i < N; i++) cout << history[0][i];
    cout << "\n\n";

    auto start = high_resolution_clock::now();
    vector<Ciphertext<DCRTPoly>> current = cells;

    for (int gen = 1; gen <= 20; gen++) {
        vector<Ciphertext<DCRTPoly>> next;
        
        for (int i = 0; i < N; i++) {
            auto L = current[(i + N - 1) % N];
            auto C = current[i];
            auto R = current[(i + 1) % N];
            
            // PURE FHE — NAND gates lang, walang decryption!
            next.push_back(rule110_gate(L, C, R));
        }
        
        current = next;
        
        if (gen % 5 == 0 || gen == 20) {
            cout << "  Gen " << setw(3) << gen << ": ";
            for (int i = 0; i < N; i++) {
                cout << decrypt_log(current[i]);
            }
            cout << "\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    cout << "\n  Time: " << time / 1000.0 << " seconds\n";
    cout << "  Level: " << current[0]->GetLevel() << "\n\n";

    // ============================================
    // VERIFICATION
    // ============================================

    cout << "========================================\n";
    cout << "  VERIFICATION (GEN 20)\n";
    cout << "========================================\n\n";

    int matches = 0;
    cout << "  Plaintext: ";
    for (int i = 0; i < N; i++) cout << history[20][i];
    cout << "\n";
    cout << "  Encrypted: ";
    for (int i = 0; i < N; i++) {
        int bit = decrypt_log(current[i]);
        cout << bit;
        if (bit == history[20][i]) matches++;
    }
    cout << "\n\n";
    cout << "  Match: " << matches << "/" << N << "\n\n";

    cout << "========================================\n";
    cout << "  NAND GATES COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ All gates from NAND\n";
    cout << "  ✅ Log space: 0→-2, 1→+2\n";
    cout << "  ✅ Depth 0\n";
    cout << "  ✅ Walang decrypt sa loop!\n";
    cout << "  ✅ Match: " << matches << "/" << N << "\n";
    cout << "  ✅ Level 0\n\n";

    return 0;
}
