// ============================================
// φ-GATES DEBUG — HANAPIN ANG PATTERN
//
// 8/16 ay kalahati — anong pattern?
// I-print ang LAHAT ng raw values
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-GATES DEBUG — RAW VALUES\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(50);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    
    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 10; i++) {
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }
    
    auto encrypt_val = [&](double val) {
        vector<double> v(1, val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };
    
    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return result_pt->GetCKKSPackedValue()[0].real();
    };
    
    // Encoding: 0 → L_2 = 3, 1 → L_4 = 7
    // Threshold: > L_3 = 4
    
    auto enc_0 = encrypt_val(3.0);  // L_2
    auto enc_1 = encrypt_val(7.0);  // L_4
    
    cout << "ENCODING:\n";
    cout << "  0 → " << decrypt_val(enc_0) << " (L_2 = 3)\n";
    cout << "  1 → " << decrypt_val(enc_1) << " (L_4 = 7)\n\n";
    
    cout << "RAW GATE VALUES:\n";
    cout << "========================================\n";
    cout << "  A | B | AND_raw | OR_raw | XOR_raw | NAND_raw\n";
    cout << "  --|---|---------|--------|---------|---------\n";
    
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            auto ct_a = encrypt_val(A == 0 ? 3.0 : 7.0);
            auto ct_b = encrypt_val(B == 0 ? 3.0 : 7.0);
            
            // AND = a + b - L_2 = a + b - 3
            auto and_sum = cc->EvalAdd(ct_a, ct_b);
            auto and_ct = cc->EvalAdd(and_sum, encrypt_val(-3.0));
            
            // OR = a + b + L_2 = a + b + 3
            auto or_ct = cc->EvalAdd(and_sum, encrypt_val(3.0));
            
            // XOR = a + b - L_4 = a + b - 7
            auto xor_ct = cc->EvalAdd(and_sum, encrypt_val(-7.0));
            
            // NAND = L_4 - (a + b - L_2) = 7 - AND
            auto nand_ct = cc->EvalAdd(encrypt_val(7.0), cc->EvalNegate(and_ct));
            
            cout << "  " << A << " | " << B << " | "
                 << setw(7) << fixed << setprecision(1) << decrypt_val(and_ct) << " | "
                 << setw(7) << decrypt_val(or_ct) << " | "
                 << setw(7) << decrypt_val(xor_ct) << " | "
                 << setw(8) << decrypt_val(nand_ct) << "\n";
        }
    }
    
    cout << "\nTHRESHOLD ANALYSIS (threshold = 4):\n";
    cout << "========================================\n";
    cout << "  Gate | (0,0) | (0,1) | (1,0) | (1,1) | Correct\n";
    cout << "  -----|-------|-------|-------|-------|--------\n";
    
    for (string gate_name : {"AND", "OR", "XOR", "NAND"}) {
        cout << "  " << setw(4) << gate_name << " |";
        
        for (int A : {0, 1}) {
            for (int B : {0, 1}) {
                auto ct_a = encrypt_val(A == 0 ? 3.0 : 7.0);
                auto ct_b = encrypt_val(B == 0 ? 3.0 : 7.0);
                auto sum = cc->EvalAdd(ct_a, ct_b);
                
                double raw;
                if (gate_name == "AND") raw = decrypt_val(cc->EvalAdd(sum, encrypt_val(-3.0)));
                else if (gate_name == "OR") raw = decrypt_val(cc->EvalAdd(sum, encrypt_val(3.0)));
                else if (gate_name == "XOR") raw = decrypt_val(cc->EvalAdd(sum, encrypt_val(-7.0)));
                else raw = decrypt_val(cc->EvalAdd(encrypt_val(7.0), cc->EvalNegate(cc->EvalAdd(sum, encrypt_val(-3.0)))));
                
                int thresh = (raw > 4.0) ? 1 : 0;
                cout << " " << setw(5) << thresh;
            }
        }
        cout << "\n";
    }
    
    cout << "\nPATTERN ANALYSIS:\n";
    cout << "========================================\n";
    cout << "  AND: 3,3 → 3 | 3,7 → 7 | 7,3 → 7 | 7,7 → 11\n";
    cout << "  Threshold 4: 3→0, 7→1, 11→1\n";
    cout << "  AND expected: 0, 0, 0, 1 → threshold gives: 0, 1, 1, 1\n\n";
    
    cout << "  PROBLEM: (0,1) at (1,0) ay DAPAT 0 pero naging 1\n";
    cout << "  Kasi 3+7-3 = 7 > 4\n\n";
    
    cout << "  NATURAL FIX:\n";
    cout << "  Imbes na L_2 = 3, gamitin ang L_1 = 1\n";
    cout << "  Imbes na L_4 = 7, gamitin ang L_3 = 4\n\n";
    
    cout << "  NEW ENCODING:\n";
    cout << "  0 → L_1 = 1\n";
    cout << "  1 → L_3 = 4\n";
    cout << "  Threshold: L_2 = 3\n\n";
    
    cout << "  VERIFY:\n";
    cout << "  AND(0,0) = 1+1-1 = 1 < 3 → 0 ✅\n";
    cout << "  AND(0,1) = 1+4-1 = 4 > 3 → 1 ❌ (dapat 0)\n\n";
    
    cout << "  ISA PANG TRY:\n";
    cout << "  0 → L_0 = 2\n";
    cout << "  1 → L_3 = 4\n";
    cout << "  Threshold: L_2 = 3\n\n";
    
    cout << "  AND(0,0) = 2+2-2 = 2 < 3 → 0 ✅\n";
    cout << "  AND(0,1) = 2+4-2 = 4 > 3 → 1 ❌ (dapat 0)\n\n";
    
    cout << "  ANG TAMANG ENCODING:\n";
    cout << "  0 → L_0 = 2\n";
    cout << "  1 → L_4 = 7\n";
    cout << "  Threshold: L_3 = 4\n\n";
    
    cout << "  AND(0,0) = 2+2-2 = 2 < 4 → 0 ✅\n";
    cout << "  AND(0,1) = 2+7-2 = 7 > 4 → 1 ❌ (dapat 0)\n\n";
    
    cout << "  ANG TALAGANG TAMANG ENCODING:\n";
    cout << "  0 → L_0 = 2\n";
    cout << "  1 → L_2 = 3\n";
    cout << "  Threshold: L_1 = 1 (middle ng 2 at 3)\n\n";
    
    cout << "  AND(0,0) = 2+2-2 = 2 > 1 → 1 ❌ (dapat 0)\n\n";
    
    cout << "  CONCLUSION:\n";
    cout << "  Ang Lucas encoding ay kailangan ng\n";
    cout << "  ASYMMETRIC threshold — hindi linear.\n";
    cout << "  Ang natural fix: GAMITIN ANG LOG SPACE!\n\n";
    
    cout << "  LOG SPACE ENCODING:\n";
    cout << "  0 → log(L_0) = log(2)\n";
    cout << "  1 → log(L_4) = log(7)\n";
    cout << "  AND = log(2) + log(7) - log(2) = log(7) → 1\n\n";
    
    cout << "  PERO: Sa log space, ang AND ay multiplication\n";
    cout << "  na zero-level na!\n\n";
    
    cout << "  ========================================\n";
    cout << "  ANG PATTERN NG 8/16\n";
    cout << "  ========================================\n\n";
    cout << "  Ang 8/16 ay dahil ang (0,1) at (1,0)\n";
    cout << "  ay SAME ang raw value (7) sa AND/OR.\n";
    cout << "  Kailangan ng ASYMMETRY para ma-distinguish.\n\n";
    
    cout << "  ANG NATURAL NA ASYMMETRY:\n";
    cout << "  Log space! log(3) ≠ log(7) ≠ log(3)+log(7)\n";
    cout << "  Sa log space, lahat ay unique!\n\n";
    
    return 0;
}
