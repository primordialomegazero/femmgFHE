// ============================================
// φ-ADDER DEBUG — ANO BA TALAGA ANG LOOB?
//
// I-print ang raw values ng Full Adder
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

class PhiAdderDebug {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = 1.6180339887498948482;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiAdderDebug() {
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
        cout << "  φ-ADDER DEBUG\n";
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
    
    void run() {
        cout << "  FULL ADDER RAW VALUES:\n\n";
        cout << "  A B Cin | XOR_AB | XOR(XOR_AB,Cin) | SUM_BIT | Exp\n";
        cout << "  --------|--------|-----------------|---------|----\n";
        
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                for (int Cin : {0, 1}) {
                    auto ct_a = encrypt_bit(A);
                    auto ct_b = encrypt_bit(B);
                    auto ct_cin = encrypt_bit(Cin);
                    
                    auto xor_ab = gate_xor(ct_a, ct_b);
                    double xor_ab_val = decrypt_val(xor_ab);
                    
                    auto xor_abc = gate_xor(xor_ab, ct_cin);
                    double xor_abc_val = decrypt_val(xor_abc);
                    
                    int sum_bit = (abs(xor_abc_val) < 0.01) ? 1 : 0;
                    int exp_sum = (A + B + Cin) % 2;
                    
                    cout << "  " << A << " " << B << " " << Cin << " | "
                         << setw(6) << xor_ab_val << " | "
                         << setw(15) << xor_abc_val << " | "
                         << setw(7) << sum_bit << " | "
                         << setw(3) << exp_sum << "\n";
                }
            }
        }
        
        cout << "\n  ANG ISSUE:\n";
        cout << "  XOR(0,1) = 0.000 → decode 1 ✅\n";
        cout << "  XOR(XOR(0,1), 0) = XOR(1, 0) = ?\n";
        cout << "  Kapag zero-crossing pa rin → 1 ✅\n";
        cout << "  Pero kapag hindi na → mali!\n";
    }
};

int main() {
    PhiAdderDebug debug;
    debug.run();
    return 0;
}
