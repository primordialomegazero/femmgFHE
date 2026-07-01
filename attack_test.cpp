#include "femmg_operations.h"
#include <iostream>
#include <cmath>
using namespace std;

int main() {
    banach::NDimBanachEngine eng;
    eng.set_chaos_nonce(0xDEADBEEF);
    
    auto ct42 = eng.encrypt(42, 0);
    auto ct43 = eng.encrypt(43, 0);
    
    double diff = abs(ct42.coordinates[0] - ct43.coordinates[0]);
    cout << "=== AVALANCHE CHECK ===" << endl;
    cout << "CT(42)[0] = " << ct42.coordinates[0] << endl;
    cout << "CT(43)[0] = " << ct43.coordinates[0] << endl;
    cout << "Difference = " << diff << endl;
    cout << "Claim: >= " << pow(1.6180339887498948482, 42) << " (phi^42)" << endl;
    cout << "Actual: " << diff << endl;
    cout << "Meets claim? " << (diff >= pow(1.6180339887498948482, 42) ? "YES" : "NO") << endl;
    
    // Bit difference analysis
    uint64_t bits42 = *(uint64_t*)&ct42.coordinates[0];
    uint64_t bits43 = *(uint64_t*)&ct43.coordinates[0];
    uint64_t xor_val = bits42 ^ bits43;
    int flipped = __builtin_popcountll(xor_val);
    cout << "\nBits flipped: " << flipped << "/64 (" << (flipped*100.0/64) << "%)" << endl;
    
    return 0;
}
