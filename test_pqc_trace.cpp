#include "security_complete.h"
#include <iostream>
#include <cstdio>

int main() {
    security::PhiLatticeKEM kem;
    auto kp = kem.generate_keypair();
    auto [ct, secret] = kem.encapsulate(kp.public_key);
    
    printf("ct.data[0] (from encapsulate) = %016lx\n", ct.data[0]);
    printf("secret[0..7] = ");
    for(int i=0; i<8; i++) printf("%02x", secret[i]);
    printf("\n");
    
    // Manually compute what decapsulate should get
    // Pero hindi accessible ang phi_hash kasi private... let's just run decapsulate
    auto recovered = kem.decapsulate(ct);
    
    printf("recovered[0..7] = ");
    for(int i=0; i<8; i++) printf("%02x", recovered[i]);
    printf("\n");
    
    printf("Match: %s\n", secret == recovered ? "YES" : "NO");
    
    return 0;
}
