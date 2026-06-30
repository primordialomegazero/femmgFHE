#include "security_complete.h"
#include <iostream>

int main() {
    security::PhiLatticeKEM kem;
    auto kp = kem.generate_keypair();
    
    std::cout << "kp.public_key.size() = " << kp.public_key.size() << "\n";
    
    // Check kung ang public_lattice ay populated
    auto [ct, secret] = kem.encapsulate(kp.public_key);
    std::cout << "secret.size() = " << secret.size() << "\n";
    std::cout << "ct.data.size() = " << ct.data.size() << "\n";
    
    auto recovered = kem.decapsulate(ct);
    std::cout << "recovered.size() = " << recovered.size() << "\n";
    
    // Check first 4 bytes
    std::cout << "secret[0..3]: ";
    for(int i=0; i<4; i++) printf("%02x", secret[i]);
    std::cout << "\nrecovered[0..3]: ";
    for(int i=0; i<4; i++) printf("%02x", recovered[i]);
    std::cout << "\n";
    
    return 0;
}
