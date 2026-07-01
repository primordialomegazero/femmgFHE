#include <iostream>
#include <cmath>
#include <cstdint>
#include "../src/core/banach_engine.h"
#include "../src/chaos/triple_rashomon.h"

using namespace std;

int main() {
    cout << "=== AVALANCHE VERIFICATION ===" << endl;
    
    triple_rashomon::TripleRashomonEngine chaos;
    chaos.set_nonce(0xDEADBEEF);
    
    // Test: Encrypt 42 and 43
    auto [ct42, hist42] = chaos.observe(42.0, 1);
    auto [ct43, hist43] = chaos.observe(43.0, 2);
    
    double diff = abs(ct42 - ct43);
    double phi = 1.6180339887498948482;
    double claimed = pow(phi, 42);
    
    cout << "CT(42) = " << ct42 << endl;
    cout << "CT(43) = " << ct43 << endl;
    cout << "Difference = " << diff << endl;
    cout << "Claim: >= " << claimed << " (phi^42)" << endl;
    cout << "Meets claim? " << (diff >= claimed ? "YES" : "NO") << endl;
    
    // Check noise convergence — 1000 additions
    cout << "\n=== NOISE CONVERGENCE TEST ===" << endl;
    double noise = 1.82815;
    double phi_inv = 0.6180339887498948482;
    for (int i = 0; i < 1000; i++) {
        noise = noise * phi_inv + 1.82815 * (1.0 - phi_inv);
        if (i < 10 || i % 100 == 0) {
            cout << "Op " << i << ": noise = " << noise << " bits" << endl;
        }
    }
    cout << "Final noise after 1000 ops: " << noise << " bits" << endl;
    
    return 0;
}
