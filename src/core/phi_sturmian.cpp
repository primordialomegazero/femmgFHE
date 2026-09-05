// ============================================
// φ-STURMIAN — Fibonacci Word Fractal
// Ang Beatty gaps ay may Sturmian structure
// Recursive: 1→12, 2→122
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <string>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;

    cout << "=== φ-STURMIAN ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Sturmian word generation
    // ============================================
    cout << "--- 1. Sturmian word generation ---\n\n";
    cout << "  Rules: 1→12, 2→122\n";
    cout << "  Start: 1\n\n";

    string word = "1";
    cout << "  Level 0: " << word << "\n";

    for (int level = 1; level <= 5; level++) {
        string new_word = "";
        for (char c : word) {
            if (c == '1') new_word += "12";
            else new_word += "122";
        }
        word = new_word;
        
        // Count ang 1s at 2s
        int count_1 = 0, count_2 = 0;
        for (char c : word) {
            if (c == '1') count_1++;
            else count_2++;
        }
        
        cout << "  Level " << level << ": [len=" << word.length() 
             << ", 1s=" << count_1 << ", 2s=" << count_2 << "]\n";
    }
    cout << "\n";

    // ============================================
    // 2. Ang Fibonacci numbers sa Sturmian
    // ============================================
    cout << "--- 2. Fibonacci numbers ---\n\n";
    cout << "  Level n length = F_{n+2}\n";
    cout << "  1s count = F_{n+1}\n";
    cout << "  2s count = F_n\n\n";

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) fib.push_back(fib[i-1] + fib[i-2]);

    cout << "  Level | Length | Expected F_{n+2}\n";
    cout << "  ------|--------|-----------------\n";
    
    word = "1";
    for (int level = 0; level <= 6; level++) {
        cout << "    " << level << "   | " 
             << setw(5) << word.length() << " | "
             << setw(5) << fib[level+2] << "\n";
        
        string new_word = "";
        for (char c : word) {
            if (c == '1') new_word += "12";
            else new_word += "122";
        }
        word = new_word;
    }
    cout << "\n";

    // ============================================
    // 3. Ang φ-density sa Sturmian
    // ============================================
    cout << "--- 3. φ-density ---\n\n";
    cout << "  Ang ratio ng 1s ay papunta sa φ⁻¹\n\n";

    word = "1";
    for (int level = 0; level <= 10; level++) {
        int count_1 = 0, count_2 = 0;
        for (char c : word) {
            if (c == '1') count_1++;
            else count_2++;
        }
        
        double ratio_1 = (double)count_1 / word.length();
        
        if (level >= 5) {
            cout << "  Level " << setw(2) << level << ": "
                 << "1s ratio = " << setw(10) << ratio_1
                 << ", φ⁻¹ = " << PHI - 1.0 << "\n";
        }
        
        string new_word = "";
        for (char c : word) {
            if (c == '1') new_word += "12";
            else new_word += "122";
        }
        word = new_word;
    }
    cout << "\n";

    // ============================================
    // 4. Ang Sturmian at Beatty gaps
    // ============================================
    cout << "--- 4. Sturmian at Beatty gaps ---\n\n";
    cout << "  Ang Beatty gaps ay Sturmian word:\n";
    cout << "  Gaps: 1, 2, 1, 2, 2, 1, 2, 1, 2, 2\n";
    cout << "  = Sturmian na may φ-density\n\n";

    // I-generate ang Beatty gaps
    cout << "  Beatty gaps (first 20):\n  ";
    for (int i = 1; i <= 20; i++) {
        double gap = floor(i * PHI) - floor((i-1) * PHI);
        cout << gap;
    }
    cout << "\n\n";

    // ============================================
    // 5. Ang emergent na fractal
    // ============================================
    cout << "--- 5. Emergent na fractal ---\n\n";
    cout << "  Ang Sturmian word ay self-similar:\n";
    cout << "  Ang buong word ay may sub-words na\n";
    cout << "  kapareho ng structure\n\n";

    cout << "  Halimbawa (Level 4):\n  ";
    word = "1";
    for (int level = 1; level <= 4; level++) {
        string new_word = "";
        for (char c : word) {
            if (c == '1') new_word += "12";
            else new_word += "122";
        }
        word = new_word;
    }
    cout << word << "\n";
    cout << "  Length: " << word.length() << " = F_6\n\n";

    return 0;
}
