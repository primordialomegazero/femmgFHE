#include "pqc_simple.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Simple PQC KEM Test\n";
    std::cout << "===================\n\n";

    pqc_simple::SimpleKEM kem;

    // Generate keypair
    auto kp = kem.generate();
    std::cout << "Keypair generated\n";

    // Encapsulate
    auto [ct, secret] = kem.encapsulate(kp.public_key);
    std::cout << "Encapsulated\n";

    // Decapsulate
    auto recovered = kem.decapsulate(ct);
    std::cout << "Decapsulated\n\n";

    // Check
    bool ok = (secret == recovered);
    std::cout << "Secret: ";
    for(int i=0; i<8; i++) printf("%02x", secret[i]);
    printf("...\n");

    std::cout << "Recovered: ";
    for(int i=0; i<8; i++) printf("%02x", recovered[i]);
    printf("...\n");

    std::cout << "\nMatch: " << (ok ? "✅ PASS" : "❌ FAIL") << "\n";
    return 0;
}
