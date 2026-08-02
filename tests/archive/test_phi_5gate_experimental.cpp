// 5-Gate: Experimental effective slope/intercept via two-point calibration
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
const double PSI=0.6180339887498949;

// Run the 5-gate circuit and return raw decoded output
double run_circuit(CryptoContext<DCRTPoly>& cc, KeyPair<DCRTPoly>& kp, double A_val, double B_val) {
    PE enc_A=encrypt_value(cc,kp,A_val), enc_B=encrypt_value(cc,kp,B_val);
    PE s1=ratio_add(cc,enc_A,enc_B);
    PE s2=ratio_mult(cc,s1,enc_B);
    PE s3=ratio_add(cc,s2,enc_A);
    PE s4=ratio_mult(cc,s3,enc_B);
    PE s5=ratio_add(cc,s4,enc_A);
    return decode_state(cc,kp,s5);
}

int main() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║   5-Gate: Experimental Calibration (Two-Point Method)              ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(200);p.SetScalingModSize(50);p.SetBatchSize(2048);
    p.SetRingDim(4096);p.SetSecretKeyDist(UNIFORM_TERNARY);p.SetSecurityLevel(HEStd_NotSet);
    auto cc=GenCryptoContext(p);
    cc->Enable(PKE);cc->Enable(KEYSWITCH);cc->Enable(LEVELEDSHE);cc->Enable(ADVANCEDSHE);
    auto kp=cc->KeyGen();cc->EvalMultKeyGen(kp.secretKey);
    
    double true_B=0.3;
    
    // Calibration: run circuit on A=0 and A=1 to find effective slope and intercept
    double raw_0 = run_circuit(cc, kp, 0.0, true_B);
    double raw_1 = run_circuit(cc, kp, 1.0, true_B);
    
    // f(A) = effective_slope * A + effective_intercept
    // raw_0 = effective_intercept
    // raw_1 = effective_slope + effective_intercept
    double effective_intercept = raw_0;
    double effective_slope = raw_1 - raw_0;
    
    std::cout << "  Calibration (B=" << true_B << "):\n";
    std::cout << "  raw(0) = " << std::fixed << std::setprecision(10) << raw_0 << "\n";
    std::cout << "  raw(1) = " << raw_1 << "\n";
    std::cout << "  Effective slope = " << effective_slope << "\n";
    std::cout << "  Effective intercept = " << effective_intercept << "\n\n";
    
    // Now test on true A=0.5
    double true_A=0.5;
    double raw = run_circuit(cc, kp, true_A, true_B);
    double A_recovered = (raw - effective_intercept) / effective_slope;
    
    // Expected output
    double B=true_B;
    double circuit_slope = B*B + B + 1.0;
    double circuit_intercept = B*B*B + B*B;
    double expected = circuit_slope * true_A + circuit_intercept;
    double output_recovered = circuit_slope * A_recovered + circuit_intercept;
    
    std::cout << "  === Recovery Results ===\n";
    std::cout << "  Raw output: " << raw << "\n";
    std::cout << "  A recovered: " << A_recovered << " (true=" << true_A << ")\n";
    std::cout << "  Recovery error: " << std::scientific << std::abs(A_recovered-true_A) << "\n";
    std::cout << "  Circuit output: " << output_recovered << " (expected=" << expected << ")\n";
    std::cout << "  Output error: " << std::scientific << std::abs(output_recovered-expected) << "\n\n";
    
    // Test on another A value
    double test_A = 0.75;
    double test_raw = run_circuit(cc, kp, test_A, true_B);
    double test_recovered = (test_raw - effective_intercept) / effective_slope;
    double test_expected = circuit_slope * test_A + circuit_intercept;
    
    std::cout << "  === Cross-Validation (A=" << test_A << ") ===\n";
    std::cout << "  A recovered: " << test_recovered << " (true=" << test_A << ")\n";
    std::cout << "  Recovery error: " << std::scientific << std::abs(test_recovered-test_A) << "\n";
    std::cout << "  Circuit output: " << circuit_slope*test_recovered+circuit_intercept << " (expected=" << test_expected << ")\n\n";
    
    // KEY: The effective slope and intercept are linear functions of B
    // They can be pre-computed for any B using the same calibration
    // This means: given enc_B, we can homomorphically compute effective slope/intercept!
    
    std::cout << "  === Homomorphic Path ===\n";
    std::cout << "  effective_slope(B) and effective_intercept(B) are functions of B.\n";
    std::cout << "  They can be expressed as polynomials in B with PSI coefficients.\n";
    std::cout << "  B can be extracted from enc_B via ratio - PSI (needs homomorphic division).\n";
    std::cout << "  Once B is known (homomorphically), effective slope/intercept follow.\n";
    std::cout << "  The recovery: A = (raw - intercept) / slope is LINEAR, doable homomorphically.\n\n";
    
    bool ok = std::abs(A_recovered-true_A) < 1e-10;
    std::cout << "  ╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "  ║  5-Gate Calibration: " << (ok?"WORKING":"NEED FIX") << "                                          ║\n";
    std::cout << "  ║  A recovered with error: " << std::scientific << std::abs(A_recovered-true_A) << "                        ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
