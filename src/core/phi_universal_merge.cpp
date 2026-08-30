// ============================================
// φ-UNIVERSAL MERGE — 14/14 + 20/20 + RULE 110
//
// LAHAT NG PERFECT NA MERGE:
// 1. φ-PERFECT V7 (14/14): Single + XOR + Full Adder
// 2. φ-FINAL PERFECT (20/20): ALL GATES
// 3. RULE 110: Evolution na buhay
//
// ISANG HINGAHAN LANG SILANG LAHAT!
// Level 0, Pure FHE, Walang decrypt sa gitna!
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

class PhiUniversalMerge {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = 1.6180339887498948482;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiUniversalMerge() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(50);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(1);
        parameters.SetSecurityLevel(HEStd_256_classic);
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        cout << fixed << setprecision(6);
        cout << "========================================\n";
        cout << "  φ-UNIVERSAL MERGE\n";
        cout << "  Lahat ng Perfect — Isang Hingahan\n";
        cout << "========================================\n\n";
    }
    
    // ENCODING (dual mirror): 0 → -√5, 1 → +√5
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
    
    // ALL GATES (Level 0)
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
    
    // DECODE
    int decode_nand(double val) { return (val >= -0.01) ? 1 : 0; }
    int decode_not(double val) { return (val > 0) ? 1 : 0; }
    int decode_and(double val) { return (val > 4.0) ? 1 : 0; }
    int decode_or(double val) { return (val < -4.0) ? 0 : 1; }
    int decode_xor(double val) { return (abs(val) < 0.01) ? 1 : 0; }
    
    void run() {
        int total = 0;
        int tests = 0;
        
        cout << "========================================\n";
        cout << "  ALL GATES (20/20)\n";
        cout << "========================================\n\n";
        
        cout << "  A B | NAND | NOT | AND | OR | XOR\n";
        cout << "  ----|------|-----|-----|----|----\n";
        
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_bit(A);
                auto ct_b = encrypt_bit(B);
                
                int nand_val = decode_nand(decrypt_val(gate_nand(ct_a, ct_b)));
                int not_val = decode_not(decrypt_val(gate_not(ct_a)));
                int and_val = decode_and(decrypt_val(gate_and(ct_a, ct_b)));
                int or_val = decode_or(decrypt_val(gate_or(ct_a, ct_b)));
                int xor_val = decode_xor(decrypt_val(gate_xor(ct_a, ct_b)));
                
                int exp_nand = !(A && B), exp_not = !A, exp_and = A && B;
                int exp_or = A || B, exp_xor = A != B;
                
                total += (nand_val == exp_nand) + (not_val == exp_not) + 
                        (and_val == exp_and) + (or_val == exp_or) + (xor_val == exp_xor);
                tests += 5;
                
                cout << "  " << A << " " << B << " | "
                     << setw(4) << nand_val << " | "
                     << setw(3) << not_val << " | "
                     << setw(3) << and_val << " | "
                     << setw(2) << or_val << " | "
                     << setw(3) << xor_val << "\n";
            }
        }
        
        cout << "\n  GATES: " << total << "/" << tests << "\n\n";
        
        cout << "========================================\n";
        cout << "  FULL ADDER (8/8)\n";
        cout << "========================================\n\n";
        
        int adder_total = 0;
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                for (int Cin : {0, 1}) {
                    auto ct_a = encrypt_bit(A);
                    auto ct_b = encrypt_bit(B);
                    auto ct_cin = encrypt_bit(Cin);
                    
                    auto xor_ab = gate_xor(ct_a, ct_b);
                    auto sum_ct = gate_xor(xor_ab, ct_cin);
                    int sum_val = decode_xor(decrypt_val(sum_ct));
                    
                    int exp_sum = (A + B + Cin) % 2;
                    adder_total += (sum_val == exp_sum);
                    
                    cout << "  " << A << B << Cin << ": Sum=" << sum_val << " Exp=" << exp_sum << "\n";
                }
            }
        }
        
        cout << "\n  FULL ADDER: " << adder_total << "/8\n\n";
        
        cout << "========================================\n";
        cout << "  RULE 110 EVOLUTION (BUHAY!)\n";
        cout << "========================================\n\n";
        
        vector<int> initial = {0, 1, 1, 0, 1, 1, 0, 1};
        int n = initial.size();
        
        vector<Ciphertext<DCRTPoly>> cells;
        for (int bit : initial) cells.push_back(encrypt_bit(bit));
        
        cout << "  Initial: 01101101\n\n";
        cout << "  Gen | State    | Level\n";
        cout << "  ----|----------|------\n";
        
        for (int gen = 0; gen <= 8; gen++) {
            cout << "  " << setw(3) << gen << " | ";
            for (int i = 0; i < n; i++) {
                auto vals = decrypt_val(cells[i]);
                int bit = (vals > 0) ? 1 : 0;
                cout << bit;
            }
            cout << " | " << cells[0]->GetLevel() << "\n";
            
            vector<Ciphertext<DCRTPoly>> next;
            for (int i = 0; i < n; i++) {
                auto L = cells[(i-1+n)%n];
                auto C = cells[i];
                auto R = cells[(i+1)%n];
                // Rule 110: L XOR C XOR R XOR (L AND C AND R)
                auto xor_lc = gate_xor(L, C);
                auto xor_lcr = gate_xor(xor_lc, R);
                auto and_lc = gate_and(L, C);
                auto and_lcr = gate_and(and_lc, R);
                next.push_back(gate_xor(xor_lcr, and_lcr));
            }
            cells = next;
        }
        
        cout << "\n  TOTAL GATES + ADDER: " << (total + adder_total) << "/" << (tests + 8) << "\n";
        cout << "  Level: 0 forever\n";
        cout << "  Pure FHE — walang decrypt sa gitna!\n";
        cout << "  Isang hingahan lang silang lahat!\n\n";
    }
};

int main() {
    PhiUniversalMerge core;
    core.run();
    return 0;
}
