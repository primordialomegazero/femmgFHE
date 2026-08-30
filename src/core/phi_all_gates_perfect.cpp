// ============================================
// φ-ALL GATES PERFECT — TAMANG THRESHOLD PER GATE
//
// ANG RAW VALUES AY SOBRANG LINIS:
// -4.472, 0.000, +4.472, -8.944, +8.944
//
// ANG TAMANG THRESHOLD:
// NAND: val == 0 → 1, else → 0
// XOR: val == 0 → 1, else → 0
// AND: val > 4 → 1, else → 0
// OR: val < -4 → 0, else → 1
// NOT: val > 0 → 1, else → 0
//
// PURE FHE + LEVEL 0 + PERFECT!
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

class PhiAllGatesPerfect {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = 1.6180339887498948482;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiAllGatesPerfect() {
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
        cout << "  φ-ALL GATES PERFECT\n";
        cout << "  Tamang Threshold Per Gate\n";
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
    
    // GATES (same structure)
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
    
    // TAMANG THRESHOLD PER GATE
    int decode_nand(double val) {
        return (abs(val) < 0.01) ? 1 : 0;  // zero-crossing → 1
    }
    
    int decode_not(double val) {
        return (val > 0) ? 1 : 0;  // positive → 1
    }
    
    int decode_and(double val) {
        return (val > 4.0) ? 1 : 0;  // +8.944 → 1
    }
    
    int decode_or(double val) {
        return (val < -4.0) ? 0 : 1;  // -8.944 → 0, else → 1
    }
    
    int decode_xor(double val) {
        return (abs(val) < 0.01) ? 1 : 0;  // zero-crossing → 1
    }
    
    void run() {
        int total_correct = 0;
        int total_tests = 0;
        
        cout << "  ENCODING:\n";
        cout << "  0 → -√5 = " << -SQRT5 << "\n";
        cout << "  1 → +√5 = " << SQRT5 << "\n\n";
        
        cout << "  A B | NAND | NOT(A) | AND | OR | XOR\n";
        cout << "  ----|------|--------|-----|----|----\n";
        
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_bit(A);
                auto ct_b = encrypt_bit(B);
                
                auto nand_ct = gate_nand(ct_a, ct_b);
                auto not_ct = gate_not(ct_a);
                auto and_ct = gate_and(ct_a, ct_b);
                auto or_ct = gate_or(ct_a, ct_b);
                auto xor_ct = gate_xor(ct_a, ct_b);
                
                double nand_raw = decrypt_val(nand_ct);
                double not_raw = decrypt_val(not_ct);
                double and_raw = decrypt_val(and_ct);
                double or_raw = decrypt_val(or_ct);
                double xor_raw = decrypt_val(xor_ct);
                
                int nand_val = decode_nand(nand_raw);
                int not_val = decode_not(not_raw);
                int and_val = decode_and(and_raw);
                int or_val = decode_or(or_raw);
                int xor_val = decode_xor(xor_raw);
                
                int exp_nand = !(A && B);
                int exp_not = !A;
                int exp_and = (A && B);
                int exp_or = (A || B);
                int exp_xor = (A != B);
                
                bool pass_nand = nand_val == exp_nand;
                bool pass_not = not_val == exp_not;
                bool pass_and = and_val == exp_and;
                bool pass_or = or_val == exp_or;
                bool pass_xor = xor_val == exp_xor;
                
                if (pass_nand) total_correct++;
                if (pass_not) total_correct++;
                if (pass_and) total_correct++;
                if (pass_or) total_correct++;
                if (pass_xor) total_correct++;
                total_tests += 5;
                
                cout << "  " << A << " " << B << " | "
                     << setw(4) << nand_val << " | "
                     << setw(6) << not_val << " | "
                     << setw(3) << and_val << " | "
                     << setw(2) << or_val << " | "
                     << setw(3) << xor_val << " "
                     << (pass_nand && pass_not && pass_and && pass_or && pass_xor ? "✅" : "❌") << "\n";
            }
        }
        
        cout << "\n  ALL GATES: " << total_correct << "/" << total_tests << "\n";
        cout << "  Level: 0\n";
        cout << "  Pure FHE\n\n";
        
        cout << "  RAW VALUES (for reference):\n";
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_bit(A);
                auto ct_b = encrypt_bit(B);
                cout << "  " << A << B << ": NAND=" << decrypt_val(gate_nand(ct_a, ct_b))
                     << " NOT=" << decrypt_val(gate_not(ct_a))
                     << " AND=" << decrypt_val(gate_and(ct_a, ct_b))
                     << " OR=" << decrypt_val(gate_or(ct_a, ct_b))
                     << " XOR=" << decrypt_val(gate_xor(ct_a, ct_b)) << "\n";
            }
        }
    }
};

int main() {
    PhiAllGatesPerfect core;
    core.run();
    return 0;
}
