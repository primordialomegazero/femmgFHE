#include "security_complete_fixed.h"
#include <iostream>

int main() {
    std::cout << "Perturbation Test\n";
    std::cout << "==================\n\n";

    // Generate 3 different seeds
    security::PerturbationSeed s1;
    security::PerturbationSeed s2;
    security::PerturbationSeed s3;

    // Check if seeds are different
    auto seed1 = s1.get_seed();
    auto seed2 = s2.get_seed();
    auto seed3 = s3.get_seed();

    std::cout << "Seed 1: " << seed1.to_hex().substr(0, 16) << "...\n";
    std::cout << "Seed 2: " << seed2.to_hex().substr(0, 16) << "...\n";
    std::cout << "Seed 3: " << seed3.to_hex().substr(0, 16) << "...\n";
    std::cout << "s1 != s2: " << (seed1 != seed2 ? "✅" : "❌") << "\n";
    std::cout << "s2 != s3: " << (seed2 != seed3 ? "✅" : "❌") << "\n\n";

    // Get perturbations for each
    double p1 = s1.get_perturbation(0, 0, 0);
    double p2 = s2.get_perturbation(0, 0, 0);
    double p3 = s3.get_perturbation(0, 0, 0);

    std::cout << "Perturbation 1: " << p1 << "\n";
    std::cout << "Perturbation 2: " << p2 << "\n";
    std::cout << "Perturbation 3: " << p3 << "\n";
    std::cout << "p1 != p2: " << (p1 != p2 ? "✅" : "❌") << "\n";
    std::cout << "p2 != p3: " << (p2 != p3 ? "✅" : "❌") << "\n";

    // Check if perturbation depends on dim, layer, party
    double p00 = s1.get_perturbation(0, 0, 0);
    double p01 = s1.get_perturbation(0, 0, 1);
    double p10 = s1.get_perturbation(0, 1, 0);
    double p11 = s1.get_perturbation(1, 0, 0);

    std::cout << "\np(0,0,0): " << p00 << "\n";
    std::cout << "p(0,0,1): " << p01 << "\n";
    std::cout << "p(0,1,0): " << p10 << "\n";
    std::cout << "p(1,0,0): " << p11 << "\n";
    std::cout << "Different dimensions: " << (p00 != p01 && p00 != p10 && p00 != p11 ? "✅" : "❌") << "\n";

    return 0;
}
