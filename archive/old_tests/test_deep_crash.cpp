#include "../src/core/true_poly_fhe.h"
#include <iostream>
using namespace std;

int main() {
    TruePolyFHE fhe;
    uint64_t seed = 0xDEADBEEF;
    
    // Reproduce: 20 multiplications then decrypt
    auto ct = fhe.encrypt(2, seed);
    cout << "Starting depth=0, noise=" << ct.noise_level << endl;
    
    for (int i = 0; i < 20; i++) {
        ct = fhe.multiply(ct, fhe.encrypt(2, seed));
        cout << "  depth=" << ct.depth << " noise=" << ct.noise_level;
        cout << " coeffs[0]=" << ct.coeffs[0] << endl;
    }
    
    cout << "\nAttempting decrypt..." << endl;
    cout << "ct.depth=" << ct.depth << endl;
    cout << "ct.nonce=" << ct.nonce << endl;
    
    // Manual decrypt step-by-step
    int64_t noise0 = fhe.small_noise(seed, 0, ct.nonce);
    cout << "noise0=" << noise0 << endl;
    
    int64_t cleaned = ct.coeffs[0] - noise0;
    cout << "cleaned=" << cleaned << endl;
    
    // Calculate divisor
    int64_t divisor = 1024; // DELTA
    cout << "divisor start=" << divisor << endl;
    for (int d = 0; d < ct.depth; d++) {
        cout << "  depth loop " << d << ": divisor=" << divisor << " * 1024 = ";
        divisor *= 1024;
        cout << divisor << endl;
        if (divisor == 0) {
            cout << "  💀 DIVISOR BECAME ZERO! Overflow!" << endl;
            break;
        }
    }
    
    cout << "Final divisor=" << divisor << endl;
    if (divisor != 0) {
        cout << "Result=" << (cleaned / divisor) << endl;
    }
    
    return 0;
}
