// φ-RECURRENCE PSEUDO-BOOTSTRAP V2
// May exception handling at auto-stop pag naubos ang depth
// Para ma-test natin ang recycling theory

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-RECURRENCE PSEUDO-BOOTSTRAP V2\n";
    std::cout << "  With Exception Handling\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    std::cout << "Generating keys...\n";
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    std::cout << "Ring: " << cc->GetRingDimension() << ", Slots: " << slots << "\n";
    std::cout << "Depth: 20\n\n";

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // TEST 1: STANDARD NAND — Count max gates
    // ============================================
    std::cout << "TEST 1: STANDARD NAND (1-mult)\n";
    std::cout << "=================================\n";
    
    auto a = make_ct(1.0);
    auto b = make_ct(0.0);
    auto ct_standard = a;
    int standard_gates = 0;
    
    try {
        for (int i = 0; i < 30; i++) {
            ct_standard = cc->EvalSub(make_ct(1.0), cc->EvalMult(ct_standard, b));
            standard_gates++;
            
            if (i % 5 == 0) {
                double v = decrypt_val(ct_standard);
                std::cout << "  Gate " << i << ": v=" << v 
                          << " level=" << ct_standard->GetLevel() << " ✓\n";
            }
        }
    } catch (const std::exception& e) {
        std::cout << "  [STOPPED at gate " << standard_gates << ": " << e.what() << "]\n";
    }
    
    std::cout << "  Standard NAND max: " << standard_gates << " gates\n\n";

    // ============================================
    // TEST 2: φ-RECURRENCE RECYCLING
    // ============================================
    std::cout << "TEST 2: φ-RECURRENCE RECYCLING\n";
    std::cout << "=================================\n";
    
    auto ct_phi = make_ct(phi);
    auto ct_zero = make_ct(0.0);
    auto ct_recycle = ct_phi;
    int recycled_gates = 0;
    
    try {
        for (int i = 0; i < 40; i++) {
            // φ-domain NAND: NAND(a,b) = φ - a·b·ψ
            auto mult_result = cc->EvalMult(ct_recycle, ct_zero);
            auto scaled = cc->EvalMult(mult_result, make_ct(psi));
            ct_recycle = cc->EvalSub(ct_phi, scaled);
            
            recycled_gates++;
            
            if (i % 10 == 0) {
                double v = decrypt_val(ct_recycle);
                std::cout << "  Gate " << i << ": v=" << v 
                          << " level=" << ct_recycle->GetLevel() 
                          << " (φ=" << phi << ")\n";
            }
            
            // Recycle attempt every 4 gates
            if ((i + 1) % 4 == 0 && i < 36) {
                auto fib3 = make_ct(2.0);  // F(3) = 2
                ct_recycle = cc->EvalSub(ct_recycle, fib3);
                
                double v_after = decrypt_val(ct_recycle);
                std::cout << "  [Recycle at gate " << i << "]: v=" << v_after 
                          << " level=" << ct_recycle->GetLevel() << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cout << "  [STOPPED at gate " << recycled_gates << ": " << e.what() << "]\n";
    }
    
    std::cout << "\n  Recycled NAND: " << recycled_gates << " gates attempted\n\n";

    // ============================================
    // RESULT
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Standard: " << standard_gates << " gates max\n";
    std::cout << "  Recycled: " << recycled_gates << " gates attempted\n";
    std::cout << "  Extension: " << (recycled_gates - standard_gates) 
              << " gates beyond standard!\n";
    std::cout << "========================================\n";

    return 0;
}
