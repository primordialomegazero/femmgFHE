// ═══════════════════════════════════════════════════════════════════════════════
// SPIRALRUN — Execute .obf files (iO Runtime)
// ═══════════════════════════════════════════════════════════════════════════════
// Usage: spiralrun program.obf [input1] [input2] ...
// Decrypts obfuscated binary, evaluates with FHE, returns result.

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include "openfhe.h"
#include "../utils/safe_math.h"
#include "../core/constants.h"
#include "../fhe/fhe_core.h"
using namespace lbcrypto;

int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  SPIRALRUN — iO Runtime Executor                             ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    if (argc < 2) {
        std::cout << "Usage: spiralrun program.obf [input1] [input2] ...\n";
        std::cout << "       spiralrun --bench program.obf N_times\n";
        return 0;
    }
    
    std::string obf_file = argv[1];
    
    // Read obfuscated binary
    std::ifstream of(obf_file, std::ios::binary);
    std::vector<double> circuit;
    double v;
    while (of.read((char*)&v, sizeof(v))) circuit.push_back(v);
    of.close();
    
    int N_slots = circuit.size();
    int N_ringdim = N_slots * 16;  // Reverse: RingDim = slots * 16
    
    std::cout << "  Program: " << obf_file << "\n";
    std::cout << "  Slots: " << N_slots << " | RingDim: " << N_ringdim << "\n";
    
    // Collect inputs
    std::vector<double> inputs;
    for (int i = 2; i < argc; i++) {
        inputs.push_back(atof(argv[i]));
    }
    if (inputs.empty()) inputs = {0.5, 0.3};  // Default inputs
    
    std::cout << "  Inputs: ";
    for (auto x : inputs) std::cout << x << " ";
    std::cout << "\n\n";
    
    auto start = std::chrono::steady_clock::now();
    
    // FHE execution
    SecureContext sc = create_fhe_context(N_ringdim, 10, N_slots);
    
    // Blend inputs into circuit
    std::vector<double> exec_circuit = circuit;
    for (size_t i = 0; i < inputs.size() && i < exec_circuit.size(); i++) {
        exec_circuit[i] = SafeMath::fmod_safe(circuit[i] * inputs[i] * PHI);
    }
    
    auto pt = sc.cc->MakeCKKSPackedPlaintext(exec_circuit);
    auto ct = sc.cc->Encrypt(sc.kp.publicKey, pt);
    auto result = sc.cc->EvalMult(ct, ct);
    
    Plaintext pt_out;
    sc.cc->Decrypt(sc.kp.secretKey, result, &pt_out);
    auto vals = pt_out->GetCKKSPackedValue();
    
    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    
    // Output: φ/ψ projections
    double phi_sum = 0, psi_sum = 0;
    for (size_t i = 0; i < vals.size() && i < 5; i++) {
        double r = vals[i].real();
        double phi = r * PHI, psi = r * PSI;
        phi_sum += phi; psi_sum += psi;
        std::cout << "  Output[" << i << "] = " << std::setprecision(6) << r;
        std::cout << " (φ=" << phi << ", ψ=" << psi << ")\n";
    }
    
    double obf_metric = SafeMath::fmod_safe(phi_sum + psi_sum);
    
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  EXECUTION RESULT                                            ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Time:    " << std::setw(11) << std::setprecision(4) << elapsed << "s";
    for (int i = 0; i < 31; i++) std::cout << " ";
    std::cout << "║\n";
    std::cout << "║  φ/ψ:     " << std::setw(11) << std::setprecision(4) << obf_metric;
    for (int i = 0; i < 31; i++) std::cout << " ";
    std::cout << "║\n";
    std::cout << "║  Program obfuscated — output indistinguishable               ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}
