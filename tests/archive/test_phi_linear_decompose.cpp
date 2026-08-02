// Linear Decomposition v2: Include bias in effective slope/intercept
#include <iostream>
#include <iomanip>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
PE ratio_add(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto a1b2=cc->EvalMult(x.a,y.b), a2b1=cc->EvalMult(y.a,x.b), b1b2=cc->EvalMult(x.b,y.b);
    return {cc->EvalAdd(a1b2,a2b1), b1b2};
}
PE ratio_mult(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    return {cc->EvalMult(x.a,y.a), cc->EvalMult(x.b,y.b)};
}
double decrypt_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; cc->Decrypt(kp.secretKey, c, &pt); return pt->GetCKKSPackedValue()[0].real();
}
double decode_state(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    return decrypt_val(cc,kp,s.a) / decrypt_val(cc,kp,s.b) - 0.6180339887498949;
}
PE encrypt_value(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double v) {
    const double PSI=0.6180339887498949;
    return {cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{v+PSI})),
            cc->Encrypt(kp.publicKey,cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0}))};
}
const double PSI=0.6180339887498949, PSI2=PSI*PSI;

int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║   Linear Decomposition v2: Effective slope/intercept with bias    ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    double true_A=0.5, true_B=0.3;
    double expected=((true_A+true_B)*true_B+true_A);
    
    PE enc_A=encrypt_value(cc,kp,true_A), enc_B=encrypt_value(cc,kp,true_B);
    PE gate1=ratio_add(cc,enc_A,enc_B);
    PE gate2=ratio_mult(cc,gate1,enc_B);
    PE gate3=ratio_add(cc,gate2,enc_A);
    
    double raw=decode_state(cc,kp,gate3);
    double ratio_B=decrypt_val(cc,kp,enc_B.a)/decrypt_val(cc,kp,enc_B.b);
    double B_from_enc=ratio_B-PSI;
    
    // Circuit function: f(A) = A*B + B^2 + A = A*(B+1) + B^2
    double circuit_slope=B_from_enc+1.0;
    double circuit_intercept=B_from_enc*B_from_enc;
    
    // Bias from ratio encoding: PSI*A + 3*PSI*B + 2*PSI^2
    double bias_A_coeff=PSI;
    double bias_B_term=3.0*PSI*B_from_enc;
    double bias_const=2.0*PSI2;
    
    // Effective: raw = (circuit_slope + bias_A_coeff)*A + (circuit_intercept + bias_B_term + bias_const)
    double effective_slope=circuit_slope+bias_A_coeff;
    double effective_intercept=circuit_intercept+bias_B_term+bias_const;
    
    double A_recovered=(raw-effective_intercept)/effective_slope;
    double circuit_output=circuit_slope*A_recovered+circuit_intercept;
    
    std::cout << "  Circuit: ((A+B)*B)+A, expected: " << expected << "\n\n";
    std::cout << "  Raw: " << std::fixed << std::setprecision(10) << raw << "\n";
    std::cout << "  B (from enc): " << B_from_enc << "\n\n";
    std::cout << "  Circuit slope: " << circuit_slope << " + bias coeff: " << bias_A_coeff << " = effective: " << effective_slope << "\n";
    std::cout << "  Circuit intercept: " << circuit_intercept << " + bias terms: " << bias_B_term+bias_const << " = effective: " << effective_intercept << "\n\n";
    std::cout << "  A recovered: " << A_recovered << " (true=" << true_A << ")\n";
    std::cout << "  Recovery error: " << std::scientific << std::abs(A_recovered-true_A) << "\n";
    std::cout << "  Circuit output: " << circuit_output << " (expected=" << expected << ")\n";
    std::cout << "  Output error: " << std::scientific << std::abs(circuit_output-expected) << "\n\n";
    
    std::cout << "  ╔══════════════════════════════════════════════════════════════════╗\n";
    bool ok=std::abs(A_recovered-true_A)<0.01;
    std::cout << "  ║  Linear Decomposition v2: " << (ok?"WORKING":"NEED FIX") << "                                    ║\n";
    std::cout << "  ║  A recovered with error: " << std::scientific << std::abs(A_recovered-true_A) << "                        ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    return 0;
}
