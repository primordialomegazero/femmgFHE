// ΦΩ0 — ARBITRARY CIRCUIT ENGINE v1.2
// Earth OS Patch v3: ACTUAL NOISE-BASED Adaptive Divine
// Uses GetNoiseScaleDeg() for real noise measurement, not theoretical estimates
// "THE CIRCUIT IS FREE. EVERY WIRE. EVERY GATE. NO LIMITS."
// "I AM THAT I AM"

#ifndef PHI_ARBITRARY_CIRCUIT_H
#define PHI_ARBITRARY_CIRCUIT_H

#include <openfhe.h>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include "zans_production_lib.h"

using namespace lbcrypto;
using namespace std;

enum class GateType {
    INPUT, CONSTANT, ADD, MUL, MUL_SCALAR, MUL_CONST, SUB, NEG, OUTPUT
};

string gate_type_str(GateType g) {
    switch(g) {
        case GateType::INPUT:      return "INPUT";
        case GateType::CONSTANT:   return "CONST";
        case GateType::ADD:        return "ADD";
        case GateType::MUL:        return "MUL";
        case GateType::MUL_SCALAR: return "MUL_SC";
        case GateType::MUL_CONST:  return "MUL_CT";
        case GateType::SUB:        return "SUB";
        case GateType::NEG:        return "NEG";
        case GateType::OUTPUT:     return "OUT";
        default:                   return "???";
    }
}

struct CircuitNode {
    int id;
    GateType type;
    string name;
    int64_t value;
    vector<int> inputs;
    Ciphertext<DCRTPoly> ct;
    double actual_noise;
    int divine_count;
    int zans_count;
    int64_t plaintext_result;
    bool verified;
    
    CircuitNode() : id(-1), type(GateType::INPUT), value(0), 
                    actual_noise(0), divine_count(0), zans_count(0),
                    plaintext_result(0), verified(false) {}
};

class TopologicalSorter {
public:
    static vector<int> sort(const vector<CircuitNode>& nodes) {
        int n = nodes.size();
        map<int, vector<int>> adjacency;
        map<int, int> in_degree;
        for (const auto& node : nodes) {
            if (in_degree.find(node.id) == in_degree.end()) in_degree[node.id] = 0;
            adjacency[node.id] = {};
        }
        for (const auto& node : nodes) {
            for (int input_id : node.inputs) {
                adjacency[input_id].push_back(node.id);
                in_degree[node.id]++;
            }
        }
        queue<int> q;
        for (const auto& [id, deg] : in_degree) if (deg == 0) q.push(id);
        vector<int> order;
        while (!q.empty()) {
            int current = q.front(); q.pop();
            order.push_back(current);
            for (int child : adjacency[current]) {
                in_degree[child]--;
                if (in_degree[child] == 0) q.push(child);
            }
        }
        if (order.size() != nodes.size()) throw runtime_error("CIRCUIT HAS A CYCLE!");
        return order;
    }
};

class ArbitraryCircuitEngine {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    ZANSAnchorPool pool;
    Ciphertext<DCRTPoly> anchor0;
    Ciphertext<DCRTPoly> M;
    int64_t modulus;
    int64_t half_mod;
    
    map<int, CircuitNode> node_map;
    vector<int> execution_order;
    
    int total_gates;
    int divine_injections;
    int zans_applications;
    int pinky_swears;
    
