// Quick debug: verify AND/OR/NOT primitives work in dual encoding
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

DualGate unified_nand(CryptoContext<DCRTPoly>& cc, const DualGate& X, const DualGate& Y) {
    auto a1a2 = cc->EvalMult(X.a, Y.a);
    auto b1b2 = cc->EvalMult(X.b, Y.b);
    auto one = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto a_out = cc->EvalSub(one, cc->EvalAdd(a1a2, b1b2));
    auto a1b2 = cc->EvalMult(X.a, Y.b);
    auto a2b1 = cc->EvalMult(Y.a, X.b);
    auto sum = cc->EvalAdd(cc->EvalAdd(a1b2, a2b1), b1b2);
    auto neg_one = cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    auto b_out = cc->EvalMult(neg_one, sum);
    return {a_out, b_out};
}

double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
double decode_dual(DualGate& dg, CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double root) {
    return decrypt_val(cc, kp, dg.a) + decrypt_val(cc, kp, dg.b) * root;
}
DualGate encode_dual(CryptoContext<DCRTPoly>& cc, const DualGate& gA, const DualGate& gB) {
    auto bA_phi = cc->EvalMult(gA.b, cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI}));
    auto val_A = cc->EvalAdd(gA.a, bA_phi);
    auto bB_psi = cc->EvalMult(gB.b, cc->MakeCKKSPackedPlaintext(std::vector<double>{PSI}));
    auto val_B = cc->EvalAdd(gB.a, bB_psi);
    auto diff = cc->EvalSub(val_A, val_B);
    auto inv_denom = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0 / (PHI - PSI)});
    auto b_out = cc->EvalMult(diff, inv_denom);
    auto b_phi = cc->EvalMult(b_out, cc->MakeCKKSPackedPlaintext(std::vector<double>{PHI}));
    auto a_out = cc->EvalSub(val_A, b_phi);
    return {a_out, b_out};
}
DualGate make_input(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double val) {
    return {cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            cc->Encrypt(kp.publicKey, cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

int main() {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(30); p.SetScalingModSize(50); p.SetBatchSize(1024);
    p.SetRingDim(8192); p.SetSecretKeyDist(UNIFORM_TERNARY); p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    std::cout << "\n=== DUAL GATE DEBUG ===\n\n";
    
    // Test: encode two inputs (1 and 0) into one DualGate
    DualGate gA = make_input(cc, kp, 1.0); // Circuit A sees 1
    DualGate gB = make_input(cc, kp, 0.0); // Circuit B sees 0
    DualGate merged = encode_dual(cc, gA, gB);
    
    double phi_out = decode_dual(merged, cc, kp, PHI);
    double psi_out = decode_dual(merged, cc, kp, PSI);
    
    std::cout << "Input A=1, B=0 encoded:\n";
    std::cout << "  φ-decode: " << phi_out << " (expect 1.0)\n";
    std::cout << "  ψ-decode: " << psi_out << " (expect 0.0)\n\n";
    
    // Test: NAND truth table in dual encoding
    std::cout << "Dual NAND truth table:\n";
    for (int a=0; a<=1; a++) {
        for (int b=0; b<=1; b++) {
            DualGate X = make_input(cc, kp, (double)a);
            DualGate Y = make_input(cc, kp, (double)b);
            DualGate R = unified_nand(cc, X, Y);
            double val = decode_dual(R, cc, kp, PHI);
            int bit = (val > 0.5) ? 1 : 0;
            int exp = !(a & b);
            std::cout << "  NAND(" << a << "," << b << ") = " << bit << " (expect " << exp << ") " 
                      << (bit == exp ? "OK" : "FAIL") << " val=" << val << "\n";
        }
    }
    
    return 0;
}
