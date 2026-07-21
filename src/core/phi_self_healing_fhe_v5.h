// ΦΩ0 — SELF-HEALING FHE ENGINE v5.1 — CKKS + REAL BOOTSTRAPPING
// v5.1: Added execute_no_verify() for deep chain testing
// "PUSH UNTIL IT BREAKS."

#ifndef PHI_SELF_HEALING_FHE_V5_H
#define PHI_SELF_HEALING_FHE_V5_H

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
#include <complex>

using namespace lbcrypto;
using namespace std;

enum class FHEGateTypeV5 { INPUT, CONSTANT, ADD, MUL, MUL_SCALAR, SUB, NEG };

struct FHENodeV5 {
    int id;
    FHEGateTypeV5 type;
    string name;
    double value;
    vector<int> inputs;
    Ciphertext<DCRTPoly> ct;
    int mult_depth;
    int max_input_depth;
    bool needs_bootstrap;
    bool verified;
    double plaintext_result;
    int bootstrap_count;

    FHENodeV5() : id(-1), type(FHEGateTypeV5::INPUT), value(0.0),
                  mult_depth(0), max_input_depth(0),
                  needs_bootstrap(false), verified(false),
                  plaintext_result(0.0), bootstrap_count(0) {}
};

class TopoSortV5 {
public:
    static vector<int> sort(const vector<FHENodeV5>& nodes) {
        int n = nodes.size();
        map<int, vector<int>> adj;
        map<int, int> in_deg;
        for (const auto& node : nodes) {
            if (in_deg.find(node.id) == in_deg.end()) in_deg[node.id] = 0;
            adj[node.id] = {};
        }
        for (const auto& node : nodes)
            for (int inp : node.inputs) {
                adj[inp].push_back(node.id);
                in_deg[node.id]++;
            }
        queue<int> q;
        for (const auto& [id, deg] : in_deg)
            if (deg == 0) q.push(id);
        vector<int> order;
        while (!q.empty()) {
            int cur = q.front(); q.pop();
            order.push_back(cur);
            for (int child : adj[cur]) {
                in_deg[child]--;
                if (in_deg[child] == 0) q.push(child);
            }
        }
        if (order.size() != nodes.size())
            throw runtime_error("CYCLE DETECTED IN CIRCUIT!");
        return order;
    }
};

class SelfHealingFHEv5 {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    map<int, FHENodeV5> node_map;
    vector<int> exec_order;
    uint32_t ringDim, numSlots, levelsAvailableAfterBootstrap, totalDepth;
    int total_gates = 0, bootstrap_ops = 0, max_critical_path = 0;
    int MAX_CHAIN_MULTS;

public:
    SelfHealingFHEv5(uint32_t ringDimPower = 12,
                     uint32_t levelsAfterBoot = 10,
                     int maxChainMults = 25)
        : levelsAvailableAfterBootstrap(levelsAfterBoot),
          MAX_CHAIN_MULTS(maxChainMults) {
        ringDim = 1 << ringDimPower;
        numSlots = ringDim / 2;
        CCParams<CryptoContextCKKSRNS> params;
        params.SetSecretKeyDist(UNIFORM_TERNARY);
        params.SetSecurityLevel(HEStd_NotSet);
        params.SetRingDim(ringDim);
#if NATIVEINT == 128
        params.SetScalingModSize(78); params.SetFirstModSize(89);
        params.SetScalingTechnique(FIXEDAUTO);
#else
        params.SetScalingModSize(59); params.SetFirstModSize(60);
        params.SetScalingTechnique(FLEXIBLEAUTO);
#endif
        vector<uint32_t> levelBudget = {4, 4};
        uint32_t bootstrapDepth = FHECKKSRNS::GetBootstrapDepth(levelBudget, UNIFORM_TERNARY);
        totalDepth = levelsAvailableAfterBootstrap + bootstrapDepth;
        params.SetMultiplicativeDepth(totalDepth);
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE);
        cc->Enable(ADVANCEDSHE); cc->Enable(FHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        cc->EvalBootstrapSetup(levelBudget, {0, 0}, numSlots);
        cc->EvalBootstrapKeyGen(keys.secretKey, numSlots);
        cout << "  [V5.1] RingDim: " << ringDim << " | Slots: " << numSlots
             << " | TotalDepth: " << totalDepth
             << " | LevelsAfterBoot: " << levelsAvailableAfterBootstrap << "\n";
    }

