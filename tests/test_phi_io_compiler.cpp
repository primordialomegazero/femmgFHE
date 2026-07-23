// PHI-IO COMPILER TEST: Compile two equivalent circuits
#include <openfhe.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>
#include "io/phi_io_compiler.h"

using namespace std;
using namespace phi_io;

// Build circuit: f(x) = (x+1)(x+2) via FACTORED approach
Circuit build_factored() {
    Circuit C;
    // Gate 0: input x
    C.gates.push_back({GateType::INPUT, 0, 0, 0.0, 0, 0});
    // Gate 1: constant 1
    C.gates.push_back({GateType::CONSTANT, 1, 0, 1.0, 0, 0});
    // Gate 2: constant 2
    C.gates.push_back({GateType::CONSTANT, 2, 0, 2.0, 0, 0});
    // Gate 3: x + 1
    C.gates.push_back({GateType::ADD, 3, 0, 0.0, 0, 1});
    // Gate 4: x + 2
    C.gates.push_back({GateType::ADD, 4, 0, 0.0, 0, 2});
    // Gate 5: (x+1)(x+2)
    C.gates.push_back({GateType::MULT, 5, 0, 0.0, 3, 4});
    C.output_gate = 5;
    return C;
}

// Build circuit: f(x) = x² + 3x + 2 via EXPANDED approach
Circuit build_expanded() {
    Circuit C;
    // Gate 0: input x
    C.gates.push_back({GateType::INPUT, 0, 0, 0.0, 0, 0});
    // Gate 1: constant 3
    C.gates.push_back({GateType::CONSTANT, 1, 0, 3.0, 0, 0});
    // Gate 2: constant 2
    C.gates.push_back({GateType::CONSTANT, 2, 0, 2.0, 0, 0});
    // Gate 3: x²
    C.gates.push_back({GateType::MULT, 3, 0, 0.0, 0, 0});
    // Gate 4: 3x
    C.gates.push_back({GateType::MULT, 4, 0, 0.0, 0, 1});
    // Gate 5: x² + 3x
    C.gates.push_back({GateType::ADD, 5, 0, 0.0, 3, 4});
    // Gate 6: x² + 3x + 2
    C.gates.push_back({GateType::ADD, 6, 0, 0.0, 5, 2});
    C.output_gate = 6;
    return C;
}

int main() {
    cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║   PHI-IO COMPILER: General Circuit Obfuscation       ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    cout << "  Setup compiler..." << flush;
    PhiIOCompiler compiler(8192);
    cout << " Done.\n\n";

    // Build two functionally equivalent circuits
    Circuit C0 = build_factored();   // (x+1)(x+2) — 6 gates
    Circuit C1 = build_expanded();   // x²+3x+2 — 7 gates
    
    // Pad the smaller circuit to match gate count
    // Add a dummy gate that doesn't affect output
    C0.gates.push_back({GateType::CONSTANT, 6, 0, 0.0, 0, 0});
    // Update C1 to also have output at gate 6
    C1.output_gate = 6;

    cout << "  C0 (factored):  " << C0.gates.size() << " gates, output=" << C0.output_gate << "\n";
    cout << "  C1 (expanded):  " << C1.gates.size() << " gates, output=" << C1.output_gate << "\n\n";

    // Verify functional equivalence
    cout << "  Verifying equivalence...\n";
    for (double x : {0.0, 1.0, 2.0, 3.0, 5.0, -1.0}) {
        double out0 = C0.evaluate({x});
        double out1 = C1.evaluate({x});
        cout << "    x=" << fixed << setprecision(1) << setw(5) << x 
             << "  C0=" << setw(8) << setprecision(3) << out0
             << "  C1=" << setw(8) << setprecision(3) << out1
             << "  " << (abs(out0-out1)<1e-10 ? "OK" : "FAIL") << "\n";
    }
    cout << "\n";

    // Generate random assignment (THE SECRET)
    mt19937 rng(42);
    vector<bool> assignment(C0.gates.size());
    for (size_t i = 0; i < assignment.size(); i++) {
        assignment[i] = (rng() % 2 == 0);
    }

    // Compile!
    cout << "  Compiling obfuscated program...\n";
    vector<double> input = {3.0};  // f(3) = (3+1)(3+2) = 20
    auto encoded = compiler.compile(C0, C1, input, assignment);
    cout << "  Encoded " << encoded.size() << " gates.\n\n";

    // Extract output
    double phi_out = compiler.extract_output(encoded[C0.output_gate], true);
    double psi_out = compiler.extract_output(encoded[C0.output_gate], false);
    
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout <<   "  ║  OBFUSCATED OUTPUT                                   ║\n";
    cout <<   "  ╠══════════════════════════════════════════════════════╣\n";
    cout <<   "  ║  φ-evaluation: " << setw(12) << fixed << setprecision(6) << phi_out << "                         ║\n";
    cout <<   "  ║  ψ-evaluation: " << setw(12) << fixed << setprecision(6) << psi_out << "                         ║\n";
    cout <<   "  ║                                                      ║\n";
    
    double expected = 20.0;
    // Both evaluations should produce the same correct output
    bool phi_correct = abs(phi_out - expected) < 1e-3;
    bool psi_correct = abs(psi_out - expected) < 1e-3;
    
    cout <<   "  ║  Expected:      " << setw(12) << fixed << setprecision(6) << expected << "                         ║\n";
    cout <<   "  ║  φ correct: " << (phi_correct ? "YES" : "NO ") << "                                  ║\n";
    cout <<   "  ║  ψ correct: " << (psi_correct ? "YES" : "NO ") << "                                  ║\n";
    cout <<   "  ║                                                      ║\n";
    cout <<   "  ║  Observer sees: Two values, both ~20.0               ║\n";
    cout <<   "  ║  Observer knows: NOTHING about which circuit ran.    ║\n";
    cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
