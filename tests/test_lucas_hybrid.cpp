#include "../src/fhe/golden_lucas_hybrid.h"
#include <iostream>

int main() {
    std::cout << "LUCAS HYBRID FHE TEST\n";
    std::cout << "=====================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");

    golden_lucas_hybrid::LucasHybridFHE fhe(Q, 2048);

    std::cout << "L(2048) bits: " << NTL::NumBits(fhe.L_k) << "\n\n";

    auto ct0 = fhe.encrypt(false);
    auto ct1 = fhe.encrypt(true);

    std::cout << "Encrypt/Decrypt:\n";
    std::cout << "  Decrypt(ct0) = " << fhe.decrypt(ct0) << " (exp 0)\n";
    std::cout << "  Decrypt(ct1) = " << fhe.decrypt(ct1) << " (exp 1)\n\n";

    std::cout << "NAND gates:\n";
    std::cout << "  NAND(0,0) = " << fhe.decrypt(fhe.nand(ct0, ct0)) << " (exp 1)\n";
    std::cout << "  NAND(0,1) = " << fhe.decrypt(fhe.nand(ct0, ct1)) << " (exp 1)\n";
    std::cout << "  NAND(1,0) = " << fhe.decrypt(fhe.nand(ct1, ct0)) << " (exp 1)\n";
    std::cout << "  NAND(1,1) = " << fhe.decrypt(fhe.nand(ct1, ct1)) << " (exp 0)\n\n";

    std::cout << "Deep NAND chain (1000 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 1000; i++) {
        bool dec = fhe.decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = fhe.nand(current, current);
    }
    std::cout << "  Errors: " << errors << "/1001\n";

    return 0;
}
