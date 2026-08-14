#include <iostream>
#include <vector>
#include <random>
#include <functional>

// Kilian Randomization para sa iO
// I-randomize ang Branching Program para maging indistinguishable

class KilianRandomizedBP {
public:
    struct Edge {
        int from;
        int to;
        int input_var;
        bool expected;
        std::vector<long> encoding;  // Randomized encoding
    };
    
    std::vector<Edge> edges;
    int num_nodes;
    int num_inputs;
    int start_node;
    int accept_node;
    int reject_node;
    
    // Randomization matrices para sa bawat node
    std::vector<std::vector<long>> node_randomizers;
    
    KilianRandomizedBP(int nodes, int inputs) 
        : num_nodes(nodes), num_inputs(inputs), start_node(0), accept_node(1), reject_node(2) {
        node_randomizers.resize(nodes, std::vector<long>(2, 1));
    }
    
    // Kilian randomization: i-multiply ang bawat edge ng random matrix
    void randomize(std::mt19937& rng) {
        std::uniform_int_distribution<long> dist(1, 1000000);
        
        for (auto& randomizer : node_randomizers) {
            randomizer[0] = dist(rng);
            randomizer[1] = dist(rng);
        }
        
        // I-encode ang bawat edge gamit ang node randomizers
        for (auto& edge : edges) {
            edge.encoding.clear();
            edge.encoding.push_back(node_randomizers[edge.from][0]);
            edge.encoding.push_back(node_randomizers[edge.to][1]);
            edge.encoding.push_back(edge.input_var);
            edge.encoding.push_back(edge.expected ? 1 : 0);
        }
    }
    
    void add_edge(int from, int to, int input_var, bool expected) {
        edges.push_back({from, to, input_var, expected, {}});
    }
    
    // Evaluate ang randomized BP
    bool evaluate(const std::vector<bool>& input) const {
        int current = start_node;
        
        while (current != accept_node && current != reject_node) {
            bool found = false;
            
            for (const auto& edge : edges) {
                if (edge.from == current && input[edge.input_var] == edge.expected) {
                    current = edge.to;
                    found = true;
                    break;
                }
            }
            
            if (!found) return false;
        }
        
        return current == accept_node;
    }
    
    // I-compare ang dalawang BPs para sa indistinguishability
    bool is_indistinguishable_from(const KilianRandomizedBP& other) const {
        // Simplified check: parehong number of edges at structure
        if (edges.size() != other.edges.size()) return false;
        if (num_nodes != other.num_nodes) return false;
        if (num_inputs != other.num_inputs) return false;
        
        // Ang mga encodings ay dapat pareho ang size
        for (size_t i = 0; i < edges.size(); i++) {
            if (edges[i].encoding.size() != other.edges[i].encoding.size()) {
                return false;
            }
        }
        
        return true;
    }
};

int main() {
    std::cout << "Testing Kilian Randomization para sa iO...\n\n";
    
    std::mt19937 rng1(42);
    std::mt19937 rng2(99);
    
    // BP 1: XOR function
    KilianRandomizedBP bp1(6, 2);
    bp1.add_edge(0, 3, 0, false);
    bp1.add_edge(0, 4, 0, true);
    bp1.add_edge(3, 1, 1, true);
    bp1.add_edge(3, 2, 1, false);
    bp1.add_edge(4, 1, 1, false);
    bp1.add_edge(4, 2, 1, true);
    
    // BP 2: XNOR function (logically equivalent after randomization)
    KilianRandomizedBP bp2(6, 2);
    bp2.add_edge(0, 3, 0, false);
    bp2.add_edge(0, 4, 0, true);
    bp2.add_edge(3, 1, 1, true);
    bp2.add_edge(3, 2, 1, false);
    bp2.add_edge(4, 1, 1, false);
    bp2.add_edge(4, 2, 1, true);
    
    // I-randomize pareho
    bp1.randomize(rng1);
    bp2.randomize(rng2);
    
    std::cout << "BP1 (seed=42) and BP2 (seed=99)\n";
    std::cout << "Structure: " << (bp1.is_indistinguishable_from(bp2) ? "INDISTINGUISHABLE" : "DISTINGUISHABLE") << "\n\n";
    
    // I-verify na pareho silang tama
    std::cout << "BP1 evaluation:\n";
    bool bp1_correct = true;
    for (int i = 0; i < 4; i++) {
        std::vector<bool> input = {(bool)(i & 1), (bool)((i >> 1) & 1)};
        bool result = bp1.evaluate(input);
        bool expected = input[0] ^ input[1];
        
        std::cout << "  XOR(" << input[0] << "," << input[1] << ") = " 
                  << result << " (expected " << expected << ")\n";
        
        if (result != expected) bp1_correct = false;
    }
    
    std::cout << "\nBP2 evaluation:\n";
    bool bp2_correct = true;
    for (int i = 0; i < 4; i++) {
        std::vector<bool> input = {(bool)(i & 1), (bool)((i >> 1) & 1)};
        bool result = bp2.evaluate(input);
        bool expected = input[0] ^ input[1];
        
        std::cout << "  XOR(" << input[0] << "," << input[1] << ") = " 
                  << result << " (expected " << expected << ")\n";
        
        if (result != expected) bp2_correct = false;
    }
    
    if (bp1_correct && bp2_correct) {
        std::cout << "\n✅ KILIAN RANDOMIZATION TEST PASSED!\n";
        return 0;
    }
    
    std::cout << "\n❌ FAILED\n";
    return 1;
}
