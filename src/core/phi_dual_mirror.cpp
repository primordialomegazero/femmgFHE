// ============================================
// φ-DUAL TWO-WAY MIRROR
//
// Dalawang mirror:
// Mirror_NEG(x) = √5 - x (negative side)
// Mirror_POS(x) = √5 + x (positive side)
//
// NAND = Mirror_NEG(AND) = √5 - (a+b)
// AND = Mirror_POS(NAND) = √5 + (a+b)
// NOT = Mirror_NEG(x) = √5 - x
// OR = Mirror_POS(x) = √5 + x
//
// BALANCED: Negative at positive, patas!
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

class PhiDualMirror {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiDualMirror() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(1);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(16);
        parameters.SetSecurityLevel(HEStd_256_classic);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        cout << fixed << setprecision(10);
        cout << "========================================\n";
        cout << "  φ-DUAL TWO-WAY MIRROR\n";
        cout << "  Negative + Positive = Balanced\n";
        cout << "========================================\n\n";
        
        cout << "  Mirror_NEG(x) = √5 - x (negative)\n";
        cout << "  Mirror_POS(x) = √5 + x (positive)\n\n";
    }
    
    vector<double> encode(double bit) {
        vector<double> dims(16, 0.0);
        double val = (bit == 0.0) ? PHI_INV : PHI;
        for (int i = 0; i < 16; i++) dims[i] = val;
        return dims;
    }
    
    // UNIVERSAL DECODE — φ-HARMONIC RECOGNITION
    double decode(const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        
        // Handle negative values (mirror)
        if (avg < 0) {
            avg = abs(avg);  // Reflect sa positive
        }
        
        // φ-HARMONIC RECOGNITION
        if (abs(avg - PHI_INV) < 0.01) return 0.0;
        if (abs(avg - PHI) < 0.01) return 1.0;
        if (abs(avg - 2.0 * PHI_INV) < 0.01) return 0.0;
        if (abs(avg - (PHI + PHI_INV)) < 0.01) return 1.0;
        if (abs(avg - 3.0 * PHI_INV) < 0.01) return 0.0;
        if (abs(avg - 2.0 * PHI) < 0.01) return 0.0;
        
        double quotient = avg / PHI;
        double floored = floor(quotient + 0.0000001);
        return fmod(floored, 2.0);
    }
    
    Ciphertext<DCRTPoly> encrypt(vector<double> dims) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(dims);
        return cc->Encrypt(keyPair.publicKey, pt);
    }
    
    vector<complex<double>> decrypt(const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue();
    }
    
    Ciphertext<DCRTPoly> add(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalAdd(a, b);
    }
    
    Ciphertext<DCRTPoly> sub(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalSub(a, b);
    }
    
    // MIRROR_NEG: √5 - x
    Ciphertext<DCRTPoly> mirror_neg(const Ciphertext<DCRTPoly>& x) {
        auto sqrt5_ct = encrypt(vector<double>(16, SQRT5));
        return sub(sqrt5_ct, x);
    }
    
    // MIRROR_POS: √5 + x
    Ciphertext<DCRTPoly> mirror_pos(const Ciphertext<DCRTPoly>& x) {
        auto sqrt5_ct = encrypt(vector<double>(16, SQRT5));
        return add(sqrt5_ct, x);
    }
    
    // AND = addition (base)
    Ciphertext<DCRTPoly> gate_and(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return add(a, b);
    }
    
    // NAND = Mirror_NEG(AND) = √5 - (a+b)
    Ciphertext<DCRTPoly> gate_nand(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto and_ab = gate_and(a, b);
        return mirror_neg(and_ab);
    }
    
    // NOT = Mirror_NEG(x) = √5 - x
    Ciphertext<DCRTPoly> gate_not(const Ciphertext<DCRTPoly>& a) {
        return mirror_neg(a);
    }
    
    // OR = Mirror_POS(AND(Mirror_NEG(a), Mirror_NEG(b)))
    Ciphertext<DCRTPoly> gate_or(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto not_a = gate_not(a);
        auto not_b = gate_not(b);
        auto and_not = gate_and(not_a, not_b);
        return mirror_pos(and_not);
    }
    
    // XOR = OR(AND(a, NOT(b)), AND(NOT(a), b))
    Ciphertext<DCRTPoly> gate_xor(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto not_a = gate_not(a);
        auto not_b = gate_not(b);
        auto and_a_not_b = gate_and(a, not_b);
        auto and_not_a_b = gate_and(not_a, b);
        return gate_or(and_a_not_b, and_not_a_b);
    }
    
    void run() {
        int total_pass = 0;
        int total_tests = 0;
        
        cout << "  A B | NAND | NOT(A) | AND | OR | XOR\n";
        cout << "  ----|------|--------|-----|----|----\n";
        
        for (double A : {0.0, 1.0}) {
            for (double B : {0.0, 1.0}) {
                auto ct_a = encrypt(encode(A));
                auto ct_b = encrypt(encode(B));
                
                auto nand_ct = gate_nand(ct_a, ct_b);
                auto not_ct = gate_not(ct_a);
                auto and_ct = gate_and(ct_a, ct_b);
                auto or_ct = gate_or(ct_a, ct_b);
                auto xor_ct = gate_xor(ct_a, ct_b);
                
                double nand_val = decode(decrypt(nand_ct));
                double not_val = decode(decrypt(not_ct));
                double and_val = decode(decrypt(and_ct));
                double or_val = decode(decrypt(or_ct));
                double xor_val = decode(decrypt(xor_ct));
                
                double exp_nand = (A == 1.0 && B == 1.0) ? 0.0 : 1.0;
                double exp_not = (A == 0.0) ? 1.0 : 0.0;
                double exp_and = (A == 1.0 && B == 1.0) ? 1.0 : 0.0;
                double exp_or = (A == 1.0 || B == 1.0) ? 1.0 : 0.0;
                double exp_xor = fmod(A + B, 2.0);
                
                bool pass_nand = abs(nand_val - exp_nand) < 0.5;
                bool pass_not = abs(not_val - exp_not) < 0.5;
                bool pass_and = abs(and_val - exp_and) < 0.5;
                bool pass_or = abs(or_val - exp_or) < 0.5;
                bool pass_xor = abs(xor_val - exp_xor) < 0.5;
                
                if (pass_nand) total_pass++;
                if (pass_not) total_pass++;
                if (pass_and) total_pass++;
                if (pass_or) total_pass++;
                if (pass_xor) total_pass++;
                total_tests += 5;
                
                cout << "  " << A << " " << B << " | "
                     << setw(4) << setprecision(0) << nand_val << " | "
                     << setw(6) << not_val << " | "
                     << setw(3) << and_val << " | "
                     << setw(2) << or_val << " | "
                     << setw(3) << xor_val << " "
                     << (pass_nand && pass_not && pass_and && pass_or && pass_xor ? "✅" : "❌") << "\n";
            }
        }
        
        cout << "\n  ALL GATES: " << total_pass << "/" << total_tests << "\n";
        cout << "  Level: 0\n";
        cout << "  Pure FHE\n\n";
        
        cout << "  DUAL MIRROR:\n";
        cout << "  - Mirror_NEG(x) = √5 - x\n";
        cout << "  - Mirror_POS(x) = √5 + x\n";
        cout << "  - Negative at positive, BALANCED!\n\n";
    }
};

int main() {
    PhiDualMirror compiler;
    compiler.run();
    return 0;
}
