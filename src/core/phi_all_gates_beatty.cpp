// ============================================
// φ-ALL GATES BEATTY — LAHAT NG GATES MAY φ-STRUCTURE?
//
// Check natin kung ang bawat gate ay may φ-natural
// na structure — hindi lang XOR!
//
// NAND: Beatty partition?
// AND: Beatty partition?
// OR: Beatty partition?
// NOT: Beatty partition?
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

class PhiAllGatesBeatty {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keyPair;
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double SQRT5 = sqrt(5.0);
    
public:
    PhiAllGatesBeatty() {
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
        
        cout << fixed << setprecision(10);
        cout << "========================================\n";
        cout << "  φ-ALL GATES BEATTY\n";
        cout << "  Lahat ba φ-natural?\n";
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
    
    Ciphertext<DCRTPoly> sub(const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalSub(a, b);
    }
    
    // BEATTY CHECK: k ∈ Beatty(φ)?
    bool is_beatty_phi(int k) {
        if (k <= 0) return true;  // Zero at negative → Beatty(φ)
        double floor_k_phi = floor(k / PHI);
        double floor_k_plus_1_phi = floor((k + 1) / PHI);
        return floor_k_plus_1_phi > floor_k_phi;
    }
    
    // BEATTY-BASED DECODE
    int decode_beatty(double val) {
        int int_k = (int)round(abs(val));
        return is_beatty_phi(int_k) ? 1 : 0;
    }
    
    void run() {
        cout << "  BEATTY SEQUENCES:\n";
        cout << "  Beatty(φ) = {";
        for (int i = 1; i <= 8; i++) cout << (int)floor(i * PHI) << (i < 8 ? ", " : "");
        cout << "}\n";
        cout << "  Beatty(φ²) = {";
        for (int i = 1; i <= 8; i++) cout << (int)floor(i * PHI * PHI) << (i < 8 ? ", " : "");
        cout << "}\n\n";
        
        cout << "  LAHAT NG GATES — RAW VALUES AT BEATTY:\n\n";
        cout << "  A B | NAND_Log | XOR_Norm | AND_Log | OR_Log | NOT_Log\n";
        cout << "  ----|----------|----------|---------|--------|--------\n";
        
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_dual(A);
                auto ct_b = encrypt_dual(B);
                
                // NAND: negate(add) sa log
                auto nand_ct = negate(add(ct_a, ct_b));
                double nand_raw = decrypt_dual(nand_ct)[1].real();
                
                // XOR: add sa normal
                auto xor_ct = add(ct_a, ct_b);
                double xor_raw = decrypt_dual(xor_ct)[0].real();
                
                // AND: NOT(NAND)
                auto and_nand = negate(add(ct_a, ct_b));
                auto and_ct = negate(and_nand);
                double and_raw = decrypt_dual(and_ct)[1].real();
                
                // OR: NAND(NOT, NOT)
                auto not_a = negate(ct_a);
                auto not_b = negate(ct_b);
                auto or_nand = negate(add(not_a, not_b));
                double or_raw = decrypt_dual(or_nand)[1].real();
                
                // NOT: NAND(a,a)
                auto not_ct = negate(add(ct_a, ct_a));
                double not_raw = decrypt_dual(not_ct)[1].real();
                
                cout << "  " << A << " " << B << " | "
                     << setw(8) << nand_raw << " | "
                     << setw(8) << xor_raw << " | "
                     << setw(7) << and_raw << " | "
                     << setw(6) << or_raw << " | "
                     << setw(7) << not_raw << "\n";
            }
        }
        
        cout << "\n  BEATTY CLASSIFICATION:\n\n";
        cout << "  A B | NAND | XOR | AND | OR | NOT\n";
        cout << "  ----|------|-----|-----|----|----\n";
        
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_dual(A);
                auto ct_b = encrypt_dual(B);
                
                auto nand_ct = negate(add(ct_a, ct_b));
                auto xor_ct = add(ct_a, ct_b);
                auto and_nand = negate(add(ct_a, ct_b));
                auto and_ct = negate(and_nand);
                auto not_a = negate(ct_a);
                auto not_b = negate(ct_b);
                auto or_nand = negate(add(not_a, not_b));
                auto not_ct = negate(add(ct_a, ct_a));
                
                int nand_val = decode_beatty(decrypt_dual(nand_ct)[1].real());
                int xor_val = decode_beatty(decrypt_dual(xor_ct)[0].real());
                int and_val = decode_beatty(decrypt_dual(and_ct)[1].real());
                int or_val = decode_beatty(decrypt_dual(or_nand)[1].real());
                int not_val = decode_beatty(decrypt_dual(not_ct)[1].real());
                
                int exp_nand = !(A && B), exp_xor = A != B;
                int exp_and = A && B, exp_or = A || B, exp_not = !A;
                
                cout << "  " << A << " " << B << " | "
                     << setw(4) << nand_val << "(" << exp_nand << ") | "
                     << setw(3) << xor_val << "(" << exp_xor << ") | "
                     << setw(3) << and_val << "(" << exp_and << ") | "
                     << setw(2) << or_val << "(" << exp_or << ") | "
                     << setw(3) << not_val << "(" << exp_not << ")\n";
            }
        }
        
        cout << "\n  EMERGENT OBSERVATIONS:\n";
        cout << "  - XOR: Perfect Beatty partition\n";
        cout << "  - Iba pang gates: May φ-structure din?\n";
        cout << "  - Kailangan ng per-gate φ-analysis\n\n";
    }
};

int main() {
    PhiAllGatesBeatty core;
    core.run();
    return 0;
}
