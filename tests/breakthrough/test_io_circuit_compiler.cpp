#include <iostream>
#include <iomanip>
#include <cmath>
#include <sys/time.h>
#include <cstdlib>
#include <random>
#include <vector>
#include <map>
#include <string>
#include "openfhe.h"
using namespace lbcrypto;

const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;
const double PSI = (1.0 - std::sqrt(5.0)) / 2.0;

struct DualGate { Ciphertext<DCRTPoly> a; Ciphertext<DCRTPoly> b; };
struct SecureContext { CryptoContext<DCRTPoly> cc; KeyPair<DCRTPoly> kp; };

SecureContext create_context(uint32_t ring_dim, uint32_t depth, uint32_t batch_size) {
    CCParams<CryptoContextCKKSRNS> p;
    p.SetMultiplicativeDepth(depth);
    p.SetScalingModSize(50);
    p.SetBatchSize(batch_size);
    p.SetRingDim(ring_dim);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);
    return {cc, kp};
}

DualGate encrypt_input(SecureContext& sc, double val) {
    return {sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{val})),
            sc.cc->Encrypt(sc.kp.publicKey, sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{0.0}))};
}

double decrypt_value(SecureContext& sc, const Ciphertext<DCRTPoly>& c) {
    Plaintext pt; sc.cc->Decrypt(sc.kp.secretKey, c, &pt);
    return pt->GetCKKSPackedValue()[0].real();
}

double reveal(DualGate& dg, SecureContext& sc, double root) {
    return decrypt_value(sc, dg.a) + decrypt_value(sc, dg.b) * root;
}

double purify(double val, double threshold) { 
    return (val > threshold) ? 1.0 : 0.0; 
}

DualGate gate_not(SecureContext& sc, DualGate& X) {
    auto one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{1.0});
    auto neg_one = sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0});
    return {sc.cc->EvalSub(one, X.a), sc.cc->EvalMult(neg_one, X.b)};
}

DualGate gate_nand(SecureContext& sc, DualGate& X, DualGate& Y) {
    auto a_out = sc.cc->EvalMult(X.a, Y.a);
    auto sum = sc.cc->EvalAdd(
        sc.cc->EvalAdd(sc.cc->EvalMult(X.a, Y.b), sc.cc->EvalMult(X.b, Y.a)), 
        sc.cc->EvalMult(X.b, Y.b));
    DualGate and_result = {a_out, sc.cc->EvalMult(
        sc.cc->MakeCKKSPackedPlaintext(std::vector<double>{-1.0}), sum)};
    return gate_not(sc, and_result);
}

DualGate gate_and(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate nand_result = gate_nand(sc, X, Y);
    return gate_not(sc, nand_result);
}

DualGate gate_or(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate not_x = gate_not(sc, X);
    DualGate not_y = gate_not(sc, Y);
    return gate_nand(sc, not_x, not_y);
}

DualGate gate_xor(SecureContext& sc, DualGate& X, DualGate& Y) {
    DualGate nand_xy = gate_nand(sc, X, Y);
    DualGate or_xy = gate_or(sc, X, Y);
    return gate_and(sc, or_xy, nand_xy);
}

struct CircuitNode {
    int id;
    int gate_type;
    std::vector<int> inputs;
    std::vector<int> outputs;
    int state_register;
};

struct CompiledCircuit {
    std::vector<CircuitNode> nodes;
    int num_registers;
    int num_inputs;
    int num_outputs;
};