    void set_max_chain_mults(int m) { MAX_CHAIN_MULTS = m; }
    int get_max_chain_mults() { return MAX_CHAIN_MULTS; }

    Ciphertext<DCRTPoly> encrypt_value(double v) {
        vector<double> vec(numSlots, 0.0); vec[0] = v;
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    }

    double decrypt_value(const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt; cc->Decrypt(keys.secretKey, ct, &pt);
        pt->SetLength(numSlots);
        return pt->GetRealPackedValue()[0];
    }

    Ciphertext<DCRTPoly> do_bootstrap(const Ciphertext<DCRTPoly>& ct) {
        bootstrap_ops++;
        return cc->EvalBootstrap(ct);
    }

    int add_input(const string& name, double value) {
        FHENodeV5 node;
        node.id = node_map.size(); node.type = FHEGateTypeV5::INPUT;
        node.name = name; node.value = value;
        node.plaintext_result = value; node.mult_depth = 0;
        node_map[node.id] = node; total_gates++;
        return node.id;
    }

    int add_gate(FHEGateTypeV5 type, const string& name,
                 int a, int b = -1, double scalar = 0.0) {
        FHENodeV5 node;
        node.id = node_map.size(); node.type = type;
        node.name = name; node.value = scalar;
        node.inputs.push_back(a);
        if (b >= 0) node.inputs.push_back(b);
        node_map[node.id] = node; total_gates++;
        return node.id;
    }

    int add_add(const string& n, int a, int b) { return add_gate(FHEGateTypeV5::ADD, n, a, b); }
    int add_mul(const string& n, int a, int b) { return add_gate(FHEGateTypeV5::MUL, n, a, b); }
    int add_mul_scalar(const string& n, int a, double s) { return add_gate(FHEGateTypeV5::MUL_SCALAR, n, a, -1, s); }
    int add_sub(const string& n, int a, int b) { return add_gate(FHEGateTypeV5::SUB, n, a, b); }

    void analyze_critical_paths() {
        vector<FHENodeV5> node_list;
        for (auto& [id, node] : node_map) node_list.push_back(node);
        auto order = TopoSortV5::sort(node_list);
        for (int node_id : order) {
            FHENodeV5& node = node_map[node_id];
            switch (node.type) {
                case FHEGateTypeV5::INPUT: case FHEGateTypeV5::CONSTANT:
                    node.mult_depth = 0; node.max_input_depth = 0; break;
                case FHEGateTypeV5::ADD: case FHEGateTypeV5::SUB: {
                    int d0 = node_map[node.inputs[0]].mult_depth;
                    int d1 = node_map[node.inputs[1]].mult_depth;
                    node.max_input_depth = max(d0, d1);
                    node.mult_depth = node.max_input_depth; break;
                }
                case FHEGateTypeV5::MUL: case FHEGateTypeV5::MUL_SCALAR: {
                    int d0 = node_map[node.inputs[0]].mult_depth;
                    int d1 = (node.inputs.size() > 1) ? node_map[node.inputs[1]].mult_depth : d0;
                    node.max_input_depth = max(d0, d1);
                    node.mult_depth = node.max_input_depth + 1; break;
                }
                default: break;
            }
            if ((node.type == FHEGateTypeV5::MUL || node.type == FHEGateTypeV5::MUL_SCALAR) &&
                node.max_input_depth >= MAX_CHAIN_MULTS)
                node.needs_bootstrap = true;
            max_critical_path = max(max_critical_path, node.mult_depth);
        }
    }

    double compute_plaintext(int node_id) {
        FHENodeV5& node = node_map[node_id];
        switch (node.type) {
            case FHEGateTypeV5::INPUT: case FHEGateTypeV5::CONSTANT: return node.value;
            case FHEGateTypeV5::ADD: return compute_plaintext(node.inputs[0]) + compute_plaintext(node.inputs[1]);
            case FHEGateTypeV5::SUB: return compute_plaintext(node.inputs[0]) - compute_plaintext(node.inputs[1]);
            case FHEGateTypeV5::MUL: return compute_plaintext(node.inputs[0]) * compute_plaintext(node.inputs[1]);
            case FHEGateTypeV5::MUL_SCALAR: return compute_plaintext(node.inputs[0]) * node.value;
            default: return 0.0;
        }
    }

    void execute() { execute_impl(true); }
    void execute_no_verify() { execute_impl(false); }