    // ACTUAL noise thresholds
    static constexpr double NOISE_CRITICAL = 30.0;
    static constexpr double NOISE_HIGH = 15.0;
    static constexpr double NOISE_ELEVATED = 8.0;
    static constexpr int MAX_DIVINE_ROUNDS = 20;
    static constexpr int MAX_ZANS_ROUNDS = 10;
    
public:
    ArbitraryCircuitEngine(CryptoContext<DCRTPoly> ctx, KeyPair<DCRTPoly> kp,
                           int64_t mod, int pool_size = 10)
        : cc(ctx), keys(kp), pool(ctx, kp, pool_size), modulus(mod), 
          half_mod(mod / 2), total_gates(0), divine_injections(0),
          zans_applications(0), pinky_swears(0) {
        vector<int64_t> zv = {0};
        anchor0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(zv));
        vector<int64_t> mv = {half_mod};
        M = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext(mv));
    }
    
    int64_t mod_pos(int64_t v) { return ((v % modulus) + modulus) % modulus; }
    
    double measure_actual_noise(const Ciphertext<DCRTPoly>& ct) {
        return ct->GetNoiseScaleDeg();
    }
    
    // ==========================================
    // ADAPTIVE DIVINE v3 — ACTUAL NOISE BASED
    // ==========================================
    
    Ciphertext<DCRTPoly> adaptive_divine_actual(const Ciphertext<DCRTPoly>& ct_in,
                                                  const Ciphertext<DCRTPoly>& overflow_in,
                                                  int& divine_used, int& zans_used) {
        auto result = ct_in;
        auto overflow = overflow_in;
        divine_used = 0;
        zans_used = 0;
        
        double noise = measure_actual_noise(result);
        int rounds = 0;
        
        while (noise > NOISE_CRITICAL && rounds < MAX_DIVINE_ROUNDS) {
            // Divine + heavy ZANS
            for (int d = 0; d < 3 && noise > NOISE_CRITICAL && rounds < MAX_DIVINE_ROUNDS; d++) {
                divine_injections++;
                divine_used++;
                auto divine = cc->EvalMult(overflow, anchor0);
                result = cc->EvalAdd(result, divine);
                result = cc->EvalAdd(result, anchor0);
                rounds++;
            }
            
            // Multi-round ZANS
            for (int z = 0; z < 5 && rounds < MAX_ZANS_ROUNDS; z++) {
                zans_applications++;
                zans_used++;
                result = pool.stabilize(result);
            }
            
            // Re-pinky for next iteration
            if (rounds < MAX_DIVINE_ROUNDS) {
                auto sum = cc->EvalAdd(result, M);
                auto back = cc->EvalSub(sum, M);
                overflow = cc->EvalSub(result, back);
                pinky_swears++;
            }
            
            noise = measure_actual_noise(result);
        }
        
        // Medium noise: 2 divine + 3 zans
        while (noise > NOISE_HIGH && rounds < MAX_DIVINE_ROUNDS) {
            for (int d = 0; d < 2 && noise > NOISE_HIGH; d++) {
                divine_injections++;
                divine_used++;
                auto divine = cc->EvalMult(overflow, anchor0);
                result = cc->EvalAdd(result, divine);
                result = cc->EvalAdd(result, anchor0);
                rounds++;
            }
            for (int z = 0; z < 3; z++) {
                zans_applications++;
                zans_used++;
                result = pool.stabilize(result);
            }
            if (rounds < MAX_DIVINE_ROUNDS) {
                auto sum = cc->EvalAdd(result, M);
                auto back = cc->EvalSub(sum, M);
                overflow = cc->EvalSub(result, back);
                pinky_swears++;
            }
            noise = measure_actual_noise(result);
        }
        
        // Elevated noise: 1 divine + 2 zans
        if (noise > NOISE_ELEVATED) {
            divine_injections++;
            divine_used++;
            auto divine = cc->EvalMult(overflow, anchor0);
            result = cc->EvalAdd(result, divine);
            result = cc->EvalAdd(result, anchor0);
            for (int z = 0; z < 2; z++) {
                zans_applications++;
                zans_used++;
                result = pool.stabilize(result);
            }
        }
        
        // Always at least 1 ZANS
        if (zans_used == 0) {
            zans_applications++;
            zans_used++;
            result = pool.stabilize(result);
        }
        
        return result;
    }
    
    Ciphertext<DCRTPoly> pinky_swear(const Ciphertext<DCRTPoly>& ct) {
        pinky_swears++;
        auto sum = cc->EvalAdd(ct, M);
        auto back = cc->EvalSub(sum, M);
        return cc->EvalSub(ct, back);
    }
    
    // ==========================================
    // CIRCUIT CONSTRUCTION
    // ==========================================
    
    int add_input(const string& name, int64_t value) {
        CircuitNode node;
        node.id = node_map.size();
        node.type = GateType::INPUT;
        node.name = name;
        node.value = value;
        node.plaintext_result = value;
        node_map[node.id] = node;
        total_gates++;
        return node.id;
    }
    
    int add_constant(const string& name, int64_t value) {
        CircuitNode node;
        node.id = node_map.size();
        node.type = GateType::CONSTANT;
        node.name = name;
        node.value = value;
        node.plaintext_result = value;
        node_map[node.id] = node;
        total_gates++;
        return node.id;
    }
    
    int add_gate(GateType type, const string& name, int input_a, int input_b = -1, int64_t scalar = 0) {
        CircuitNode node;
        node.id = node_map.size();
        node.type = type;
        node.name = name;
        node.value = scalar;
        node.inputs.push_back(input_a);
        if (input_b >= 0) node.inputs.push_back(input_b);
        node_map[node.id] = node;
        total_gates++;
        return node.id;
    }
    
    int add_add(const string& name, int a, int b) { return add_gate(GateType::ADD, name, a, b); }
    int add_mul(const string& name, int a, int b) { return add_gate(GateType::MUL, name, a, b); }
    int add_mul_scalar(const string& name, int a, int64_t scalar) { return add_gate(GateType::MUL_SCALAR, name, a, -1, scalar); }
    int add_sub(const string& name, int a, int b) { return add_gate(GateType::SUB, name, a, b); }
    int add_neg(const string& name, int a) { return add_gate(GateType::NEG, name, a); }
    
    int64_t compute_plaintext(int node_id) {
        CircuitNode& node = node_map[node_id];
        switch(node.type) {
            case GateType::INPUT:
            case GateType::CONSTANT:
                return node.value;
            case GateType::ADD:
                return mod_pos(compute_plaintext(node.inputs[0]) + compute_plaintext(node.inputs[1]));
            case GateType::SUB:
                return mod_pos(compute_plaintext(node.inputs[0]) - compute_plaintext(node.inputs[1]));
            case GateType::NEG:
                return mod_pos(-compute_plaintext(node.inputs[0]));
            case GateType::MUL:
                return mod_pos(compute_plaintext(node.inputs[0]) * compute_plaintext(node.inputs[1]));
            case GateType::MUL_SCALAR:
                return mod_pos(compute_plaintext(node.inputs[0]) * node.value);
            default:
                return 0;
        }
    }
    
    // ==========================================
    // EXECUTION v1.2 — Actual noise measurement
    // ==========================================
    
    void execute() {
        vector<CircuitNode> node_list;
        for (auto& [id, node] : node_map) node_list.push_back(node);
        execution_order = TopologicalSorter::sort(node_list);
        
        cout << "\n  ╔══════════════════════════════════════════════════╗\n";
        cout <<   "  ║   ARBITRARY CIRCUIT v1.2 (Actual Noise Adaptive)  ║\n";
        cout <<   "  ║   Gates: " << setw(4) << total_gates 
             << " | Order: " << setw(4) << execution_order.size() << "                      ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════╝\n\n";
        
        cout << "  " << setw(4) << "ID" << setw(10) << "Gate"
             << setw(14) << "Name" << setw(10) << "Noise"
             << setw(6) << "Div" << setw(6) << "ZANS"
             << setw(12) << "PText" << setw(12) << "CText" 
             << setw(8) << "Match" << endl;
        cout << "  " + string(90, '-') << endl;
        
        for (int node_id : execution_order) {
            CircuitNode& node = node_map[node_id];
            int div_used = 0, zans_used = 0;
            
            switch(node.type) {
                case GateType::INPUT:
                case GateType::CONSTANT:
                    node.ct = cc->Encrypt(keys.publicKey, 
                        cc->MakePackedPlaintext(vector<int64_t>{node.value}));
                    node.actual_noise = measure_actual_noise(node.ct);
                    break;
                
                case GateType::ADD: {
                    auto& a = node_map[node.inputs[0]];
                    auto& b = node_map[node.inputs[1]];
                    auto ov_a = pinky_swear(a.ct);
                    auto ov_b = pinky_swear(b.ct);
                    node.ct = cc->EvalAdd(a.ct, b.ct);
                    auto combined = cc->EvalAdd(ov_a, ov_b);
                    node.ct = adaptive_divine_actual(node.ct, combined, div_used, zans_used);
                    node.actual_noise = measure_actual_noise(node.ct);
                    break;
                }
                
                case GateType::MUL: {
                    auto& a = node_map[node.inputs[0]];
                    auto& b = node_map[node.inputs[1]];
                    auto ov = pinky_swear(a.ct);
                    node.ct = cc->EvalMult(a.ct, b.ct);
                    node.ct = adaptive_divine_actual(node.ct, ov, div_used, zans_used);
                    node.actual_noise = measure_actual_noise(node.ct);
                    break;
                }
                
                case GateType::MUL_SCALAR: {
                    auto& a = node_map[node.inputs[0]];
                    auto ov = pinky_swear(a.ct);
                    auto ct_s = cc->Encrypt(keys.publicKey,
                        cc->MakePackedPlaintext(vector<int64_t>{node.value}));
                    node.ct = cc->EvalMult(a.ct, ct_s);
                    node.ct = adaptive_divine_actual(node.ct, ov, div_used, zans_used);
                    node.actual_noise = measure_actual_noise(node.ct);
                    break;
                }
                
                case GateType::SUB: {
                    auto& a = node_map[node.inputs[0]];
                    auto& b = node_map[node.inputs[1]];
                    auto ov_a = pinky_swear(a.ct);
                    auto ov_b = pinky_swear(b.ct);
                    node.ct = cc->EvalSub(a.ct, b.ct);
                    auto combined = cc->EvalAdd(ov_a, ov_b);
                    node.ct = adaptive_divine_actual(node.ct, combined, div_used, zans_used);
                    node.actual_noise = measure_actual_noise(node.ct);
                    break;
                }
                
                case GateType::NEG: {
                    auto& a = node_map[node.inputs[0]];
                    node.ct = cc->EvalNegate(a.ct);
                    zans_applications++; zans_used++;
                    node.ct = pool.stabilize(node.ct);
                    node.actual_noise = measure_actual_noise(node.ct);
                    break;
                }
                
                default:
                    break;
            }
            
            node.divine_count = div_used;
            node.zans_count = zans_used;
            node.plaintext_result = compute_plaintext(node.id);
            
            Plaintext pt;
            cc->Decrypt(keys.secretKey, node.ct, &pt);
            pt->SetLength(1);
            int64_t decrypted = mod_pos(pt->GetPackedValue()[0]);
            node.verified = (decrypted == node.plaintext_result);
            
            cout << "  " << setw(4) << node.id 
                 << setw(10) << gate_type_str(node.type)
                 << setw(14) << node.name
                 << setw(10) << fixed << setprecision(1) << node.actual_noise
                 << setw(6) << node.divine_count
                 << setw(6) << node.zans_count
                 << setw(12) << node.plaintext_result
                 << setw(12) << decrypted
                 << setw(8) << (node.verified ? "OK" : "FAIL") << endl;
            
            if (!node.verified && node.type != GateType::INPUT && node.type != GateType::CONSTANT) {
                cout << "  *** FAIL @" << node.id << " " << node.name 
                     << " | Exp:" << node.plaintext_result << " Got:" << decrypted 
                     << " Noise:" << node.actual_noise << endl;
            }
        }
        
        cout << "  " + string(90, '-') << endl;
    }
    
    CircuitNode& get_node(int id) { return node_map[id]; }
    
    void print_stats() {
        int verified_count = 0, total_compute = 0;
        for (auto& [id, node] : node_map) {
            if (node.type != GateType::INPUT && node.type != GateType::CONSTANT) {
                total_compute++;
                if (node.verified) verified_count++;
            }
        }
        cout << "\n  ╔══════════════════════════════════════════════════╗\n";
        cout <<   "  ║   CIRCUIT STATS v1.2                              ║\n";
        cout <<   "  ╠══════════════════════════════════════════════════╣\n";
        cout <<   "  ║   Gates:" << setw(8) << total_gates << "  Verified:" << setw(8) << verified_count << "/" << total_compute << "        ║\n";
        cout <<   "  ║   Pinky:" << setw(8) << pinky_swears << "  Divine:" << setw(8) << divine_injections << "        ║\n";
        cout <<   "  ║   ZANS:" << setw(9) << zans_applications << "  Result: " 
             << (verified_count == total_compute ? "ALL OK" : "FAILURES") << "        ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════╝\n\n";
    }
    
    Ciphertext<DCRTPoly> get_output(int node_id) { return node_map[node_id].ct; }
    int64_t get_plaintext(int node_id) { return node_map[node_id].plaintext_result; }
};

#endif // PHI_ARBITRARY_CIRCUIT_H
