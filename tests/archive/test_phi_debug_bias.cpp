// Debug: Print actual FHE raw values for add-then-mul tests
#include <iostream>
#include <cmath>
#include "openfhe.h"
using namespace lbcrypto;

struct PE { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };

PE ratio_add(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    auto a1b2 = cc->EvalMult(x.a, y.b);
    auto a2b1 = cc->EvalMult(y.a, x.b);
    auto b1b2 = cc->EvalMult(x.b, y.b);
    return {cc->EvalAdd(a1b2, a2b1), b1b2};
}

PE ratio_mult(CryptoContext<DCRTPoly>& cc, const PE& x, const PE& y) {
    return {cc->EvalMult(x.a, y.a), cc->EvalMult(x.b, y.b)};
}

double get_val(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, 
               const Ciphertext<DCRTPoly>& c) {
    Plaintext pt;
    cc->Decrypt(kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double get_ratio(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, const PE& s) {
    double a = get_val(cc, kp, s.a);
    double b = get_val(cc, kp, s.b);
    return (std::abs(b) > 1e-10) ? a / b : a;
}

PE make_state(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double value) {
    const double PSI = 0.6180339887498949;
    double ratio = value + PSI;
    auto pta = cc->MakeCKKSPackedPlaintext(std::vector<double>{ratio});
    auto ptb = cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    return {cc->Encrypt(kp.publicKey, pta), cc->Encrypt(kp.publicKey, ptb)};
}

double decode_value(double ratio) {
    return ratio - 0.6180339887498949;
}

int main() {
    const double PSI = 0.6180339887498949;
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(50);
    p.SetScalingModSize(50);
    p.SetBatchSize(2048);
    p.SetRingDim(4096);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    
    PE A = make_state(cc, kp, 0.500);
    PE B = make_state(cc, kp, 0.300);
    PE C = make_state(cc, kp, 0.200);
    PE E = make_state(cc, kp, 1.000);
    
    std::cout << "=== ACTUAL FHE RAW VALUES ===\n\n";
    
    // N=1: (A+B)×C
    PE am1 = ratio_mult(cc, ratio_add(cc, A, B), C);
    double raw1 = decode_value(get_ratio(cc, kp, am1));
    std::cout << "N=1: raw=" << std::fixed << std::setprecision(10) << raw1 << "\n";
    std::cout << "  Expected raw (from plaintext math): 0.705573\n\n";
    
    // N=2: (A+B+E)×C
    PE sum_abe = ratio_add(cc, ratio_add(cc, A, B), E);
    double after_add2 = decode_value(get_ratio(cc, kp, sum_abe));
    PE am2 = ratio_mult(cc, sum_abe, C);
    double raw2 = decode_value(get_ratio(cc, kp, am2));
    std::cout << "N=2: after add=" << after_add2 << "\n";
    std::cout << "N=2: raw=" << std::fixed << std::setprecision(10) << raw2 << "\n";
    std::cout << "  Expected raw (from plaintext math): 1.620156\n\n";
    
    // N=4: (5×A)×C
    PE sum_5a = A;
    for (int i = 0; i < 4; i++) sum_5a = ratio_add(cc, sum_5a, A);
    double after_add4 = decode_value(get_ratio(cc, kp, sum_5a));
    PE am3 = ratio_mult(cc, sum_5a, C);
    double raw3 = decode_value(get_ratio(cc, kp, am3));
    std::cout << "N=4: after add=" << after_add4 << "\n";
    std::cout << "N=4: raw=" << std::fixed << std::setprecision(10) << raw3 << "\n";
    std::cout << "  Expected raw (from plaintext math): 3.449341\n\n";
    
    // Check: difference between FHE raw and plaintext expected
    std::cout << "=== DIFFERENCES ===\n";
    double expected_raw1 = 0.705573, expected_raw2 = 1.620156, expected_raw3 = 3.449341;
    std::cout << "N=1 diff: " << raw1 - expected_raw1 << "\n";
    std::cout << "N=2 diff: " << raw2 - expected_raw2 << "\n";
    std::cout << "N=4 diff: " << raw3 - expected_raw3 << "\n";
    
    return 0;
}