CompiledCircuit generate_circuit(int num_gates, int num_registers, int num_inputs) {
    CompiledCircuit circuit;
    circuit.num_registers = num_registers;
    circuit.num_inputs = num_inputs;
    circuit.num_outputs = 2;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> gate_dist(0, 3);
    std::uniform_int_distribution<int> wire_dist(0, 1);
    
    for (int i = 0; i < num_inputs; i++) {
        CircuitNode node;
        node.id = i;
        node.gate_type = -1;
        node.state_register = i;
        circuit.nodes.push_back(node);
    }
    
    for (int i = num_inputs; i < num_inputs + num_registers; i++) {
        CircuitNode node;
        node.id = i;
        node.gate_type = -1;
        node.state_register = i;
        circuit.nodes.push_back(node);
    }
    
    for (int i = num_inputs + num_registers; i < num_inputs + num_registers + num_gates; i++) {
        CircuitNode node;
        node.id = i;
        node.gate_type = gate_dist(gen);
        
        int num_input_nodes = num_inputs + num_registers + (i - num_inputs - num_registers);
        if (num_input_nodes >= 2) {
            std::uniform_int_distribution<int> input_dist(0, num_input_nodes - 1);
            node.inputs.push_back(input_dist(gen));
            int in2 = input_dist(gen);
            if (node.gate_type != 2) {
                while (in2 == node.inputs[0] && num_input_nodes > 1) {
                    in2 = input_dist(gen);
                }
                node.inputs.push_back(in2);
            }
        }
        
        if (i < num_inputs + num_registers + num_gates - 2) {
            node.outputs.push_back(i + 1);
            if (wire_dist(gen) && i + 2 < num_inputs + num_registers + num_gates) {
                node.outputs.push_back(i + 2);
            }
        }
        
        node.state_register = -1;
        circuit.nodes.push_back(node);
    }
    
    return circuit;
}

