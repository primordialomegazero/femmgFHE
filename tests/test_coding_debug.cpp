#include "../src/math/anti_lattice.h"
#include <iostream>
#include <iomanip>
using namespace std;
using namespace anti_lattice;

int main() {
    CodingTheoryLayer coding;
    
    // Test message: [1,0,1,0,1,1,0,0, 0...]
    vector<uint8_t> msg(CODE_DIM, 0);
    msg[0] = 1; msg[2] = 1; msg[4] = 1; msg[5] = 1;
    
    cout << "=== ENCODE ===" << endl;
    auto cw = coding.encode(msg);
    cout << "Original msg bits set: ";
    for (int i = 0; i < 8; i++) cout << (int)msg[i];
    cout << endl;
    
    cout << "\n=== GENERATOR MATRIX SAMPLE (rows 0-3, cols 0-7) ===" << endl;
    // We need to access G_ — let's just check encode/decode
    
    cout << "\n=== NO ERRORS: Decode without errors ===" << endl;
    auto dec0 = coding.decode(cw, 0);
    cout << "Decoded: ";
    for (int i = 0; i < 8; i++) cout << (int)dec0[i];
    cout << endl;
    bool perfect = true;
    for (int i = 0; i < 8; i++) if (dec0[i] != msg[i]) perfect = false;
    cout << "Perfect decode (no errors): " << (perfect ? "YES" : "NO") << endl;
    
    cout << "\n=== WITH 3 ERRORS ===" << endl;
    auto cw_err = cw;
    coding.add_errors(cw_err, 3, 123);
    cout << "Codeword after 3 errors (first 16 bits): ";
    for (int i = 0; i < 16; i++) cout << (int)cw_err[i];
    cout << endl;
    
    // Check how many bits actually flipped
    int flips = 0;
    for (int i = 0; i < CODE_LEN; i++) if (cw[i] != cw_err[i]) flips++;
    cout << "Actual bit flips: " << flips << endl;
    
    auto dec3 = coding.decode(cw_err, 3);
    cout << "Decoded: ";
    for (int i = 0; i < 8; i++) cout << (int)dec3[i];
    cout << endl;
    bool ok = true;
    for (int i = 0; i < 8; i++) if (dec3[i] != msg[i]) ok = false;
    cout << "Correct decode (3 errors): " << (ok ? "YES" : "NO") << endl;
    
    // Per-bit analysis
    cout << "\n=== PER-BIT ANALYSIS ===" << endl;
    for (int i = 0; i < 8; i++) {
        int ones = 0, total = 0;
        for (int j = 0; j < CODE_LEN; j++) {
            // We can't access G_ directly, but let's check the decode logic
        }
        cout << "Bit " << i << ": original=" << (int)msg[i] << " decoded=" << (int)dec3[i] << endl;
    }
    
    return ok ? 0 : 1;
}
