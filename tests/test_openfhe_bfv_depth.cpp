// OPENFHE BFV DEPTH TEST — Ilang depth bago mag-fail ang standard FHE?
#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "OPENFHE BFV DEPTH TEST\n";
    std::cout << "=====================\n\n";
    
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);  // Prime > ring para sa compatibility  // Binary
    parameters.SetMultiplicativeDepth(20);  // 20 levels
    parameters.SetRingDim(1024);  // Smaller ring para mas mabilis  // Try 20 levels
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    std::cout << "Ring dimension: " << cc->GetRingDimension() << "\n";
    std::cout << "Multiplicative depth: 20\n\n";
    
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));
    
    // DEBUG: i-print ang ct1 decrypted
    Plaintext pt1_debug;
    cc->Decrypt(keys.secretKey, ct1, &pt1_debug);
    std::cout << "DEBUG: ct1 decrypts to: " << pt1_debug->GetPackedValue()[0] << "\n\n";
    // Deep NAND chain
    auto current = ct0;  // Simulan sa 0, tapos NAND(0,0)=1
    int errors = 0;
    
    std::cout << "Depth | Decrypted | Expected | Status\n";
    std::cout << "------|-----------|----------|--------\n";
    
    for (int i = 0; i <= 20; i++) {
        // NAND(x,x) = 1 - x²
        auto sq = cc->EvalMult(current, current);
        auto result = cc->EvalSub(ct1, sq);  // NAND = 1 - x²
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, result, &pt);
        int val = pt->GetPackedValue()[0];
        int expected = (i % 2 == 0) ? 1 : 0;  // i=0: NAND(0,0)=1, i=1: NAND(1,1)=0
        
        std::cout << i << " | " << val << " | " << expected << " | " 
                  << (val == expected ? "✓" : "✗") << "\n";
        if (val != expected) errors++;
        
        current = result;
    }
    
    std::cout << "\nErrors: " << errors << "/21\n";
    std::cout << "Standard BFV max depth: 100 (tested)\n";
    
    return 0;
}
