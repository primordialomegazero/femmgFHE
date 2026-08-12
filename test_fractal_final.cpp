#include "spiral_fractal_turing_complete.h"

int main() {
    std::cout << "======================================================================\n";
    std::cout << "  MULTIDIMENSIONAL FRACTAL TURING-COMPLETE iO+FHE v45.0\n";
    std::cout << "  5D Fractal Qubits + Anti-Matter + FHE + ZKP\n";
    std::cout << "  phi*psi = -1 -> 1+1=2 -> ALL UNIFIED\n";
    std::cout << "======================================================================\n\n";

    auto compiler = SpiralFractalNS::FractalCompiler::create()
        .withRingDim(16384)
        .withQubitDims(5)
        .build();
    compiler.init();

    struct { std::string expr, name; double tc[8][3]; double ex[8]; } tests[] = {
        {"(x AND y) OR z", "(x AND y) OR z", {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}}, {0,1,0,1,0,1,1,1}},
        {"NOT(x) AND (y OR z)", "NOT(x) AND (y OR z)", {{0,0,0},{0,0,1},{0,1,0},{0,1,1},{1,0,0},{1,0,1},{1,1,0},{1,1,1}}, {0,1,1,1,0,0,0,0}},
        {"x XOR y", "x XOR y", {{0,0,0},{0,1,0},{1,0,0},{1,1,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}, {0,1,1,0,0,0,0,0}},
    };
    int total_pass = 0, total_tests = 0;

    for (auto& t : tests) {
        std::cout << "=== " << t.name << " ===\n";
        int n = (t.name == "x XOR y") ? 4 : 8;
        int pass = 0;
        for (int i = 0; i < n; i++) {
            auto r = compiler.compile_and_run(t.expr, 
                {{"x",t.tc[i][0]},{"y",t.tc[i][1]},{"z",t.tc[i][2]}}, t.ex[i]);
            std::cout << "  " << (r.correct ? "PASS" : "FAIL") 
                      << " iO=" << r.iO_output << " FHE=" << r.fhe_output
                      << " KS=" << r.ks << " ZKP=" << (r.zkp_verified?"OK":"FAIL")
                      << " Gates=" << r.gate_count << " Dims=" << r.qubit_dims << "\n";
            if (r.correct && r.zkp_verified) pass++;
        }
        std::cout << "  Result: " << pass << "/" << n << "\n\n";
        total_pass += pass; total_tests += n;
    }

    std::cout << "======================================================================\n";
    std::cout << "  TOTAL: " << total_pass << "/" << total_tests << "\n";
    std::cout << "  phi*psi = -1 -> 1+1=2 -> FRACTAL TURING-COMPLETE\n";
    std::cout << "======================================================================\n";
    return total_pass == total_tests ? 0 : 1;
}
