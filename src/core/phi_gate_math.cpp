// ============================================
// φ-GATE MATH — ANALYZE ANG LAHAT
//
// Tingnan ang φ² encoding:
// 0 → -2, 1 → +2
//
// Para sa bawat gate:
// NAND: -(A+B)
// NOT: -A
// AND: ?
// OR: ?
// XOR: ?
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-GATE MATH — ANALYSIS\n";
    cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    // φ² encoding
    auto encode = [&](int bit) {
        return (bit == 0) ? -2.0 : 2.0;
    };

    auto decode = [&](double val) {
        return (val >= -0.01) ? 1 : 0;
    };

    // ============================================
    // TRUTH TABLE ANALYSIS
    // ============================================

    cout << "  φ² ENCODING: 0→-2, 1→+2\n\n";
    cout << "  A | B | Enc(A) | Enc(B) | Sum | -Sum | Decode(-Sum) | NAND?\n";
    cout << "  --|---|--------|--------|-----|------|---------------|------\n";

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double enc_a = encode(A);
            double enc_b = encode(B);
            double sum = enc_a + enc_b;
            double neg_sum = -sum;
            int decoded = decode(neg_sum);
            int exp_nand = !(A && B);
            
            cout << "  " << A << " | " << B << " | "
                 << setw(6) << enc_a << " | "
                 << setw(6) << enc_b << " | "
                 << setw(4) << sum << " | "
                 << setw(4) << neg_sum << " | "
                 << setw(13) << decoded << " | "
                 << (decoded == exp_nand ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // AND ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  AND GATE ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  AND(A,B) = NAND(NAND(A,B), NAND(A,B))\n\n";
    cout << "  A | B | NAND | NAND(NAND,NAND) | Decode | AND?\n";
    cout << "  --|---|------|------------------|--------|------\n";

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double enc_a = encode(A);
            double enc_b = encode(B);
            double nand_ab = -(enc_a + enc_b);
            double nand_nand = -(nand_ab + nand_ab);
            int decoded = decode(nand_nand);
            int exp_and = (A && B);
            
            cout << "  " << A << " | " << B << " | "
                 << setw(4) << nand_ab << " | "
                 << setw(16) << nand_nand << " | "
                 << setw(6) << decoded << " | "
                 << (decoded == exp_and ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // OR ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  OR GATE ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  OR(A,B) = NAND(NOT(A), NOT(B))\n\n";
    cout << "  A | B | NOT(A) | NOT(B) | NAND(NOT,NOT) | Decode | OR?\n";
    cout << "  --|---|--------|--------|----------------|--------|-----\n";

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double enc_a = encode(A);
            double enc_b = encode(B);
            double not_a = -enc_a;
            double not_b = -enc_b;
            double nand_not = -(not_a + not_b);
            int decoded = decode(nand_not);
            int exp_or = (A || B);
            
            cout << "  " << A << " | " << B << " | "
                 << setw(6) << not_a << " | "
                 << setw(6) << not_b << " | "
                 << setw(14) << nand_not << " | "
                 << setw(6) << decoded << " | "
                 << (decoded == exp_or ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // XOR ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  XOR GATE ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  XOR(A,B) = NAND(NAND(A, NAND(A,B)), NAND(B, NAND(A,B)))\n\n";
    cout << "  A | B | NAND(AB) | NAND(A,N) | NAND(B,N) | NAND(N1,N2) | Decode | XOR?\n";
    cout << "  --|---|----------|-----------|-----------|-------------|--------|------\n";

    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double enc_a = encode(A);
            double enc_b = encode(B);
            double nand_ab = -(enc_a + enc_b);
            double nand_a_n = -(enc_a + nand_ab);
            double nand_b_n = -(enc_b + nand_ab);
            double nand_final = -(nand_a_n + nand_b_n);
            int decoded = decode(nand_final);
            int exp_xor = (A != B);
            
            cout << "  " << A << " | " << B << " | "
                 << setw(8) << nand_ab << " | "
                 << setw(9) << nand_a_n << " | "
                 << setw(9) << nand_b_n << " | "
                 << setw(11) << nand_final << " | "
                 << setw(6) << decoded << " | "
                 << (decoded == exp_xor ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n";

    // ============================================
    // EMERGENT PATTERN
    // ============================================

    cout << "========================================\n";
    cout << "  EMERGENT PATTERN\n";
    cout << "========================================\n\n";

    cout << "  NAND(A,B) = -(A+B)\n";
    cout << "  Values: ";
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double nand = -(encode(A) + encode(B));
            cout << nand << " ";
        }
    }
    cout << "\n\n";

    cout << "  AND(A,B) = NAND(NAND,NAND)\n";
    cout << "  Values: ";
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double nand = -(encode(A) + encode(B));
            double and_val = -(nand + nand);
            cout << and_val << " ";
        }
    }
    cout << "\n\n";

    cout << "  OR(A,B) = NAND(NOT,NOT)\n";
    cout << "  Values: ";
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double or_val = -(-encode(A) + -encode(B));
            cout << or_val << " ";
        }
    }
    cout << "\n\n";

    cout << "  XOR(A,B) = NAND(NAND(A,NAND), NAND(B,NAND))\n";
    cout << "  Values: ";
    for (int A : {0, 1}) {
        for (int B : {0, 1}) {
            double nand_ab = -(encode(A) + encode(B));
            double nand_a_n = -(encode(A) + nand_ab);
            double nand_b_n = -(encode(B) + nand_ab);
            double xor_val = -(nand_a_n + nand_b_n);
            cout << xor_val << " ";
        }
    }
    cout << "\n\n";

    // ============================================
    // THRESHOLD ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  THRESHOLD ANALYSIS\n";
    cout << "========================================\n\n";

    cout << "  Gate | Values (A=0,1; B=0,1) | Correct Threshold?\n";
    cout << "  -----|----------------------|--------------------\n";

    cout << "  NAND | ";
    for (int A : {0, 1}) for (int B : {0, 1}) cout << -(encode(A) + encode(B)) << " ";
    cout << " | >= -0.01 → 1\n";

    cout << "  AND  | ";
    for (int A : {0, 1}) for (int B : {0, 1}) {
        double nand = -(encode(A) + encode(B));
        cout << -(nand + nand) << " ";
    }
    cout << " | >= -0.01 → 1\n";

    cout << "  OR   | ";
    for (int A : {0, 1}) for (int B : {0, 1}) cout << -(-encode(A) + -encode(B)) << " ";
    cout << " | >= -0.01 → 1\n";

    cout << "  XOR  | ";
    for (int A : {0, 1}) for (int B : {0, 1}) {
        double nand_ab = -(encode(A) + encode(B));
        double nand_a_n = -(encode(A) + nand_ab);
        double nand_b_n = -(encode(B) + nand_ab);
        cout << -(nand_a_n + nand_b_n) << " ";
    }
    cout << " | >= -0.01 → 1\n\n";

    return 0;
}
