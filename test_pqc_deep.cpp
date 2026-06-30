#include "security_complete.h"
#include <cstdio>
#include <cstring>

int main() {
    security::PhiLatticeKEM kem;
    auto kp = kem.generate_keypair();
    
    // I-print ang unang 2 elements ng public_key vs public_lattice
    // Pero public_lattice is private... kaya sa encapsulate/decapsulate na lang
    
    auto [ct, secret] = kem.encapsulate(kp.public_key);
    
    // Print ct.data[0] and first phi_hash values
    printf("ct.data[0] = %016lx\n", ct.data[0]);
    printf("ct.nonce first 8 bytes: ");
    for(int i=0; i<8; i++) printf("%02x", ((uint8_t*)ct.nonce.to_hex().c_str())[i]);
    printf("\n");
    
    auto recovered = kem.decapsulate(ct);
    printf("secret[0..3] = %02x%02x%02x%02x\n", secret[0], secret[1], secret[2], secret[3]);
    printf("recov[0..3]  = %02x%02x%02x%02x\n", recovered[0], recovered[1], recovered[2], recovered[3]);
    
    // Simple XOR test: if ct.data[0] = phi_hash(pub) ^ nonce ^ phi_seed
    // then secret should be recoverable
    printf("Match: %s\n", secret == recovered ? "YES" : "NO");
    
    return 0;
}
