#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
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
        v[0] = (bit == 0) ? -2.0 : 2.0;
        v[4] = (bit == 0) ? 0.0 : 1.0;
        for (int i = 1; i < 4; i++) v[i] = v[0] * pow(PHI, i);
        for (int i = 5; i < 8; i++) v[i] = v[4] * pow(PHI, i-4);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_normal = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(8);
        return result_pt->GetCKKSPackedValue()[4].real();
    };

    auto gate_xor = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, cc->EvalNegate(b));
    };

    auto gate_and = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    };

    auto gate_or = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    };

    cout << "  PROBLEM CASES:\n\n";

    for (auto [A, B, Cin] : vector<tuple<int,int,int>>{{0,0,1}, {0,1,1}, {1,0,0}}) {
        auto ct_a = encrypt_dual(A);
        auto ct_b = encrypt_dual(B);
        auto ct_cin = encrypt_dual(Cin);

        auto xor_ab = gate_xor(ct_a, ct_b);
        auto sum_ct = gate_xor(xor_ab, ct_cin);
        auto and_ab = gate_and(ct_a, ct_b);
        auto and_cin_xor = gate_and(ct_cin, xor_ab);
        auto cout_ct = gate_or(and_ab, and_cin_xor);

        cout << "  A=" << A << " B=" << B << " Cin=" << Cin << "\n";
        cout << "  XOR(A,B) norm: " << decrypt_normal(xor_ab) << "\n";
        cout << "  Sum norm: " << decrypt_normal(sum_ct) << "\n";
        cout << "  AND(A,B) norm: " << decrypt_normal(and_ab) << "\n";
        cout << "  AND(Cin,XOR) norm: " << decrypt_normal(and_cin_xor) << "\n";
        cout << "  Cout norm: " << decrypt_normal(cout_ct) << "\n\n";
    }

    return 0;
}
