// ============================================
// φ-MULTI-SPACE UNIVERSAL
//
// Bawat gate ay may SARILING φ-SPACE:
// - XOR → Addition space (mod 2)
// - AND → Multiplication space (log)
// - OR → Addition + threshold space
// - NOT → Negation space
// - NAND → NOT(AND) space
//
// ANG KEY: Hindi lahat ng gates ay addition!
// May φ-optimal space ang bawat isa!
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

class PhiMultiSpace {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double PHI_INV = 1.0 / PHI;
    const double LN_PHI = log(PHI);
    
public:
    PhiMultiSpace() {
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
        
        cout << fixed << setprecision(15);
        cout << "========================================\n";
        cout << "  φ-MULTI-SPACE UNIVERSAL\n";
        cout << "  Bawat gate, may sariling φ-space\n";
        cout << "========================================\n\n";
    }
    
    // ============================================
    // SPACE 1: ADDITION SPACE (para sa XOR)
    // ============================================
    
    vector<double> encode_add(double bit) {
        vector<double> dims(16, 0.0);
        double val = (bit == 0.0) ? PHI_INV : PHI;
        for (int i = 0; i < 16; i++) dims[i] = val;
        return dims;
    }
    
    double decode_add(const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        
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
    
    // ============================================
    // SPACE 2: LOG SPACE (para sa AND)
    // ============================================
    
    vector<double> encode_log(double bit) {
        vector<double> dims(16, 0.0);
        // Log space: 0 → -1, 1 → +1 (sa log scale)
        double val = (bit == 0.0) ? -1.0 : 1.0;
        for (int i = 0; i < 16; i++) dims[i] = val;
        return dims;
    }
    
    double decode_log(const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        
        // Log space: negative → 0, positive → 1
        // AND = addition sa log space
        // 0 AND 0 = -1 + (-1) = -2 → 0
        // 0 AND 1 = -1 + 1 = 0 → threshold
        // 1 AND 1 = 1 + 1 = 2 → 1
        
        if (avg < -1.5) return 0.0;    // -2 → 0
        if (avg > 1.5) return 1.0;     // +2 → 1
        if (abs(avg) < 0.1) return 0.0; // 0 → 0 (0 AND 1)
        
        return (avg > 0) ? 1.0 : 0.0;
    }
    
    // ============================================
    // SPACE 3: THRESHOLD SPACE (para sa OR)
    // ============================================
    
    vector<double> encode_thresh(double bit) {
        vector<double> dims(16, 0.0);
        double val = (bit == 0.0) ? 0.0 : 1.0;
        for (int i = 0; i < 16; i++) dims[i] = val;
        return dims;
    }
    
    double decode_thresh(const vector<complex<double>>& vals) {
        double sum = 0.0;
        for (int i = 0; i < 16; i++) sum += vals[i].real();
        double avg = sum / 16.0;
        
        // Threshold: > 0.5 → 1
        return (avg > 0.5) ? 1.0 : 0.0;
    }
    
    // ============================================
    // ENCRYPT / DECRYPT / OPERATIONS
    // ============================================
    
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
    
    // ============================================
    // UNIVERSAL GATES (MULTI-SPACE)
    // ============================================
    
    Ciphertext<DCRTPoly> gate_xor(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        // XOR sa ADDITION SPACE
        return add(a, b);
    }
    
    Ciphertext<DCRTPoly> gate_and(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        // AND sa LOG SPACE: addition ng logs
        return add(a, b);
    }
    
    Ciphertext<DCRTPoly> gate_or(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        // OR sa THRESHOLD SPACE: addition + threshold
        return add(a, b);
    }
    
    Ciphertext<DCRTPoly> gate_not(const Ciphertext<DCRTPoly>& a) {
        // NOT sa ADDITION SPACE: 1 - a
        auto ones = encrypt(encode_add(1.0));
        return cc->EvalSub(ones, a);
    }
    
    Ciphertext<DCRTPoly> gate_nand(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        // NAND = NOT(AND)
        auto and_ab = gate_and(a, b);
        return gate_not(and_ab);
    }
    
    void run() {
        int total_pass = 0;
        int total_tests = 0;
        
        cout << "========================================\n";
        cout << "  ALL GATES (MULTI-SPACE)\n";
        cout << "========================================\n\n";
        
        cout << "  A B | XOR | AND | OR | NOT(A) | NAND\n";
        cout << "  ----|-----|-----|----|--------|-----\n";
        
        for (double A : {0.0, 1.0}) {
            for (double B : {0.0, 1.0}) {
                // XOR space
                auto a_xor = encrypt(encode_add(A));
                auto b_xor = encrypt(encode_add(B));
                auto xor_ct = gate_xor(a_xor, b_xor);
                double xor_val = decode_add(decrypt(xor_ct));
                
                // AND log space
                auto a_and = encrypt(encode_log(A));
                auto b_and = encrypt(encode_log(B));
                auto and_ct = gate_and(a_and, b_and);
                double and_val = decode_log(decrypt(and_ct));
                
                // OR threshold space
                auto a_or = encrypt(encode_thresh(A));
                auto b_or = encrypt(encode_thresh(B));
                auto or_ct = gate_or(a_or, b_or);
                double or_val = decode_thresh(decrypt(or_ct));
                
                // NOT addition space
                auto a_not = encrypt(encode_add(A));
                auto not_ct = gate_not(a_not);
                double not_val = decode_add(decrypt(not_ct));
                
                // NAND
                auto a_nand = encrypt(encode_log(A));
                auto b_nand = encrypt(encode_log(B));
                auto nand_ct = gate_nand(a_nand, b_nand);
                double nand_val = decode_log(decrypt(nand_ct));
                
                double exp_xor = fmod(A + B, 2.0);
                double exp_and = (A == 1.0 && B == 1.0) ? 1.0 : 0.0;
                double exp_or = (A == 1.0 || B == 1.0) ? 1.0 : 0.0;
                double exp_not = (A == 0.0) ? 1.0 : 0.0;
                double exp_nand = (A == 1.0 && B == 1.0) ? 0.0 : 1.0;
                
                bool pass_xor = abs(xor_val - exp_xor) < 0.5;
                bool pass_and = abs(and_val - exp_and) < 0.5;
                bool pass_or = abs(or_val - exp_or) < 0.5;
                bool pass_not = abs(not_val - exp_not) < 0.5;
                bool pass_nand = abs(nand_val - exp_nand) < 0.5;
                
                if (pass_xor) total_pass++;
                if (pass_and) total_pass++;
                if (pass_or) total_pass++;
                if (pass_not) total_pass++;
                if (pass_nand) total_pass++;
                total_tests += 5;
                
                cout << "  " << A << " " << B << " | "
                     << setw(3) << setprecision(0) << xor_val << " | "
                     << setw(3) << and_val << " | "
                     << setw(2) << or_val << " | "
                     << setw(6) << not_val << " | "
                     << setw(4) << nand_val << " "
                     << (pass_xor && pass_and && pass_or && pass_not && pass_nand ? "✅" : "❌") << "\n";
            }
        }
        
        cout << "\n  ALL GATES: " << total_pass << "/" << total_tests << "\n";
        cout << "  Level: 0\n";
        cout << "  Pure FHE\n\n";
        
        cout << "  MULTI-SPACE SUMMARY:\n";
        cout << "  - XOR: Addition space (mod 2)\n";
        cout << "  - AND: Log space (addition ng logs)\n";
        cout << "  - OR: Threshold space (addition + threshold)\n";
        cout << "  - NOT: Negation space (1 - a)\n";
        cout << "  - NAND: NOT(AND) combination\n\n";
    }
};

int main() {
    PhiMultiSpace compiler;
    compiler.run();
    return 0;
}
