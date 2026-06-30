#include "security_complete.h"
#include <cstdio>

int main() {
    security::PhiLatticeKEM kem;
    auto kp = kem.generate_keypair();
    
    printf("kp.public_key.size() = %zu\n", kp.public_key.size());
    printf("kp.public_key[0] = %016lx\n", kp.public_key[0]);
    
    // Check kung ang encapsulate ay nagbabasa ng public_key parameter
    auto [ct, secret] = kem.encapsulate(kp.public_key);
    printf("ct.data[0] = %016lx\n", ct.data[0]);
    
    auto recovered = kem.decapsulate(ct);
    printf("recovered[0] = %02x\n", recovered[0]);
    
    return 0;
}
