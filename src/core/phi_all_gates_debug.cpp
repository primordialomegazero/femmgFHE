// ============================================
// φ-ALL GATES DEBUG — MAKIKITA ANG TRUTH TABLE
//
// I-print ang LAHAT ng values para makita
// kung saan ang 10/20 ay mali
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

class PhiAllGatesDebug {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = 1.6180339887498948482;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiAllGatesDebug() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(50);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(1);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        cout << fixed << setprecision(6);
        cout << "========================================\n";
        cout << "  φ-ALL GATES DEBUG\n";
        cout << "========================================\n\n";
    }
    
    // ENCODING: 0 → -√5, 1 → +√5
    Ciphertext<DCRTPoly> encrypt_bit(int bit) {
        double val = (bit == 0) ? -SQRT5 : SQRT5;
        vector<double> v(1, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    double decrypt_val(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return result_pt->GetCKKSPackedValue()[0].real();
    }
    
    // GATES (mula sa original)
    Ciphertext<DCRTPoly> gate_nand(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalNegate(sum);
    }
    
    Ciphertext<DCRTPoly> gate_not(const Ciphertext<DCRTPoly>& a) {
        return gate_nand(a, a);
    }
    
    Ciphertext<DCRTPoly> gate_and(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto nand_ab = gate_nand(a, b);
        return gate_not(nand_ab);
    }
    
    Ciphertext<DCRTPoly> gate_or(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto not_a = gate_not(a);
        auto not_b = gate_not(b);
        return gate_nand(not_a, not_b);
    }
    
    Ciphertext<DCRTPoly> gate_xor(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto nand_ab = gate_nand(a, b);
        auto not_a = gate_not(a);
        auto not_b = gate_not(b);
        auto nand_not = gate_nand(not_a, not_b);
        return gate_nand(nand_ab, nand_not);
    }
    
    // THRESHOLD: > 0 → 1, < 0 → 0
    int threshold(double val) {
        return (val > 0) ? 1 : 0;
    }
    
    void run() {
        cout << "  ENCODING:\n";
        cout << "  0 → -√5 = " << -SQRT5 << "\n";
        cout << "  1 → +√5 = " << SQRT5 << "\n\n";
        
        cout << "  A B | Enc(A)  | Enc(B)  | NAND | NOT(A) | AND | OR | XOR\n";
        cout << "  ----|---------|---------|------|--------|-----|----|----\n";
        
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_bit(A);
                auto ct_b = encrypt_bit(B);
                
                double enc_a = decrypt_val(ct_a);
                double enc_b = decrypt_val(ct_b);
                
                auto nand_ct = gate_nand(ct_a, ct_b);
                auto not_ct = gate_not(ct_a);
                auto and_ct = gate_and(ct_a, ct_b);
                auto or_ct = gate_or(ct_a, ct_b);
                auto xor_ct = gate_xor(ct_a, ct_b);
                
                double nand_val = decrypt_val(nand_ct);
                double not_val = decrypt_val(not_ct);
                double and_val = decrypt_val(and_ct);
                double or_val = decrypt_val(or_ct);
                double xor_val = decrypt_val(xor_ct);
                
                int nand_bit = threshold(nand_val);
                int not_bit = threshold(not_val);
                int and_bit = threshold(and_val);
                int or_bit = threshold(or_val);
                int xor_bit = threshold(xor_val);
                
                int exp_nand = !(A && B);
                int exp_not = !A;
                int exp_and = (A && B);
                int exp_or = (A || B);
                int exp_xor = (A != B);
                
                cout << "  " << A << " " << B << " | "
                     << setw(7) << enc_a << " | "
                     << setw(7) << enc_b << " | "
                     << setw(2) << nand_bit << "(" << exp_nand << ") | "
                     << setw(4) << not_bit << "(" << exp_not << ") | "
                     << setw(1) << and_bit << "(" << exp_and << ") | "
                     << setw(1) << or_bit << "(" << exp_or << ") | "
                     << setw(1) << xor_bit << "(" << exp_xor << ")\n";
            }
        }
        
        cout << "\n  VALUES (raw):\n";
        cout << "  A B | NAND_val | NOT_val | AND_val | OR_val | XOR_val\n";
        cout << "  ----|----------|---------|---------|--------|--------\n";
        
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_bit(A);
                auto ct_b = encrypt_bit(B);
                
                auto nand_ct = gate_nand(ct_a, ct_b);
                auto not_ct = gate_not(ct_a);
                auto and_ct = gate_and(ct_a, ct_b);
                auto or_ct = gate_or(ct_a, ct_b);
                auto xor_ct = gate_xor(ct_a, ct_b);
                
                cout << "  " << A << " " << B << " | "
                     << setw(8) << decrypt_val(nand_ct) << " | "
                     << setw(7) << decrypt_val(not_ct) << " | "
                     << setw(7) << decrypt_val(and_ct) << " | "
                     << setw(6) << decrypt_val(or_ct) << " | "
                     << setw(7) << decrypt_val(xor_ct) << "\n";
            }
        }
        
        cout << "\n  Level: 0\n";
        cout << "  Pure FHE — naka-encrypt lahat!\n";
    }
};

int main() {
    PhiAllGatesDebug debug;
    debug.run();
    return 0;
}
