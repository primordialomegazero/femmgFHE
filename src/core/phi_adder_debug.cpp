// ============================================
// φ-FULL ADDER DEBUG — PRINT VALUES
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-FULL ADDER DEBUG\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(0);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);

    const double PHI = 1.6180339887498948482;

    auto encrypt_dual = [&](int bit) {
        vector<double> v(8, 0.0);
        v[0] = (bit == 0) ? -2.0 : 2.0;  // Log space
        v[4] = (bit == 0) ? 0.0 : 1.0;   // Normal space
        for (int i = 1; i < 4; i++) v[i] = v[0] * pow(PHI, i);
        for (int i = 5; i < 8; i++) v[i] = v[4] * pow(PHI, i-4);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_raw = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        double log_val = result_pt->GetCKKSPackedValue()[0].real();
        double normal_val = result_pt->GetCKKSPackedValue()[4].real();
        return make_pair(log_val, normal_val);
    };

    // Gates
    auto gate_nand = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalNegate(cc->EvalAdd(a, b));
    };

    auto gate_and = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    };

    auto gate_or = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto not_a = cc->EvalNegate(a);
        auto not_b = cc->EvalNegate(b);
        return cc->EvalNegate(cc->EvalAdd(not_a, not_b));
    };

    auto gate_xor = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, cc->EvalNegate(b));
    };

    // ============================================
    // DEBUG: PROBLEM CASES
    // ============================================

    cout << "  PROBLEM CASES:\n";
    cout << "  ==============\n\n";

    for (auto [A, B, Cin] : vector<tuple<int,int,int>>{{0,1,1}, {1,0,0}}) {
        auto ct_a = encrypt_dual(A);
        auto ct_b = encrypt_dual(B);
        auto ct_cin = encrypt_dual(Cin);

        // XOR(A,B)
        auto xor_ab = gate_xor(ct_a, ct_b);
        auto [xor_ab_log, xor_ab_norm] = decrypt_raw(xor_ab);

        // Sum = XOR(XOR(A,B), Cin)
        auto sum_ct = gate_xor(xor_ab, ct_cin);
        auto [sum_log, sum_norm] = decrypt_raw(sum_ct);

        // AND(A,B)
        auto and_ab = gate_and(ct_a, ct_b);
        auto [and_ab_log, and_ab_norm] = decrypt_raw(and_ab);

        // AND(Cin, XOR(A,B))
        auto and_cin_xor = gate_and(ct_cin, xor_ab);
        auto [and_cin_xor_log, and_cin_xor_norm] = decrypt_raw(and_cin_xor);

        // Cout = OR(AND(A,B), AND(Cin, XOR(A,B)))
        auto cout_ct = gate_or(and_ab, and_cin_xor);
        auto [cout_log, cout_norm] = decrypt_raw(cout_ct);

        cout << "  A=" << A << " B=" << B << " Cin=" << Cin << "\n\n";
        cout << "  XOR(A,B):     log=" << xor_ab_log << ", norm=" << xor_ab_norm << "\n";
        cout << "  Sum:          log=" << sum_log << ", norm=" << sum_norm << "\n";
        cout << "  AND(A,B):     log=" << and_ab_log << ", norm=" << and_ab_norm << "\n";
        cout << "  AND(Cin,XOR): log=" << and_cin_xor_log << ", norm=" << and_cin_xor_norm << "\n";
        cout << "  Cout:         log=" << cout_log << ", norm=" << cout_norm << "\n\n";

        cout << "  Expected: Sum=" << ((A+B+Cin)%2) << ", Cout=" << ((A+B+Cin)/2) << "\n\n";
        cout << "  ----------------------------------------\n\n";
    }

    return 0;
}
