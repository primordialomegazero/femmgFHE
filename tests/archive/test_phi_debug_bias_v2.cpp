// Debug v2: Print actual FHE raw values for add-then-mul
#include <iostream>
#include <iomanip>
#include <cmath>

int main() {
    const double PSI = 0.6180339887498949;
    const double PSI2 = PSI * PSI;

    std::cout << "=== ADD-THEN-MUL: RAW VALUE ANALYSIS ===\n\n";
    std::cout << std::fixed << std::setprecision(10);

    struct TestCase {
        const char* name;
        double sum_v;
        double vC;
        int N;
        double raw_fhe;
    };

    TestCase tests[] = {
        {"(A+B)xC",    0.8, 0.2, 1, 1.0475388202},
        {"(A+B+E)xC",  1.8, 0.2, 2, 2.3711456180},
        {"(5xA)xC",    2.5, 0.2, 4, 3.9549150281},
    };

    for (const auto& t : tests) {
        double expected = t.sum_v * t.vC;
        double num_after_add = t.sum_v + t.N * PSI;
        double ratio_after_mul = num_after_add * (t.vC + PSI);
        double raw_expected = ratio_after_mul - PSI;
        double diff = t.raw_fhe - raw_expected;

        std::cout << t.name << " (N=" << t.N << "):\n";
        std::cout << "  sum_v=" << t.sum_v << ", vC=" << t.vC << "\n";
        std::cout << "  raw_expected = " << raw_expected << "\n";
        std::cout << "  raw_fhe      = " << t.raw_fhe << "\n";
        std::cout << "  diff         = " << diff << "\n";
        std::cout << "  diff/PSI     = " << diff/PSI << "\n";
        std::cout << "  diff/PSI2    = " << diff/PSI2 << "\n\n";
    }

    std::cout << "=== CORRECTION NEEDED ===\n\n";
    for (const auto& t : tests) {
        double expected = t.sum_v * t.vC;
        double correction_needed = t.raw_fhe - expected;
        double old_correction = t.sum_v*PSI + t.N*PSI*t.vC + t.N*PSI2 - PSI;
        double missing = correction_needed - old_correction;

        std::cout << t.name << ":\n";
        std::cout << "  correction_needed = " << correction_needed << "\n";
        std::cout << "  old_correction    = " << old_correction << "\n";
        std::cout << "  missing           = " << missing << "\n\n";
    }

    return 0;
}
