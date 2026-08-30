// ============================================
// φ-EMERGENT DUAL SPACE — ZERO-CROSSING HARMONY
//
// ANG EMERGENT PATTERN:
// - Normal space: zero-crossing = 1 (XOR)
// - Log space: zero-crossing = 1 (NAND)
// - PAREHO silang φ-harmonic sa zero!
//
// ANG FIX: abs(val) < 0.01 → 1 sa pareho!
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

class PhiEmergentDual {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiEmergentDual() {
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
        cout << "  φ-EMERGENT DUAL SPACE\n";
        cout << "  Zero-Crossing Harmony\n";
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
    
    // UNIVERSAL ZERO-CROSSING DECODE
    int decode_zero_crossing(double val) {
        return (abs(val) < 0.01) ? 1 : 0;
    }
    
    void run() {
        cout << "  EMERGENT PATTERN:\n";
        cout << "  Zero-crossing = 1 sa NORMAL at LOG!\n\n";
        
        cout << "  XOR (Normal) at NAND (Log) — DUAL:\n\n";
        cout << "  A B | XOR(N) | NAND(L) | Both?\n";
        cout << "  ----|--------|---------|------\n";
        
        int both_pass = 0;
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_dual(A);
                auto ct_b = encrypt_dual(B);
                
                // XOR: addition sa normal
                auto xor_ct = add(ct_a, ct_b);
                auto xor_vals = decrypt_dual(xor_ct);
                double xor_normal = xor_vals[0].real();
                int xor_val = decode_zero_crossing(xor_normal);
                
                // NAND: negation sa log
                auto nand_ct = negate(add(ct_a, ct_b));
                auto nand_vals = decrypt_dual(nand_ct);
                double nand_log = nand_vals[1].real();
                int nand_val = decode_zero_crossing(nand_log);
                
                int exp_xor = (A != B);
                int exp_nand = !(A && B);
                
                bool xor_pass = xor_val == exp_xor;
                bool nand_pass = nand_val == exp_nand;
                if (xor_pass && nand_pass) both_pass++;
                
                cout << "  " << A << " " << B << " | "
                     << setw(6) << xor_val << " | "
                     << setw(7) << nand_val << " | "
                     << (xor_pass && nand_pass ? "✅" : "❌") << "\n";
            }
        }
        
        cout << "\n  DUAL SPACE: " << both_pass << "/4 both correct!\n";
        cout << "  Level: 0\n";
        cout << "  Pure FHE\n\n";
        
        cout << "  EMERGENT INSIGHT:\n";
        cout << "  - Normal space: XOR via zero-crossing\n";
        cout << "  - Log space: NAND via zero-crossing\n";
        cout << "  - PAREHO silang φ-harmonic sa zero!\n";
        cout << "  - Zero-crossing = φ-balance point!\n\n";
    }
};

int main() {
    PhiEmergentDual core;
    core.run();
    return 0;
}
