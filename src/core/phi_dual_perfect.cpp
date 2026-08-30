// ============================================
// φ-DUAL PERFECT — 8/8 DUAL SPACE
//
// NAND (Log space): val >= 0 → 1
// XOR (Normal space): abs(val) < 0.01 → 1
//
// COMPLETE TRUTH TABLE:
// A B | NAND.Log | XOR.Norm | NAND | XOR
// ----|----------|----------|------|----
// 0 0 | +2.000   | -4.472   |  1   |  0
// 0 1 |  0.000   |  0.000   |  1   |  1
// 1 0 |  0.000   |  0.000   |  1   |  1
// 1 1 | -2.000   | +4.472   |  0   |  0
//
// Level 0, Pure FHE, Dual Space!
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

class PhiDualPerfect {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiDualPerfect() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(50);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(2);
        parameters.SetSecurityLevel(HEStd_256_classic);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        cout << fixed << setprecision(6);
        cout << "========================================\n";
        cout << "  φ-DUAL PERFECT\n";
        cout << "  8/8 Dual Space\n";
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
    
    int decode_nand_log(double val) { return (val >= -0.01) ? 1 : 0; }
    int decode_xor_normal(double val) { return (abs(val) < 0.01) ? 1 : 0; }
    
    void run() {
        cout << "  DUAL SPACE COMPLETE:\n\n";
        cout << "  A B | NAND(L) | XOR(N) | Both?\n";
        cout << "  ----|---------|--------|------\n";
        
        int total = 0;
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_dual(A);
                auto ct_b = encrypt_dual(B);
                
                auto sum_ct = add(ct_a, ct_b);
                auto nand_ct = negate(sum_ct);
                
                auto sum_vals = decrypt_dual(sum_ct);
                auto nand_vals = decrypt_dual(nand_ct);
                
                int nand_val = decode_nand_log(nand_vals[1].real());
                int xor_val = decode_xor_normal(sum_vals[0].real());
                
                int exp_nand = !(A && B);
                int exp_xor = (A != B);
                
                bool pass = (nand_val == exp_nand && xor_val == exp_xor);
                if (pass) total++;
                
                cout << "  " << A << " " << B << " | "
                     << setw(7) << nand_val << " | "
                     << setw(6) << xor_val << " | "
                     << (pass ? "✅" : "❌") << "\n";
            }
        }
        
        cout << "\n  DUAL SPACE: " << total << "/4 both correct!\n";
        cout << "  Level: 0\n";
        cout << "  Pure FHE\n\n";
        
        if (total == 4) {
            cout << "  🏆 DUAL PERFECT! 8/8! 🏆\n\n";
        }
    }
};

int main() {
    PhiDualPerfect core;
    core.run();
    return 0;
}
