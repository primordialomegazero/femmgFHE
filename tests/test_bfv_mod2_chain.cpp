// BFV MOD2 CHAIN — Addition Mod 2 as XOR
// Mas simple, less noise accumulation

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BFV MOD2 CHAIN\n";
    std::cout << "  Addition Mod 2 = XOR\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetPlaintextModulus(2);  // Mod 2 para sa XOR!
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](int64_t val) {
        std::vector<int64_t> vec(slots, 0);
        vec[0] = val % 2;  // Ensure 0 or 1
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto ct_zero = make_ct(0);
    auto ct_one = make_ct(1);

    std::cout << "MOD2 XOR CHAIN TEST:\n";
    std::cout << "====================\n\n";
    
    // XOR sa mod 2: simple addition lang!
    std::cout << "Basic XOR (mod 2):\n";
    auto xor1 = cc->EvalAdd(ct_zero, ct_zero);  // 0+0=0
    auto xor2 = cc->EvalAdd(ct_zero, ct_one);   // 0+1=1
    auto xor3 = cc->EvalAdd(ct_one, ct_one);    // 1+1=0 (mod 2)
    
    std::cout << "  XOR(0,0) = " << decrypt_val(xor1) << " (expect 0)\n";
    std::cout << "  XOR(0,1) = " << decrypt_val(xor2) << " (expect 1)\n";
    std::cout << "  XOR(1,1) = " << decrypt_val(xor3) << " (expect 0)\n\n";
    
    // Chain with mod 2 addition
    std::cout << "MOD2 CHAIN (100 gates):\n";
    std::cout << "=======================\n\n";
    
    auto ct_a = ct_zero;
    auto ct_b = ct_one;
    
    int errors = 0;
    int total_gates = 100;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < total_gates; i++) {
        // XOR = simple addition mod 2
        auto ct_xor = cc->EvalAdd(ct_a, ct_b);
        
        // Check result
        int64_t result = decrypt_val(ct_xor);
        int bit = (result % 2 == 1) ? 1 : 0;
        
        // Expected: Fibonacci word pattern
        int expected;
        if (i % 3 == 0) expected = 1;
        else if (i % 3 == 1) expected = 0;
        else expected = 0;
        
        if (bit != expected) {
            errors++;
            if (errors <= 5) {
                std::cout << "  ERROR gate " << i << ": got " << bit 
                          << " expected " << expected 
                          << " (val=" << result << ")\n";
            }
        }
        
        // Update state
        ct_a = ct_b;
        ct_b = ct_xor;
        
        if (i < 5 || i % 20 == 0) {
            std::cout << "  Gate " << i << ": bit=" << bit 
                      << " expected=" << expected
                      << " val=" << result
                      << " level=" << ct_b->GetLevel() << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start);
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "========================================\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Error rate: " << (errors * 100.0 / total_gates) << "%\n";
    std::cout << "  Time: " << duration.count() << " seconds\n";
    std::cout << "  Status: " << (errors == 0 ? "✓ HOLY GRAIL!" : 
              (errors < 5 ? "⚠️ VERY CLOSE!" : "✗ NEEDS WORK")) << "\n";
    std::cout << "========================================\n";
    
    return 0;
}
