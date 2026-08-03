#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include <vector>

const double PHI = 1.6180339887498948482;
const double PSI = -0.6180339887498948482;

double mirror_collapse(double value, bool was_phi_path) {
    return std::abs(was_phi_path ? value * PSI : value * PHI);
}

int main() {
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> val(0.1, 0.9);
    
    auto nand_gate = [](double a, double b) -> double {
        return 1.0 - std::min(1.0, std::max(0.0, a)) * std::min(1.0, std::max(0.0, b));
    };
    
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "=== DEEP DEBUG: Circuit Structure Retention ===\n\n";
    
    for (int i = 0; i < 20; i++) {
        double x = val(gen), y = val(gen), z = val(gen);
        
        // Circuit A: (X NAND Y) NAND (Y NAND Z)
        double nand_xy_A = nand_gate(x, y);
        double nand_yz_A = nand_gate(y, z);
        double final_A_raw = nand_gate(nand_xy_A, nand_yz_A);
        double out_A_encoded = final_A_raw > 0.5 ? PHI : PSI;
        double out_A_canon = mirror_collapse(out_A_encoded, out_A_encoded > 0);
        
        // Circuit B: (X NAND Z) NAND (X NAND Y)
        double nand_xz_B = nand_gate(x, z);
        double nand_xy_B = nand_gate(x, y);
        double final_B_raw = nand_gate(nand_xz_B, nand_xy_B);
        double out_B_encoded = final_B_raw > 0.5 ? PHI : PSI;
        double out_B_canon = mirror_collapse(out_B_encoded, out_B_encoded > 0);
        
        std::cout << "Case " << i << ": x=" << x << " y=" << y << " z=" << z << "\n";
        std::cout << "  A: nand_xy=" << nand_xy_A << " nand_yz=" << nand_yz_A 
                  << " final_raw=" << final_A_raw << " canon=" << out_A_canon << "\n";
        std::cout << "  B: nand_xz=" << nand_xz_B << " nand_xy=" << nand_xy_B 
                  << " final_raw=" << final_B_raw << " canon=" << out_B_canon << "\n";
        
        bool final_same = (std::abs(out_A_canon - out_B_canon) < 1e-10);
        bool inter_diff = (std::abs(nand_xy_A - nand_xz_B) > 0.05 || 
                          std::abs(nand_yz_A - nand_xy_B) > 0.05 ||
                          std::abs(final_A_raw - final_B_raw) > 0.05);
        
        std::cout << "  final_same=" << (final_same ? "YES" : "NO") 
                  << " inter_diff=" << (inter_diff ? "YES" : "NO") << "\n\n";
    }
    
    std::cout << "=== ANALYSIS ===\n";
    std::cout << "The issue: NAND gates on continuous [0,1] values produce\n";
    std::cout << "outputs that are ALL > 0.5 for most inputs (0.1-0.9 range).\n";
    std::cout << "Since both circuits use NAND, the internal values converge\n";
    std::cout << "to similar ranges, making intermediates less distinct.\n\n";
    std::cout << "FIX: Use MORE DIVERSE gate types (AND, OR, XOR) expressed as\n";
    std::cout << "NAND combinations to create truly different internal paths.\n";
    
    return 0;
}
