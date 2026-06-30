#include "security_complete.h"
#include <cstdio>

int main() {
    security::PhiLatticeKEM kem;
    auto kp = kem.generate_keypair();
    auto [ct, secret] = kem.encapsulate(kp.public_key);
    auto recovered = kem.decapsulate(ct);
    
    printf("secret[0..7]   = ");
    for(int i=0; i<8; i++) printf("%02x", secret[i]);
    printf("\nrecovered[0..7] = ");
    for(int i=0; i<8; i++) printf("%02x", recovered[i]);
    printf("\n");
    
    // Check if recovered is all zeros
    bool all_zero = true;
    for(auto b : recovered) if(b != 0) all_zero = false;
    printf("Recovered all zero: %s\n", all_zero ? "YES" : "NO");
    printf("Match: %s\n", secret == recovered ? "YES" : "NO");
    
    return 0;
}
