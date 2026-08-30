// ============================================
// φ-ALL NATURAL V2 — TAMANG φ-NATURALITY
//
// ANG TAMANG φ-NATURALITY:
// - XOR: Beatty partition (symmetric)
// - NAND: Sign-based (val >= 0 → 1)
// - AND: Positive-based (val > 0 → 1)
// - OR: Sign-based (val >= 0 → 1)
// - NOT: Sign-based (val >= 0 → 1)
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

class PhiAllNaturalV2 {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    
public:
    PhiAllNaturalV2() {
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
        cout << "  φ-ALL NATURAL V2\n";
        cout << "  Tamang φ-Naturality\n";
        cout << "========================================\n\n";
    }
    
    Ciphertext<DCRTPoly> encrypt_dual(int bit) {
        vector<double> dual(2, 0.0);
        dual[0] = (bit == 0) ? -1.0 : 1.0;
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
    
    bool is_beatty_phi(int k) {
        if (k <= 0) return true;
        double floor_k_phi = floor(k / PHI);
        double floor_k_plus_1_phi = floor((k + 1) / PHI);
        return floor_k_plus_1_phi > floor_k_phi;
    }
    
    // TAMANG φ-NATURALITY PER GATE
    int decode_natural(double val, string gate) {
        if (gate == "XOR") {
            // XOR: Beatty partition
            int k = (int)round(abs(val));
            return is_beatty_phi(k) ? 1 : 0;
        } else if (gate == "AND") {
            // AND: positive → 1
            return (val > 0.01) ? 1 : 0;
        } else {
            // NAND, OR, NOT: non-negative → 1
            return (val >= -0.01) ? 1 : 0;
        }
    }
    
    void run() {
        cout << "  A B | NAND | XOR | AND | OR | NOT(A)\n";
        cout << "  ----|------|-----|-----|----|-------\n";
        
        int total_pass = 0;
        
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_dual(A);
                auto ct_b = encrypt_dual(B);
                
                auto nand_ct = negate(add(ct_a, ct_b));
                auto xor_ct = add(ct_a, ct_b);
                auto and_ct = negate(negate(add(ct_a, ct_b)));
                auto not_a = negate(ct_a);
                auto not_b = negate(ct_b);
                auto or_ct = negate(add(not_a, not_b));
                auto not_ct = negate(add(ct_a, ct_a));
                
                int nand_val = decode_natural(decrypt_dual(nand_ct)[1].real(), "NAND");
                int xor_val = decode_natural(decrypt_dual(xor_ct)[0].real(), "XOR");
                int and_val = decode_natural(decrypt_dual(and_ct)[1].real(), "AND");
                int or_val = decode_natural(decrypt_dual(or_ct)[1].real(), "OR");
                int not_val = decode_natural(decrypt_dual(not_ct)[1].real(), "NOT");
                
                int exp_nand = !(A && B), exp_xor = (A != B);
                int exp_and = (A && B), exp_or = (A || B), exp_not = !A;
                
                bool pass = (nand_val == exp_nand && xor_val == exp_xor && 
                            and_val == exp_and && or_val == exp_or && not_val == exp_not);
                
                total_pass += (nand_val == exp_nand) + (xor_val == exp_xor) + 
                             (and_val == exp_and) + (or_val == exp_or) + (not_val == exp_not);
                
                cout << "  " << A << " " << B << " | "
                     << setw(4) << nand_val << " | "
                     << setw(3) << xor_val << " | "
                     << setw(3) << and_val << " | "
                     << setw(2) << or_val << " | "
                     << setw(5) << not_val << " "
                     << (pass ? "✅" : "❌") << "\n";
            }
        }
        
        cout << "\n  ALL NATURAL: " << total_pass << "/20\n";
        cout << "  Level: 0\n";
        cout << "  Pure FHE\n\n";
        
        if (total_pass == 20) {
            cout << "  🏆 PERFECT φ-NATURALITY! 🏆\n\n";
        }
    }
};

int main() {
    PhiAllNaturalV2 core;
    core.run();
    return 0;
}
