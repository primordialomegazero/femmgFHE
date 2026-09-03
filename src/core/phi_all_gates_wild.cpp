// ============================================
// φ-ALL GATES WILD — LAHAT NG LOGIC GATES
//
// Encoding: 0→0, 1→2
// Gates: AND, OR, XOR, NAND, NOR, XNOR, NOT
// Lahat sa mod 4 space
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

const double PHI = 1.6180339887498948482;

double encode(int bit) {
    return (bit == 0) ? 0.0 : 2.0;
}

int decode(double result) {
    double mod4 = fmod(result, 4.0);
    if (mod4 < 0) mod4 += 4.0;
    return (mod4 < 2.0) ? 0 : 1;
}

int main() {
    cout << "========================================\n";
    cout << "  φ-ALL GATES WILD\n";
    cout << "========================================\n\n";
    cout << "  Encoding: 0→0, 1→2\n";
    cout << "  Decode: mod 4\n\n";

    // ============================================
    // TRUTH TABLE
    // ============================================

    cout << "========================================\n";
    cout << "  COMPLETE TRUTH TABLE\n";
    cout << "========================================\n\n";
    cout << "  A B | AND | OR | XOR | NAND | NOR | XNOR\n";
    cout << "  ----|-----|----|-----|------|-----|-----\n";

    int total_gates = 0;
    int total_match = 0;

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double a = encode(A);
            double b = encode(B);
            
            // AND: A * B (sa mod 4)
            double and_result = (a * b) / 2.0;
            int and_decoded = decode(and_result);
            int and_expected = A & B;
            
            // OR: A + B - A*B/2
            double or_result = a + b - (a * b) / 2.0;
            int or_decoded = decode(or_result);
            int or_expected = A | B;
            
            // XOR: A + B (mod 4)
            double xor_result = a + b;
            int xor_decoded = decode(xor_result);
            int xor_expected = A ^ B;
            
            // NAND: NOT(AND)
            double nand_result = 2.0 - and_result;
            int nand_decoded = decode(nand_result);
            int nand_expected = !(A & B);
            
            // NOR: NOT(OR)
            double nor_result = 2.0 - or_result;
            int nor_decoded = decode(nor_result);
            int nor_expected = !(A | B);
            
            // XNOR: NOT(XOR)
            double xnor_result = 2.0 - xor_result;
            int xnor_decoded = decode(xnor_result);
            int xnor_expected = !(A ^ B);
            
            total_gates += 6;
            total_match += (and_decoded == and_expected) + 
                           (or_decoded == or_expected) + 
                           (xor_decoded == xor_expected) + 
                           (nand_decoded == nand_expected) + 
                           (nor_decoded == nor_expected) + 
                           (xnor_decoded == xnor_expected);
            
            cout << "  " << A << " " << B << " |  "
                 << and_decoded << "  |  "
                 << or_decoded << "  |  "
                 << xor_decoded << "  |   "
                 << nand_decoded << "   |  "
                 << nor_decoded << "  |   "
                 << xnor_decoded << "\n";
        }
    }

    cout << "\n  Gate Match: " << total_match << "/" << total_gates << "\n\n";

    // ============================================
    // NOT GATE
    // ============================================

    cout << "========================================\n";
    cout << "  NOT GATE\n";
    cout << "========================================\n\n";
    cout << "  A | NOT | Expected | Match?\n";
    cout << "  --|-----|----------|--------\n";

    int not_match = 0;
    for (int A : {0, 1}) {
        double a = encode(A);
        double not_result = 2.0 - a;
        int not_decoded = decode(not_result);
        int not_expected = !A;
        bool ok = (not_decoded == not_expected);
        not_match += ok;
        
        cout << "  " << A << " |  " << not_decoded << "  |     "
             << not_expected << "     | " << (ok ? "✅" : "❌") << "\n";
    }
    cout << "\n  NOT Match: " << not_match << "/2\n\n";

    // ============================================
    // 3-INPUT GATES
    // ============================================

    cout << "========================================\n";
    cout << "  3-INPUT GATES (AND3, OR3, XOR3)\n";
    cout << "========================================\n\n";
    cout << "  A B C | AND3 | OR3 | XOR3\n";
    cout << "  -------|------|-----|-----\n";

    int match3 = 0;
    int total3 = 0;
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            for (int C : {0, 1}) {
                double a = encode(A);
                double b = encode(B);
                double c = encode(C);
                
                // AND3: A * B * C / 4
                double and3_result = (a * b * c) / 4.0;
                int and3_decoded = decode(and3_result);
                int and3_expected = A & B & C;
                
                // OR3: A + B + C - AB/2 - AC/2 - BC/2 + ABC/4
                double or3_result = a + b + c - (a*b)/2.0 - (a*c)/2.0 - (b*c)/2.0 + (a*b*c)/4.0;
                int or3_decoded = decode(or3_result);
                int or3_expected = A | B | C;
                
                // XOR3: A + B + C (mod 4)
                double xor3_result = a + b + c;
                int xor3_decoded = decode(xor3_result);
                int xor3_expected = A ^ B ^ C;
                
                total3 += 3;
                match3 += (and3_decoded == and3_expected) + 
                          (or3_decoded == or3_expected) + 
                          (xor3_decoded == xor3_expected);
                
                cout << "  " << A << " " << B << " " << C << " |  "
                     << and3_decoded << "   |  "
                     << or3_decoded << "  |  "
                     << xor3_decoded << "\n";
            }
        }
    }

    cout << "\n  3-Input Match: " << match3 << "/" << total3 << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================

    cout << "========================================\n";
    cout << "  ALL GATES SUMMARY\n";
    cout << "========================================\n\n";
    cout << "  ✅ 2-input gates: " << total_match << "/" << total_gates << "\n";
    cout << "  ✅ NOT gates: " << not_match << "/2\n";
    cout << "  ✅ 3-input gates: " << match3 << "/" << total3 << "\n\n";

    return 0;
}
