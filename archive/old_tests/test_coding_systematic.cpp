#include "../src/math/anti_lattice.h"
#include <iostream>
using namespace std;
using namespace anti_lattice;

int main() {
    CodingTheoryLayer coding;
    
    // Test message: first 8 bits set
    vector<uint8_t> msg(CODE_DIM, 0);
    msg[0] = 1; msg[2] = 1; msg[4] = 1; msg[5] = 1;
    
    cout << "=== SYSTEMATIC CODE TEST ===" << endl;
    cout << "Message (first 8 bits): ";
    for (int i = 0; i < 8; i++) cout << (int)msg[i];
    cout << endl;
    
    auto cw = coding.encode(msg);
    cout << "Codeword (first 40 bits): ";
    for (int i = 0; i < 40; i++) cout << (int)cw[i];
    cout << endl;
    
    // Systematic check: first CODE_DIM bits should equal message
    bool systematic_ok = true;
    for (int i = 0; i < 8; i++) {
        if (cw[i] != msg[i]) {
            cout << "FAIL at bit " << i << ": cw=" << (int)cw[i] << " msg=" << (int)msg[i] << endl;
            systematic_ok = false;
        }
    }
    cout << "Systematic property (first 8 bits match): " << (systematic_ok ? "YES" : "NO") << endl;
    
    // No errors decode
    auto dec0 = coding.decode(cw, 0);
    cout << "\nDecoded without errors (first 8): ";
    for (int i = 0; i < 8; i++) cout << (int)dec0[i];
    cout << endl;
    bool perfect0 = true;
    for (int i = 0; i < 8; i++) if (dec0[i] != msg[i]) perfect0 = false;
    cout << "Perfect decode (no errors): " << (perfect0 ? "YES" : "NO") << endl;
    
    // With 1 error
    auto cw1 = cw;
    cw1[10] ^= 1;  // flip one bit
    auto dec1 = coding.decode(cw1, 1);
    cout << "\nWith 1 error at bit 10 (first 8 decoded): ";
    for (int i = 0; i < 8; i++) cout << (int)dec1[i];
    cout << endl;
    bool ok1 = true;
    for (int i = 0; i < 8; i++) if (dec1[i] != msg[i]) ok1 = false;
    cout << "Correct decode (1 error): " << (ok1 ? "YES" : "NO") << endl;
    
    // With 3 errors
    auto cw3 = cw;
    coding.add_errors(cw3, 3, 456);
    cout << "\nWith 3 errors (first 40 bits): ";
    for (int i = 0; i < 40; i++) cout << (int)cw3[i];
    cout << endl;
    
    // Check how many errors in identity part vs parity part
    int id_errors = 0, par_errors = 0;
    for (int i = 0; i < CODE_DIM; i++) if (cw[i] != cw3[i]) id_errors++;
    for (int i = CODE_DIM; i < CODE_LEN; i++) if (cw[i] != cw3[i]) par_errors++;
    cout << "Errors in identity part: " << id_errors << endl;
    cout << "Errors in parity part: " << par_errors << endl;
    
    auto dec3 = coding.decode(cw3, 3);
    cout << "Decoded (first 8): ";
    for (int i = 0; i < 8; i++) cout << (int)dec3[i];
    cout << endl;
    
    // Per-bit vote analysis
    cout << "\n=== VOTE ANALYSIS ===" << endl;
    for (int i = 0; i < 8; i++) {
        int votes1 = 0, votes0 = 0;
        if (cw3[i]) votes1++; else votes0++;
        int parity_votes = 0;
        for (int j = CODE_DIM; j < CODE_LEN; j++) {
            // G_[i][j] - we can check if cw[i] influenced cw[j]
            // If G_[i][j]==1, then cw[j] should equal msg[i] XOR other bits
            // For now just count rx bits where G_[i][j] might be 1
        }
        cout << "Bit " << i << ": id_vote=" << (int)cw3[i] 
             << " original=" << (int)msg[i] 
             << " decoded=" << (int)dec3[i] << endl;
    }
    
    return 0;
}
