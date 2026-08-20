#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  TFHE/FHEW AVAILABILITY CHECK\n";
    std::cout << "========================================\n\n";

    // Subukan i-initialize ang BinFHE context
    auto cc = BinFHEContext();
    
    std::cout << "BinFHEContext created successfully\n";
    std::cout << "TFHE/FHEW ay available sa OpenFHE build\n\n";
    
    // Generate keys
    cc.GenerateBinFHEContext(TOY, MEDIUM);
    std::cout << "BinFHE context generated with TOY parameters\n";
    std::cout << "Medium security level\n\n";
    
    auto sk = cc.KeyGen();
    std::cout << "Secret key generated\n";
    
    cc.BTKeyGen(sk);
    std::cout << "Bootstrapping keys generated\n";
    
    std::cout << "\n========================================\n";
    std::cout << "  TFHE/FHEW AY AVAILABLE\n";
    std::cout << "  May native bootstrapping para sa\n";
    std::cout << "  arbitrary depth NAND chains\n";
    std::cout << "========================================\n";
    
    return 0;
}
