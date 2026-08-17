// GOLDEN IO FHE — ARBITRARY CIRCUIT VALIDATION
// Full NAND circuit na may period-2 trace erasure

#include "../src/fhe/golden_io_fhe.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>

int main() {
    std::cout << "GOLDEN IO FHE — ARBITRARY CIRCUIT\n";
    std::cout << "=================================\n\n";

    golden_io_fhe::GoldenIOFHE fhe(42);

    std::cout << "1. L(k) = " << fhe.L_k << "\n\n";

    // Random circuit testing
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    const int NUM_TESTS = 100;
    int passed = 0;
    int failed = 0;

    std::cout << "2. ARBITRARY CIRCUIT TESTS (" << NUM_TESTS << "):\n\n";

    // Circuit 1: Full Adder (sum at carry)
    std::cout << "   CIRCUIT 1: FULL ADDER (8 test cases)\n";
    int full_adder_pass = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            for (int cin = 0; cin <= 1; cin++) {
                auto ca = fhe.encrypt(a);
                auto cb = fhe.encrypt(b);
                auto cc = fhe.encrypt(cin);

                // Full Adder: sum = XOR(XOR(a,b), cin), carry = OR(AND(a,b), AND(XOR(a,b), cin))
                auto xor_ab = fhe.xor_gate(ca, cb);
                auto sum = fhe.xor_gate(xor_ab, cc);

                auto and_ab = fhe.nand(fhe.nand(ca, cb), fhe.nand(ca, cb));
                auto and_xor_cin = fhe.nand(fhe.nand(xor_ab, cc), fhe.nand(xor_ab, cc));
                auto carry = fhe.nand(fhe.nand(and_ab, and_ab), fhe.nand(and_xor_cin, and_xor_cin));

                int exp_sum = (a + b + cin) % 2;
                int exp_carry = (a + b + cin) >= 2;

                if (fhe.decrypt(sum) == exp_sum && fhe.decrypt(carry) == exp_carry) {
                    full_adder_pass++;
                }
            }
        }
    }
    std::cout << "   Full Adder: " << full_adder_pass << "/8 PASS\n\n";

    // Circuit 2: 2-to-1 MUX (6 test cases)
    std::cout << "   CIRCUIT 2: 2-TO-1 MUX (6 test cases)\n";
    int mux_pass = 0;
    for (int sel = 0; sel <= 1; sel++) {
        for (int i0 = 0; i0 <= 1; i0++) {
            for (int i1 = 0; i1 <= 1; i1++) {
                auto cs = fhe.encrypt(sel);
                auto c0 = fhe.encrypt(i0);
                auto c1 = fhe.encrypt(i1);

                // MUX = (sel AND i1) OR (NOT(sel) AND i0)
                auto not_sel = fhe.nand(cs, cs); // NOT(sel)
                auto and1 = fhe.nand(fhe.nand(cs, c1), fhe.nand(cs, c1));
                auto and0 = fhe.nand(fhe.nand(not_sel, c0), fhe.nand(not_sel, c0));
                auto or_out = fhe.nand(fhe.nand(and1, and1), fhe.nand(and0, and0));

                int expected = sel ? i1 : i0;
                if (fhe.decrypt(or_out) == expected) mux_pass++;
            }
        }
    }
    std::cout << "   2-to-1 MUX: " << mux_pass << "/8 PASS\n\n";

    // Circuit 3: Random NAND chain (100 operations)
    std::cout << "   CIRCUIT 3: RANDOM NAND CHAIN (100 operations)\n";
    int chain_pass = 0;
    int chain_total = 100;
    for (int i = 0; i < chain_total; i++) {
        int a = dis(gen);
        int b = dis(gen);
        auto ca = fhe.encrypt(a);
        auto cb = fhe.encrypt(b);
        auto result = fhe.nand(ca, cb);
        
        int expected = (a == 1 && b == 1) ? 0 : 1;
        if (fhe.decrypt(result) == expected) chain_pass++;
    }
    std::cout << "   NAND chain: " << chain_pass << "/" << chain_total << " PASS\n\n";

    // Circuit 4: Period-2 trace erasure (50 operations)
    std::cout << "   CIRCUIT 4: PERIOD-2 TRACE ERASURE (50 operations)\n";
    int period2_pass = 0;
    for (int i = 0; i < 50; i++) {
        int x = dis(gen);
        auto cx = fhe.encrypt(x);
        auto result = fhe.nand_period2(cx);
        
        int expected = x;  // NAND(NAND(x)) = x
        if (fhe.decrypt(result) == expected) period2_pass++;
    }
    std::cout << "   Period-2: " << period2_pass << "/50 PASS\n\n";

    // Summary
    std::cout << "3. SUMMARY:\n";
    std::cout << "   Full Adder: " << full_adder_pass << "/8\n";
    std::cout << "   MUX: " << mux_pass << "/8\n";
    std::cout << "   NAND chain: " << chain_pass << "/" << chain_total << "\n";
    std::cout << "   Period-2: " << period2_pass << "/50\n";
    
    int total_pass = full_adder_pass + mux_pass + chain_pass + period2_pass;
    int total_tests = 8 + 8 + chain_total + 50;
    std::cout << "\n   TOTAL: " << total_pass << "/" << total_tests << "\n";
    std::cout << "   SUCCESS RATE: " << std::fixed << std::setprecision(2) 
              << (100.0 * total_pass / total_tests) << "%\n";

    return 0;
}
