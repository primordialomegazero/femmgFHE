#include "../src/core/true_poly_fhe.h"
#include <iostream>
#include <iomanip>
using namespace std;

int main() {
    TruePolyFHE fhe;
    uint64_t seed = 42;
    
    cout << "=== MULTIPLICATION DEBUG ===" << endl;
    
    // Encrypt simple values
    auto a = fhe.encrypt(6, seed);
    auto b = fhe.encrypt(7, seed);
    
    cout << "\n--- Ciphertext a (pt=6) ---" << endl;
    cout << "coeffs[0]=" << a.coeffs[0] << " (should be 6*" << (1<<10) << " + noise)" << endl;
    cout << "noise_level=" << a.noise_level << endl;
    cout << "nonce=" << a.nonce << endl;
    
    cout << "\n--- Ciphertext b (pt=7) ---" << endl;
    cout << "coeffs[0]=" << b.coeffs[0] << " (should be 7*" << (1<<10) << " + noise)" << endl;
    cout << "noise_level=" << b.noise_level << endl;
    cout << "nonce=" << b.nonce << endl;
    
    // Multiply
    auto c = fhe.multiply(a, b);
    
    cout << "\n--- Result c (should be 42) ---" << endl;
    cout << "coeffs[0]=" << c.coeffs[0] << endl;
    cout << "coeffs[1]=" << c.coeffs[1] << endl;
    cout << "coeffs[2]=" << c.coeffs[2] << endl;
    cout << "coeffs[3]=" << c.coeffs[3] << endl;
    cout << "noise_level=" << c.noise_level << endl;
    
    // Manual decrypt to see raw scaled value
    int64_t noise0 = fhe.small_noise(seed, 0, c.nonce);
    int64_t cleaned = c.coeffs[0] - noise0;
    cout << "\n--- Manual Decrypt ---" << endl;
    cout << "noise0=" << noise0 << endl;
    cout << "cleaned=" << cleaned << endl;
    cout << "cleaned/DELTA=" << (cleaned / (1<<10)) << endl;
    cout << "cleaned/DELTA^2=" << (cleaned / ((int64_t)(1<<10) * (1<<10))) << endl;
    
    // Try removing noise from ALL coeffs then summing
    int64_t total = 0;
    for (int i = 0; i < 64; i++) {
        int64_t n = fhe.small_noise(seed, i, c.nonce);
        total += (c.coeffs[i] - n);
    }
    cout << "\ntotal cleaned sum=" << total << endl;
    cout << "total/DELTA^2=" << (total / ((int64_t)(1<<10) * (1<<10))) << endl;
    
    return 0;
}
