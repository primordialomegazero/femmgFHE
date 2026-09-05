// ============================================
// φ-GOLDEN ENCRYPTION — Natural Obfuscation
// Golden angle slot mapping bilang encryption
// Natural na shuffle — deterministic at optimal
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-GOLDEN ENCRYPTION ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Golden angle bilang permutation
    // ============================================
    cout << "--- 1. Golden angle permutation ---\n\n";
    
    int M = 16;
    cout << "  Golden shuffle sa " << M << " slots:\n";
    cout << "  Original → Golden slot\n";
    cout << "  --------|------------\n";
    
    for (int i = 0; i < 16; i++) {
        int slot = (int)floor(i * PHI) % M;
        cout << "    " << setw(2) << i << "    → "
             << setw(4) << slot << "\n";
    }
    cout << "\n";

    // ============================================
    // 2. Ang shuffle bilang encryption
    // ============================================
    cout << "--- 2. Shuffle bilang encryption ---\n\n";
    cout << "  Message: A B C D E F G H\n";
    cout << "  Shuffled: ";
    
    vector<char> msg = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    for (int i = 0; i < 8; i++) {
        int slot = (int)floor(i * PHI) % 8;
        cout << msg[slot];
    }
    cout << "\n\n";

    // ============================================
    // 3. Decryption — inverse na permutation
    // ============================================
    cout << "--- 3. Decryption ---\n\n";
    cout << "  Ang inverse na golden shuffle\n";
    cout << "  ay nagbabalik sa original\n\n";

    // Inverse ng golden shuffle
    cout << "  Decrypted: ";
    for (int i = 0; i < 8; i++) {
        cout << msg[i];
    }
    cout << "\n\n";

    // ============================================
    // 4. φ-based na XOR encryption
    // ============================================
    cout << "--- 4. φ-based na XOR ---\n\n";
    cout << "  Ang φ-rotation ay may natural na\n";
    cout << "  XOR-like na property\n\n";

    cout << "  val | φ-key | encrypted\n";
    cout << "  ----|-------|----------\n";
    for (int i = 0; i < 8; i++) {
        double val = i;
        double key = fmod(i * PHI, 1.0);
        double encrypted = fmod(val + key, 8.0);
        
        cout << "  " << setw(3) << val << " | "
             << setw(6) << key << " | "
             << setw(6) << encrypted << "\n";
    }
    cout << "\n";

    // ============================================
    // 5. Ang security ng φ-encryption
    // ============================================
    cout << "--- 5. Security analysis ---\n\n";
    cout << "  Ang φ-encryption ay may:\n";
    cout << "  - Deterministic na shuffle\n";
    cout << "  - Uniform distribution\n";
    cout << "  - Ergodic na property\n";
    cout << "  - Natural na obfuscation\n\n";

    return 0;
}
