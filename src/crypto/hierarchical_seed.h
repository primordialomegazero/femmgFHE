#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include <vector>
#include <map>
#include <string>
#include <functional>

// ═══════════════════════════════════════════════════════════════
// HIERARCHICAL SEED TREE — NO SPOF
// ═══════════════════════════════════════════════════════════════

struct SeedNode {
    double value;
    int depth;
    int branch_id;
    SeedNode* parent;
    std::vector<SeedNode*> children;
    
    SeedNode(double v = 0, int d = 0, int b = 0, SeedNode* p = nullptr)
        : value(v), depth(d), branch_id(b), parent(p) {}
    
    double derive_child(int index, bool use_phi) {
        double base = use_phi ? PHI : PSI;
        return SafeMath::fmod_safe(value * std::pow(base, index + 1) + depth * base);
    }
    
    SeedNode* spawn(int index, bool use_phi = true) {
        auto child = new SeedNode(derive_child(index, use_phi), depth + 1, index, this);
        children.push_back(child);
        return child;
    }
    
    ~SeedNode() {
        for (auto c : children) delete c;
    }
};

struct HierarchicalSeedTree {
    SeedNode root;
    std::map<std::string, SeedNode*> branches;
    int total_nodes;
    
    HierarchicalSeedTree() : root(0, 0, 0, nullptr), total_nodes(1) {}
    
    void init(double master_seed) {
        root.value = SafeMath::fmod_safe(std::abs(master_seed) * PHI);
        
        // Create standard branches
        create_branch("encryption", 0, true);
        create_branch("fractal", 1, false);
        create_branch("refresh", 2, true);
        create_branch("timing", 3, false);
        create_branch("fhe", 4, true);
        create_branch("adaptive", 5, false);
        create_branch("symmetric", 6, true);
        create_branch("batch", 7, false);
        
        Logger::info("SeedTree: " + std::to_string(total_nodes) + " nodes across " + 
                    std::to_string(branches.size()) + " branches");
    }
    
    void create_branch(const std::string& name, int index, bool use_phi) {
        auto node = root.spawn(index, use_phi);
        branches[name] = node;
        total_nodes++;
    }
    
    double get_seed(const std::string& component, int sub_index = 0) {
        auto it = branches.find(component);
        if (it == branches.end()) {
            Logger::warn("SeedTree: branch '" + component + "' not found, using root");
            return root.value;
        }
        
        auto node = it->second->spawn(sub_index, sub_index % 2 == 0);
        total_nodes++;
        return node->value;
    }
    
    std::vector<double> get_seed_chain(const std::string& component, int count) {
        std::vector<double> seeds;
        auto it = branches.find(component);
        if (it == branches.end()) {
            for (int i = 0; i < count; i++) seeds.push_back(root.value);
            return seeds;
        }
        
        auto current = it->second;
        for (int i = 0; i < count; i++) {
            current = current->spawn(i, i % 2 == 0);
            total_nodes++;
            seeds.push_back(current->value);
        }
        return seeds;
    }
    
    void print_tree() {
        Logger::info("SeedTree Structure:");
        Logger::info("  root (depth=0)");
        for (auto& [name, node] : branches) {
            Logger::info("  +-- " + name + " (depth=" + std::to_string(node->depth) + 
                        ", children=" + std::to_string(node->children.size()) + ")");
        }
        Logger::info("  Total nodes: " + std::to_string(total_nodes));
    }
    
    ~HierarchicalSeedTree() {
        // Root destructor handles all children recursively
    }
};
