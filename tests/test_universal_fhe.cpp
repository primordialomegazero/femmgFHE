#include "../src/fhe/golden_universal_fhe.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "UNIVERSAL L(k) FHE — FINAL TEST\n";
    std::cout << "===============================\n\n";

    NTL::ZZ Q = NTL::to_ZZ("32317006071311007300714876688669951960444102669715484032130345427524655138867890893197201411522913463688717960921898019494119559150490921095088152386448283120630877367300996091750197750389652106796057638384067568276792218642619756161838094338476170470581645852036305042887575891541065808607552399123930385521914333389668342420684974786564569494856176035326322058077805659331026192708460314150258592864177116725943603718461857357598351152301645904403697613233287231227125684710820209725157101726931323469678542580656697935045997268352998638215525166389437335543602135433229604645318478604952148193555853611059596217801");

    golden_universal_fhe::UniversalFHE fhe(Q, 2048);

    std::cout << "Public keys:\n";
    std::cout << "  Q bits: " << NTL::NumBits(Q) << "\n";
    std::cout << "  L(k) bits: " << NTL::NumBits(fhe.L_k) << "\n";
    std::cout << "  inv_L(k) bits: " << NTL::NumBits(fhe.inv_L_k) << "\n\n";

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

    std::cout << "Derived gates:\n";
    std::cout << "  XOR(0,1) = " << fhe.decrypt(fhe.xor_gate(ct0, ct1)) << " (exp 1)\n";
    std::cout << "  AND(1,1) = " << fhe.decrypt(fhe.and_gate(ct1, ct1)) << " (exp 1)\n";
    std::cout << "  OR(0,0) = " << fhe.decrypt(fhe.or_gate(ct0, ct0)) << " (exp 0)\n\n";

    std::cout << "Deep NAND chain (1000 depths):\n";
    auto current = ct1;
    int errors = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i <= 1000; i++) {
        bool dec = fhe.decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = fhe.nand(current, current);
    }
    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "  Errors: " << errors << "/1001\n";
    std::cout << "  Time: " << ms << " ms\n";
    std::cout << "  Ops/sec: " << (1000.0 / (ms/1000.0)) << "\n";

    return 0;
}
