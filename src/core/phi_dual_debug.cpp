// ============================================
// φ-DUAL DEBUG — ANO BA TALAGA ANG VALUES?
//
// Debug ang dual space values
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

class PhiDualDebug {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiDualDebug() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(50);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(2);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        cout << fixed << setprecision(10);
        cout << "========================================\n";
        cout << "  φ-DUAL DEBUG\n";
        cout << "========================================\n\n";
    }
    
    Ciphertext<DCRTPoly> encrypt_dual(int bit) {
        vector<double> dual(2, 0.0);
        dual[0] = (bit == 0) ? -SQRT5 : SQRT5;
        dual[1] = (bit == 0) ? -1.0 : 1.0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dual);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    vector<complex<double>> decrypt_dual(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(2);
        return result_pt->GetCKKSPackedValue();
    }
    
    Ciphertext<DCRTPoly> add(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    }
    
    Ciphertext<DCRTPoly> negate(const Ciphertext<DCRTPoly>& a) {
        return cc->EvalNegate(a);
    }
    
    void run() {
        cout << "  DEBUG: A=0, B=0\n";
        cout << "  =================\n\n";
        
        auto ct_a = encrypt_dual(0);
        auto ct_b = encrypt_dual(0);
        
        auto a_vals = decrypt_dual(ct_a);
        auto b_vals = decrypt_dual(ct_b);
        
        cout << "  ENCODING:\n";
        cout << "  A: Normal=" << a_vals[0].real() << " Log=" << a_vals[1].real() << "\n";
        cout << "  B: Normal=" << b_vals[0].real() << " Log=" << b_vals[1].real() << "\n\n";
        
        // ADD (para sa XOR sa normal)
        auto sum_ct = add(ct_a, ct_b);
        auto sum_vals = decrypt_dual(sum_ct);
        
        cout << "  AFTER ADD (XOR sa normal):\n";
        cout << "  Normal: " << sum_vals[0].real() << "\n";
        cout << "  Log: " << sum_vals[1].real() << "\n\n";
        
        // NEGATE (para sa NAND sa log)
        auto nand_ct = negate(sum_ct);
        auto nand_vals = decrypt_dual(nand_ct);
        
        cout << "  AFTER NEGATE (NAND sa log):\n";
        cout << "  Normal: " << nand_vals[0].real() << "\n";
        cout << "  Log: " << nand_vals[1].real() << "\n\n";
        
        cout << "  ANALYSIS:\n";
        cout << "  XOR(0,0) sa normal: " << sum_vals[0].real() << "\n";
        cout << "  - abs < 0.01? " << (abs(sum_vals[0].real()) < 0.01 ? "YES → 1" : "NO → 0") << "\n";
        cout << "  - Expected: 0\n\n";
        
        cout << "  NAND(0,0) sa log: " << nand_vals[1].real() << "\n";
        cout << "  - abs < 0.01? " << (abs(nand_vals[1].real()) < 0.01 ? "YES → 1" : "NO → 0") << "\n";
        cout << "  - Expected: 1\n\n";
        
        cout << "  ANG PROBLEMA:\n";
        cout << "  NAND(0,0) sa log = " << nand_vals[1].real() << "\n";
        cout << "  Hindi ito zero-crossing!\n";
        cout << "  Kasi: -(-1 + -1) = -(-2) = +2\n";
        cout << "  Ang +2 ay HINDI zero-crossing!\n\n";
        
        cout << "  ANG FIX:\n";
        cout << "  NAND sa log space:\n";
        cout << "  00: +2 → 1\n";
        cout << "  01: 0 → 1\n";
        cout << "  10: 0 → 1\n";
        cout << "  11: -2 → 0\n";
        cout << "  Kaya: val >= 0 → 1, val < 0 → 0\n\n";
        
        cout << "  COMPLETE TRUTH TABLE:\n";
        cout << "  A B | A.Log | B.Log | SUM.Log | NAND.Log | XOR.Norm | NAND | XOR\n";
        cout << "  ----|-------|-------|---------|----------|----------|------|----\n";
        
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto a = encrypt_dual(A);
                auto b = encrypt_dual(B);
                auto a_v = decrypt_dual(a);
                auto b_v = decrypt_dual(b);
                auto s = decrypt_dual(add(a, b));
                auto n = decrypt_dual(negate(add(a, b)));
                
                cout << "  " << A << " " << B << " | "
                     << setw(5) << a_v[1].real() << " | "
                     << setw(5) << b_v[1].real() << " | "
                     << setw(7) << s[1].real() << " | "
                     << setw(8) << n[1].real() << " | "
                     << setw(8) << s[0].real() << " | "
                     << setw(4) << (n[1].real() >= 0 ? 1 : 0) << " | "
                     << setw(3) << (abs(s[0].real()) < 0.01 ? 1 : 0) << "\n";
            }
        }
    }
};

int main() {
    PhiDualDebug debug;
    debug.run();
    return 0;
}
