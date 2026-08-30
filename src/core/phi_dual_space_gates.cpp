// ============================================
// φ-DUAL SPACE GATES — LOG + NORMAL SABAY
//
// DUAL SPACE:
// - Normal space: 0 → -√5, 1 → +√5
// - Log space: 0 → -1, 1 → +1
//
// ANG KEY: Bawat gate ay may OPTIMAL SPACE!
// - XOR: Normal space (addition)
// - AND: Log space (addition ng logs = multiplication)
// - OR: Normal space (addition + threshold)
// - NAND: Log space (NOT ng AND)
// - NOT: Normal space (negation)
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

class PhiDualSpaceGates {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiDualSpaceGates() {
        CCParams<CryptoContextCKKSRNS> parameters;
        parameters.SetMultiplicativeDepth(50);
        parameters.SetScalingModSize(50);
        parameters.SetBatchSize(2);  // [normal, log]
        
        cc = GenCryptoContext(parameters);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);
        
        keyPair = cc->KeyGen();
        cc->EvalMultKeyGen(keyPair.secretKey);
        
        cout << fixed << setprecision(6);
        cout << "========================================\n";
        cout << "  φ-DUAL SPACE GATES\n";
        cout << "  Log + Normal Sabay\n";
        cout << "========================================\n\n";
    }
    
    // DUAL ENCODING: [normal, log]
    Ciphertext<DCRTPoly> encrypt_dual(int bit) {
        vector<double> dual(2, 0.0);
        dual[0] = (bit == 0) ? -SQRT5 : SQRT5;  // Normal space
        dual[1] = (bit == 0) ? -1.0 : 1.0;       // Log space
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
    
    // GATES SA DUAL SPACE
    Ciphertext<DCRTPoly> gate_nand_dual(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        // NAND sa log space: -(log_a + log_b)
        auto sum = add(a, b);
        return negate(sum);
    }
    
    Ciphertext<DCRTPoly> gate_xor_dual(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        // XOR sa normal space: addition
        return add(a, b);
    }
    
    void run() {
        cout << "  DUAL ENCODING:\n";
        cout << "  Slot 0 (Normal): 0 → -√5, 1 → +√5\n";
        cout << "  Slot 1 (Log): 0 → -1, 1 → +1\n\n";
        
        cout << "  XOR SA NORMAL SPACE:\n";
        cout << "  A B | Normal Raw | XOR | Exp\n";
        cout << "  ----|------------|-----|----\n";
        
        int xor_pass = 0;
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_dual(A);
                auto ct_b = encrypt_dual(B);
                auto xor_ct = gate_xor_dual(ct_a, ct_b);
                auto vals = decrypt_dual(xor_ct);
                
                double normal_val = vals[0].real();
                int xor_val = (abs(normal_val) < 0.01) ? 1 : 0;
                int exp = (A != B);
                if (xor_val == exp) xor_pass++;
                
                cout << "  " << A << " " << B << " | "
                     << setw(10) << normal_val << " | "
                     << setw(3) << xor_val << " | "
                     << setw(3) << exp << "\n";
            }
        }
        cout << "\n  XOR: " << xor_pass << "/4\n\n";
        
        cout << "  NAND SA LOG SPACE:\n";
        cout << "  A B | Log Raw | NAND | Exp\n";
        cout << "  ----|---------|------|----\n";
        
        int nand_pass = 0;
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_dual(A);
                auto ct_b = encrypt_dual(B);
                auto nand_ct = gate_nand_dual(ct_a, ct_b);
                auto vals = decrypt_dual(nand_ct);
                
                double log_val = vals[1].real();
                int nand_val = (log_val >= 0) ? 1 : 0;
                int exp = !(A && B);
                if (nand_val == exp) nand_pass++;
                
                cout << "  " << A << " " << B << " | "
                     << setw(7) << log_val << " | "
                     << setw(4) << nand_val << " | "
                     << setw(4) << exp << "\n";
            }
        }
        cout << "\n  NAND: " << nand_pass << "/4\n\n";
        
        cout << "  DUAL SPACE ADVANTAGE:\n";
        cout << "  - Normal space: XOR (addition)\n";
        cout << "  - Log space: NAND (negation ng addition)\n";
        cout << "  - Pareho Level 0, Pure FHE!\n";
        cout << "  - Isang encryption, dalawang computation!\n\n";
    }
};

int main() {
    PhiDualSpaceGates core;
    core.run();
    return 0;
}