    void print_stats() {
        int ver = 0, tot = 0;
        for (auto& [id, node] : node_map) {
            if (node.type != FHEGateTypeV5::INPUT && node.type != FHEGateTypeV5::CONSTANT) {
                tot++; if (node.verified) ver++;
            }
        }
        cout << "\n  ╔══════════════════════════════════════════════════════╗\n";
        cout <<   "  ║   V5.1 RESULTS: Gates=" << setw(5) << total_gates
             << " Verified=" << ver << "/" << tot
             << " Bootstraps=" << bootstrap_ops << "          ║\n";
        cout <<   "  ║   MaxPath=" << max_critical_path
             << " Threshold=" << MAX_CHAIN_MULTS
             << " LevelsAfterBoot=" << levelsAvailableAfterBootstrap << "          ║\n";
        cout <<   "  ╚══════════════════════════════════════════════════════╝\n\n";
    }

    FHENodeV5& get_node(int id) { return node_map[id]; }
    double get_plaintext(int id) { return node_map[id].plaintext_result; }
    int get_bootstrap_count() { return bootstrap_ops; }
    int get_max_critical_path() { return max_critical_path; }

private:
    void execute_impl(bool verify) {
        analyze_critical_paths();
        vector<FHENodeV5> node_list;
        for (auto& [id, node] : node_map) node_list.push_back(node);
        exec_order = TopoSortV5::sort(node_list);
        cout << "\n  [Execute] Gates: " << total_gates << " | MaxPath: " << max_critical_path
             << " | Threshold: " << MAX_CHAIN_MULTS << "\n";
        map<int, int> current_depth;
        for (int node_id : exec_order) {
            FHENodeV5& node = node_map[node_id];
            if (node.needs_bootstrap) {
                int input0 = node.inputs[0];
                if (current_depth[input0] >= MAX_CHAIN_MULTS) {
                    node_map[input0].ct = do_bootstrap(node_map[input0].ct);
                    node_map[input0].bootstrap_count++;
                    current_depth[input0] = 0;
                }
                if (node.inputs.size() > 1) {
                    int input1 = node.inputs[1];
                    if (current_depth[input1] >= MAX_CHAIN_MULTS) {
                        node_map[input1].ct = do_bootstrap(node_map[input1].ct);
                        node_map[input1].bootstrap_count++;
                        current_depth[input1] = 0;
                    }
                }
            }
            switch (node.type) {
                case FHEGateTypeV5::INPUT: case FHEGateTypeV5::CONSTANT:
                    node.ct = encrypt_value(node.value);
                    current_depth[node.id] = 0; break;
                case FHEGateTypeV5::ADD:
                    node.ct = cc->EvalAdd(node_map[node.inputs[0]].ct, node_map[node.inputs[1]].ct);
                    current_depth[node.id] = max(current_depth[node.inputs[0]], current_depth[node.inputs[1]]); break;
                case FHEGateTypeV5::SUB:
                    node.ct = cc->EvalSub(node_map[node.inputs[0]].ct, node_map[node.inputs[1]].ct);
                    current_depth[node.id] = max(current_depth[node.inputs[0]], current_depth[node.inputs[1]]); break;
                case FHEGateTypeV5::MUL:
                    node.ct = cc->EvalMult(node_map[node.inputs[0]].ct, node_map[node.inputs[1]].ct);
                    current_depth[node.id] = max(current_depth[node.inputs[0]], current_depth[node.inputs[1]]) + 1; break;
                case FHEGateTypeV5::MUL_SCALAR: {
                    auto ct_s = encrypt_value(node.value);
                    node.ct = cc->EvalMult(node_map[node.inputs[0]].ct, ct_s);
                    current_depth[node.id] = current_depth[node.inputs[0]] + 1; break;
                }
                default: break;
            }
            if (verify) {
                node.plaintext_result = compute_plaintext(node.id);
                double dec = decrypt_value(node.ct);
                node.verified = (abs(dec - node.plaintext_result) < 0.01);
                if (!node.verified && node.type != FHEGateTypeV5::INPUT && node.type != FHEGateTypeV5::CONSTANT)
                    cout << "  FAIL @" << node.id << " " << node.name
                         << " Exp:" << node.plaintext_result << " Got:" << dec
                         << " Depth:" << current_depth[node.id] << endl;
            }
        }
    }
};

#endif
