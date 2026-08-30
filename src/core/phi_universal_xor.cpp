// ============================================
// φ-UNIVERSAL XOR — TAMANG DECODE PARA SA LAHAT
//
// ANG UNIVERSAL XOR DECODE:
// toggles = |val| / √5
// XOR = toggles mod 2 (odd → 1, even → 0)
//
// 0.000 → 0 toggles → 0? HINDI! Dapat 1!
//
// ANG TAMANG UNIVERSAL:
// val/√5:
// 0 → 0 → XOR=0
// ±2.236/√5 = ±1 → XOR=1
// ±4.472/√5 = ±2 → XOR=0
// ±6.708/√5 = ±3 → XOR=1
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

class PhiUniversalXor {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiUniversalXor() {
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
        cout << "  φ-UNIVERSAL XOR\n";
        cout << "========================================\n\n";
    }
    
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
    
    // UNIVERSAL XOR DECODE
    int decode_xor_universal(double val) {
        double toggles = abs(val) / SQRT5;
        int int_toggles = (int)round(toggles);
        return int_toggles % 2;
    }
    
    void run() {
        cout << "  SIMPLE XOR (2 inputs):\n\n";
        cout << "  A B | Raw    | Toggles | XOR | Exp\n";
        cout << "  ----|--------|---------|-----|----\n";
        
        int xor_pass = 0;
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_bit(A);
                auto ct_b = encrypt_bit(B);
                auto xor_ct = gate_xor(ct_a, ct_b);
                double raw = decrypt_val(xor_ct);
                int xor_val = decode_xor_universal(raw);
                int exp = (A != B);
                if (xor_val == exp) xor_pass++;
                
                cout << "  " << A << " " << B << " | "
                     << setw(6) << raw << " | "
                     << setw(7) << abs(raw) / SQRT5 << " | "
                     << setw(3) << xor_val << " | "
                     << setw(3) << exp << "\n";
            }
        }
        cout << "\n  SIMPLE XOR: " << xor_pass << "/4\n\n";
        
        cout << "  FULL ADDER (3 inputs):\n\n";
        cout << "  A B Cin | Raw    | Toggles | SUM | Exp\n";
        cout << "  --------|--------|---------|-----|----\n";
        
        int adder_pass = 0;
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                for (int Cin : {0, 1}) {
                    auto ct_a = encrypt_bit(A);
                    auto ct_b = encrypt_bit(B);
                    auto ct_cin = encrypt_bit(Cin);
                    
                    auto xor_ab = gate_xor(ct_a, ct_b);
                    auto xor_abc = gate_xor(xor_ab, ct_cin);
                    double raw = decrypt_val(xor_abc);
                    int sum_val = decode_xor_universal(raw);
                    int exp = (A + B + Cin) % 2;
                    if (sum_val == exp) adder_pass++;
                    
                    cout << "  " << A << " " << B << " " << Cin << " | "
                         << setw(6) << raw << " | "
                         << setw(7) << abs(raw) / SQRT5 << " | "
                         << setw(3) << sum_val << " | "
                         << setw(3) << exp << "\n";
                }
            }
        }
        cout << "\n  FULL ADDER: " << adder_pass << "/8\n";
    }
};

int main() {
    PhiUniversalXor core;
    core.run();
    return 0;
}