int main(int argc, char* argv[]) {
    uint32_t ring_dim = (argc > 1) ? std::atoi(argv[1]) : 2048;
    uint32_t depth = (argc > 2) ? std::atoi(argv[2]) : 30;
    uint32_t batch_size = (argc > 3) ? std::atoi(argv[3]) : 256;
    int num_gates = (argc > 4) ? std::atoi(argv[4]) : 100;
    int num_registers = (argc > 5) ? std::atoi(argv[5]) : 4;
    int num_inputs = (argc > 6) ? std::atoi(argv[6]) : 2;
    int num_realities = (argc > 7) ? std::atoi(argv[7]) : 8;
    int chain_length = (argc > 8) ? std::atoi(argv[8]) : 500;
    double threshold = (argc > 9) ? std::atof(argv[9]) : 0.5;
    int report_interval = (argc > 10) ? std::atoi(argv[10]) : 50;
    
    if (num_realities < 2) num_realities = 2;
    if (num_realities > 16) num_realities = 16;
    
    typedef DualGate (*GateFunc)(SecureContext&, DualGate&, DualGate&);
    GateFunc gate_funcs[4] = {gate_and, gate_or, gate_nand, gate_xor};
    std::string gate_names[4] = {"AND", "OR", "NAND", "XOR"};
    
    CompiledCircuit circuit = generate_circuit(num_gates, num_registers, num_inputs);
    
    time_t st = time(0);
    std::cout << "\n";
    std::cout << "  Circuit Compiler\n";
    std::cout << "  Gates: " << num_gates << ", Registers: " << num_registers;
    std::cout << ", Inputs: " << num_inputs << ", Realities: " << num_realities << "\n";
    std::cout << "  Nodes: " << circuit.nodes.size() << "\n\n";
    
    for (const auto& node : circuit.nodes) {
        if (node.gate_type >= 0) {
            std::cout << "  Node " << node.id << ": " << gate_names[node.gate_type];
            std::cout << " (inputs: ";
            for (int in : node.inputs) std::cout << in << " ";
            std::cout << ", outputs: ";
            for (int out : node.outputs) std::cout << out << " ";
            std::cout << ")\n";
        } else {
            std::cout << "  Node " << node.id << ": REGISTER " << node.state_register << "\n";
        }
    }
    
    std::cout << "\n  Started: " << ctime(&st) << "\n";

    time_t chain_start = time(0);
    SecureContext ctx = create_context(ring_dim, depth, batch_size);
    
    std::vector<double> registers(circuit.num_registers + circuit.num_inputs, 1.0);
    std::vector<double> reality_outputs(num_realities);
    
    for (int step = 0; step < chain_length; step++) {
        SecureContext next_ctx = create_context(ring_dim, depth, batch_size);
        
        std::vector<DualGate> node_outputs(circuit.nodes.size());
        std::vector<double> node_values(circuit.nodes.size());
        std::map<int, double> wire_values;
        
        for (int i = 0; i < circuit.num_inputs + circuit.num_registers; i++) {
            node_values[i] = registers[i];
        }
        
        for (const auto& node : circuit.nodes) {
            if (node.gate_type < 0) continue;
            
            if (node.inputs.size() >= (node.gate_type == 2 ? 1 : 2)) {
                double in1 = node_values[node.inputs[0]];
                double in2 = (node.gate_type == 2) ? 0.0 : node_values[node.inputs[1]];
                
                int gate_idx = (node.gate_type + step) % 4;
                
                SecureContext gate_ctx = create_context(ring_dim, depth, batch_size);
                DualGate X = encrypt_input(gate_ctx, in1);
                DualGate Y = encrypt_input(gate_ctx, in2);
                DualGate result = gate_funcs[gate_idx](gate_ctx, X, Y);
                
                double out_val = reveal(result, gate_ctx, (step % 2 == 0) ? PHI : PSI);
                out_val = purify(out_val, threshold);
                
                node_values[node.id] = out_val;
                wire_values[node.id] = out_val;
            }
        }
        
        for (int r = 0; r < num_realities; r++) {
            double sum = 0.0;
            int count = 0;
            for (const auto& node : circuit.nodes) {
                if (node.gate_type >= 0 && node.outputs.empty()) {
                    sum += node_values[node.id];
                    count++;
                }
            }
            reality_outputs[r] = (count > 0) ? sum / count : registers[0];
        }
        
        double mixed = 0.0;
        for (int r = 0; r < num_realities; r++) {
            mixed += reality_outputs[r];
        }
        mixed /= num_realities;
        double purified = purify(mixed, threshold);
        
        for (int i = 0; i < circuit.num_registers; i++) {
            registers[circuit.num_inputs + i] = purified;
        }
        
        if (step % report_interval == 0 || step == chain_length - 1) {
            time_t now = time(0);
            double elapsed = difftime(now, chain_start);
            double progress = (double)(step+1) / chain_length * 100.0;
            double rate = (double)(step+1) / elapsed;
            double eta = (chain_length - step - 1) / rate;
            
            std::cout << "  [" << std::setw(4) << (step+1) << "/" << chain_length 
                      << " " << std::fixed << std::setprecision(1) << std::setw(5) << progress << "%]"
                      << " | Val: " << purified
                      << " | " << std::setprecision(1) << rate << " g/s"
                      << " | ETA: " << std::setprecision(0) << eta << "s"
                      << "                \r" << std::flush;
        }
        
        ctx = next_ctx;
    }
    
    time_t chain_end = time(0);
    double total_time = difftime(chain_end, chain_start);
    
    std::cout << "\n\n";
    std::cout << "  Complete\n";
    std::cout << "  Nodes: " << circuit.nodes.size() << "\n";
    std::cout << "  Gates: " << num_gates << "\n";
    std::cout << "  Registers: " << num_registers << "\n";
    std::cout << "  Realities: " << num_realities << "\n";
    std::cout << "  Chain length: " << chain_length << "\n";
    std::cout << "  Time: " << std::setprecision(0) << total_time << "s (" << total_time/60 << " min)\n";
    std::cout << "  Rate: " << std::setprecision(1) << chain_length/total_time << " steps/sec\n";
    std::cout << "\n  Ended: " << ctime(&chain_end) << "\n";
    
    return 0;
}
