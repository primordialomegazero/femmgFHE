// ============================================
// φ-CPU DEBUG — TINGNAN ANG VALUES
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-CPU DEBUG\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(64);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    auto encrypt_emergent = [&](int bit, int alu_id) {
        vector<double> v(64, 0.0);
        double val = (bit == 0) ? PHI : PHI_INV;
        int base = alu_id * 4;
        for (int i = 0; i < 4; i++) v[base + i] = val;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_emergent = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(64);
        vector<double> results(64);
        for (int i = 0; i < 64; i++) {
            results[i] = result_pt->GetCKKSPackedValue()[i].real();
        }
        return results;
    };

    cout << "  DEBUG: PROBLEM CASES\n\n";

    for (auto [A, B, Cin] : vector<tuple<int,int,int>>{{0,1,0}, {0,1,1}, {1,0,0}, {1,0,1}}) {
        auto ct_a = encrypt_emergent(A, 0);
        auto ct_b = encrypt_emergent(B, 1);
        auto ct_cin = encrypt_emergent(Cin, 2);
        
        auto ct_stage1 = cc->EvalAdd(ct_a, ct_b);
        auto ct_sum = cc->EvalAdd(ct_stage1, ct_cin);
        
        auto stage1_vals = decrypt_emergent(ct_stage1);
        auto sum_vals = decrypt_emergent(ct_sum);
        
        cout << "  A=" << A << " B=" << B << " Cin=" << Cin << "\n";
        cout << "  Stage1 (ALU 0-1): ";
        for (int i = 0; i < 8; i++) cout << fixed << setprecision(2) << stage1_vals[i] << " ";
        cout << "\n";
        cout << "  Sum (ALU 2): ";
        for (int i = 8; i < 12; i++) cout << fixed << setprecision(2) << sum_vals[i] << " ";
        cout << "\n";
        
        double sum_avg = (sum_vals[8] + sum_vals[9] + sum_vals[10] + sum_vals[11]) / 4.0;
        double normalized = (PHI - sum_avg) / (PHI - PHI_INV);
        double mod2 = normalized - 2.0 * floor(normalized / 2.0);
        if (mod2 > 1.0) mod2 = 2.0 - mod2;
        int sum = (int)round(mod2);
        
        cout << "  Sum avg: " << sum_avg << " → normalized: " << normalized << " → mod2: " << mod2 << " → sum: " << sum << "\n\n";
    }

    cout << "  KEY QUESTION:\n";
    cout << "  Ang dating 8/8 ay may 16 slots per value — hindi 4!\n";
    cout << "  Kaya ang averaging ay mas accurate!\n\n";

    return 0;
}
