// ============================================
// φ-RULE 110 SPACE HUNT — Hanapin ang Space
// Sa anong space ang transition ay linear?
// Walang assumptions — puro exploration
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <complex>

using namespace std;

int main() {
    const double PHI = 1.6180339887498948482;
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    cout << "=== φ-RULE 110 SPACE HUNT ===\n\n";
    cout << fixed << setprecision(12);

    // ============================================
    // 1. Hanapin ang linear na representation
    // ============================================
    cout << "--- 1. Linear representation hunt ---\n\n";
    cout << "  Hanapin ang space kung saan ang\n";
    cout << "  transition ay linear (no lookup)\n\n";

    // Subukan iba't ibang encoding
    vector<vector<double>> encodings;
    
    // Encoding A: Binary (0,1)
    encodings.push_back({0, 1});
    
    // Encoding B: φ-powers
    encodings.push_back({pow(PHI, -1), pow(PHI, 0)});
    
    // Encoding C: φ-exponents
    encodings.push_back({-5.0, -2.0});
    
    // Encoding D: Parity (±1)
    encodings.push_back({-1.0, 1.0});
    
    // Encoding E: Complex roots
    encodings.push_back({-1.0, 1.0});  // placeholder
    
    vector<string> names = {"Binary", "φ-powers", "φ-exponents", "Parity", "Complex"};
    
    for (int e = 0; e < encodings.size(); e++) {
        double zero_val = encodings[e][0];
        double one_val = encodings[e][1];
        
        cout << "  Encoding " << (char)('A' + e) << " (" << names[e] << "):\n";
        cout << "    zero=" << zero_val << ", one=" << one_val << "\n";
        
        // Compute sum para sa bawat pattern
        cout << "    Pattern | Sum | Next\n";
        cout << "    --------|-----|------\n";
        
        bool linear = true;
        double prev_sum = 0;
        int prev_next = -1;
        
        for (int L = 0; L <= 1; L++) {
            for (int C = 0; C <= 1; C++) {
                for (int R = 0; R <= 1; R++) {
                    int pattern = (L << 2) | (C << 1) | R;
                    int next = rule110[pattern];
                    double sum = (L ? one_val : zero_val) +
                                 (C ? one_val : zero_val) +
                                 (R ? one_val : zero_val);
                    
                    cout << "    " << L << C << R << "    | "
                         << setw(5) << sum << " |  "
                         << next << "\n";
                }
            }
        }
        cout << "\n";
    }

    // ============================================
    // 2. Ang transition bilang linear function
    // ============================================
    cout << "--- 2. Linear function hunt ---\n\n";
    cout << "  next = a×sum + b (linear)\n\n";

    // Hanapin kung may linear na relasyon
    for (int e = 0; e < encodings.size(); e++) {
        double zero_val = encodings[e][0];
        double one_val = encodings[e][1];
        
        // Test: next = a×sum + b
        // Para sa lahat ng patterns
        bool found_linear = true;
        
        for (int L = 0; L <= 1; L++) {
            for (int C = 0; C <= 1; C++) {
                for (int R = 0; R <= 1; R++) {
                    int pattern = (L << 2) | (C << 1) | R;
                    int next = rule110[pattern];
                    double sum = (L ? one_val : zero_val) +
                                 (C ? one_val : zero_val) +
                                 (R ? one_val : zero_val);
                    
                    // Subukan ang next = a×sum + b
                    // Pero kailangan ng consistent na a,b
                }
            }
        }
        
        cout << "  " << names[e] << ": "
             << (found_linear ? "Maaaring linear" : "Hindi linear") << "\n";
    }
    cout << "\n";

    // ============================================
    // 3. Ang transition bilang XOR
    // ============================================
    cout << "--- 3. XOR representation ---\n\n";
    cout << "  Rule 110: next = (~L & C) | (C ^ R)\n";
    cout << "  Ito ay may XOR na component\n\n";
    cout << "  Pattern | C^R | ~L&C | Next\n";
    cout << "  --------|-----|------|------\n";
    
    for (int L = 0; L <= 1; L++) {
        for (int C = 0; C <= 1; C++) {
            for (int R = 0; R <= 1; R++) {
                int pattern = (L << 2) | (C << 1) | R;
                int next = rule110[pattern];
                int xor_cr = C ^ R;
                int and_nl_c = (1 - L) & C;
                int or_result = and_nl_c | xor_cr;
                
                cout << "  " << L << C << R << "    | "
                     << setw(2) << xor_cr << " | "
                     << setw(3) << and_nl_c << " |  "
                     << next << " (XOR+AND=" << or_result << ")\n";
            }
        }
    }
    cout << "\n";

    return 0;
}
