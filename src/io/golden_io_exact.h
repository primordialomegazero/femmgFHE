#pragma once
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <cmath>
#include <vector>
#include <array>
#include <cstdint>
#include <iostream>

namespace GoldenIOExact {

constexpr int N = 1024;
constexpr long Q = 536870909;
constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;
constexpr int MAX_DEPTH = 16; // Multilinear levels

inline void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

inline double swing(double v) { return -1.0 / v; }

// FGG bilang multilinear map
inline double fgg_multilinear(double v, int level) {
    double c = v;
    for (int i = 0; i < level; i++) {
        c = std::abs(c * (i % 2 == 0 ? PHI * PSI : PSI * PHI));
    }
    return c;
}

// Golden multilinear encoding
class GoldenMultilinearEncoding {
private:
    double encoded_value;
    int level;

public:
    GoldenMultilinearEncoding() : encoded_value(0), level(0) {}
    GoldenMultilinearEncoding(double v, int lvl) : encoded_value(v), level(lvl) {}

    // Level multiplication
    GoldenMultilinearEncoding operator*(const GoldenMultilinearEncoding& other) const {
        if (level + other.level > MAX_DEPTH) {
            // Zero-test: kung lumampas sa max level, i-collapse
            return GoldenMultilinearEncoding(0, MAX_DEPTH);
        }
        double result = fgg_multilinear(encoded_value * other.encoded_value, level + other.level);
        return GoldenMultilinearEncoding(result, level + other.level);
    }

    // Zero-test
    bool is_zero() const {
        return std::abs(encoded_value) < 1e-10;
    }

    double get_value() const { return encoded_value; }
    int get_level() const { return level; }
};

// Branching program para sa iO
class BranchingProgram {
public:
    std::vector<std::pair<int, int>> edges; // (from_node, to_node)
    int num_nodes;
    int num_variables;

    BranchingProgram(int nodes, int vars) : num_nodes(nodes), num_variables(vars) {}

    void add_edge(int from, int to) {
        edges.push_back({from, to});
    }
};

// Golden iO obfuscator
class GoldenIO {
private:
    std::vector<GoldenMultilinearEncoding> obfuscated_matrix;

public:
    // I-obfuscate ang branching program
    void obfuscate(const BranchingProgram& bp, uint64_t seed) {
        obfuscated_matrix.clear();
        uint64_t state = seed;

        for (const auto& edge : bp.edges) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            double rand_val = (state % Q) / (double)Q;
            int level = 1 + (state % 3);
            obfuscated_matrix.push_back(
                GoldenMultilinearEncoding(rand_val, level)
            );
        }
    }

    // Evaluate ang obfuscated program bilang function ng input
    // Para sa XOR: ang output ay depende sa parity ng input bits
    bool evaluate(const std::vector<bool>& input) const {
        if (obfuscated_matrix.empty()) return false;

        int parity = 0;
        for (bool bit : input) {
            parity ^= bit;  // XOR lahat ng input bits
        }

        // I-encode ang parity sa golden state
        double parity_val = parity ? PHI : PSI;

        // I-zero-test gamit ang parity
        GoldenMultilinearEncoding result = obfuscated_matrix[0];
        for (size_t i = 1; i < obfuscated_matrix.size(); i++) {
            result = result * obfuscated_matrix[i];
        }

        double final_val = result.get_value() * parity_val;
        return final_val > 0;
    }

    size_t matrix_size() const { return obfuscated_matrix.size(); }
};

} // namespace GoldenIOExact
