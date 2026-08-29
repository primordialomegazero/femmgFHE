// ============================================
// φ-NAND DEBUG — RAW VALUES NG φ² ENCODING
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(50);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    
    const double PHI = 1.6180339887498948482;
    
    auto encrypt_val = [&](double log_val) {
        vector<double> v(1, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };
    
    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return result_pt->GetCKKSPackedValue()[0].real();
    };
    
    cout << "========================================\n";
    cout << "  NAND DEBUG — φ² ENCODING RAW VALUES\n";
    cout << "========================================\n\n";
    
    cout << "  Encoding:\n";
    cout << "  0 → log = -2.0\n";
    cout << "  1 → log = +2.0\n\n";
    
    cout << "  NAND = -(log_a + log_b)\n\n";
    
    cout << "  A B | log_a | log_b | sum | NAND_neg | φ^NAND | Expected\n";
    cout << "  ----|-------|-------|-----|----------|--------|---------\n";
    
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double log_a = (A == 0) ? -2.0 : 2.0;
            double log_b = (B == 0) ? -2.0 : 2.0;
            
            auto ct_a = encrypt_val(log_a);
            auto ct_b = encrypt_val(log_b);
            
            auto ct_sum = cc->EvalAdd(ct_a, ct_b);
            double sum = decrypt_val(ct_sum);
            
            auto ct_nand = cc->EvalNegate(ct_sum);
            double nand_neg = decrypt_val(ct_nand);
            
            double phi_nand = pow(PHI, nand_neg);
            int threshold = (phi_nand > 1.0) ? 1 : 0;
            int expected = !(A && B);
            
            cout << "  " << A << " " << B << " | "
                 << setw(5) << fixed << setprecision(1) << log_a << " | "
                 << setw(5) << log_b << " | "
                 << setw(3) << sum << " | "
                 << setw(8) << nand_neg << " | "
                 << setw(6) << setprecision(2) << phi_nand << " | "
                 << setw(5) << expected << "\n";
        }
    }
    
    cout << "\n  PROBLEM:\n";
    cout << "  NAND(0,1): log_a=-2, log_b=+2, sum=0, neg=0\n";
    cout << "  φ^0 = 1.0 → threshold (>1.0) → 0 ❌\n";
    cout << "  DAPAT: threshold ay ≥1.0 (kasama ang 1.0)\n";
    cout << "  O: gumamit ng φ³ encoding para mas malaki ang gap\n\n";
    
    cout << "  FIX 1: threshold ay >= 1.0\n";
    cout << "  (φ^0 = 1.0 → 1)\n\n";
    
    cout << "  FIX 2: φ³ encoding (mas malaking gap)\n";
    cout << "  0 → -3.0, 1 → +3.0\n";
    cout << "  NAND(0,1) = -( -3 + 3 ) = 0 → φ^0 = 1.0 → 1 ✅\n";
    cout << "  NAND(1,1) = -( 3 + 3 ) = -6 → φ^-6 = 0.056 → 0 ✅\n\n";
    
    cout << "  FIX 3: φ⁴ encoding (pinakaligtas)\n";
    cout << "  0 → -4.0, 1 → +4.0\n";
    cout << "  NAND(0,1) = 0 → φ^0 = 1.0 → threshold >= 1 → 1 ✅\n";
    cout << "  NAND(1,1) = -8 → φ^-8 = 0.021 → 0 ✅\n";
    cout << "  NAND(0,0) = +8 → φ^8 = 47 → 1 ✅\n\n";
    
    cout << "  ANG TAMANG THRESHOLD:\n";
    cout << "  value >= 1.0 → 1 (hindi > 1.0)\n";
    cout << "  value < 1.0 → 0\n";
    cout << "  Ito ay INCLUSIVE threshold — φ^0 ay 1!\n\n";
    
    return 0;
}
