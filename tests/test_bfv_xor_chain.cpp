// BFV XOR CHAIN — The Correct Logic Gate
// XOR preserves information, NAND loses it in feedback

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BFV XOR CHAIN\n";
    std::cout << "  Information-Preserving Logic\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextBFVRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetPlaintextModulus(65537);
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
        vec[0] = val;
        return cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetPackedValue()[0];
    };

    auto ct_zero = make_ct(0);
    auto ct_one = make_ct(1);
    auto ct_two = make_ct(2);

    std::cout << "XOR CHAIN TEST:\n";
    std::cout << "===============\n\n";
    
    // XOR(a,b) = a + b - 2ab (sa binary)
    // Para sa {0,1}: XOR = a + b - 2*a*b
    
    // Test basic XOR
    std::cout << "Basic XOR Tests:\n";
    auto xor1 = cc->EvalAdd(ct_zero, ct_zero); // 0+0-0 = 0
    auto xor2 = cc->EvalAdd(ct_zero, ct_one);  // 0+1-0 = 1
    auto xor3 = cc->EvalAdd(ct_one, ct_one);   // 1+1-2 = 0
    auto xor3_fix = cc->EvalSub(xor3, cc->EvalMult(ct_two, cc->EvalMult(ct_one, ct_one)));
    
    std::cout << "  XOR(0,0) = " << decrypt_val(xor1) << " (expect 0)\n";
    std::cout << "  XOR(0,1) = " << decrypt_val(xor2) << " (expect 1)\n";
    std::cout << "  XOR(1,1) = " << decrypt_val(xor3_fix) << " (expect 0)\n\n";
    
    // Chain with XOR feedback
    std::cout << "XOR CHAIN (100 gates):\n";
    std::cout << "======================\n\n";
    
    auto ct_a = ct_zero;
    auto ct_b = ct_one;
    
    int errors = 0;
    int total_gates = 100;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < total_gates; i++) {
        // XOR(a,b) = a + b - 2ab
        auto ct_ab = cc->EvalMult(ct_a, ct_b);
        auto ct_2ab = cc->EvalAdd(ct_ab, ct_ab);  // 2ab
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);
        auto ct_xor = cc->EvalSub(ct_sum, ct_2ab);
        
        // Check result
        int64_t result = decrypt_val(ct_xor);
        int bit = (result == 1) ? 1 : 0;
        
        // Expected: Fibonacci word (1,0,0,1,0,1,1,0,0...)
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
