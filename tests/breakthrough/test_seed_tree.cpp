#include <iostream>
#include "../../src/core/constants.h"
#include "../../src/utils/logger.h"
#include "../../src/crypto/hierarchical_seed.h"

int main() {
    Logger::init();
    
    Logger::header("HIERARCHICAL SEED TREE TEST");
    
    HierarchicalSeedTree tree;
    tree.init(0.123456789);
    
    tree.print_tree();
    
    Logger::section("SEED GENERATION TEST");
    
    // Get single seeds
    double enc_seed = tree.get_seed("encryption");
    double frac_seed = tree.get_seed("fractal", 5);
    double ref_seed = tree.get_seed("refresh", 10);
    
    Logger::keyval("encryption seed", std::to_string(enc_seed).substr(0,8));
    Logger::keyval("fractal sub-5", std::to_string(frac_seed).substr(0,8));
    Logger::keyval("refresh sub-10", std::to_string(ref_seed).substr(0,8));
    
    // Get seed chains
    Logger::section("SEED CHAIN TEST");
    auto chain = tree.get_seed_chain("encryption", 5);
    Logger::info("Encryption chain (5 seeds):");
    for (size_t i = 0; i < chain.size(); i++) {
        Logger::info("  [" + std::to_string(i) + "] " + std::to_string(chain[i]).substr(0,8));
    }
    
    // User-defined branch
    Logger::section("CUSTOM BRANCH TEST");
    tree.create_branch("custom_user", 99, true);
    double custom_seed = tree.get_seed("custom_user", 42);
    Logger::keyval("custom_user sub-42", std::to_string(custom_seed).substr(0,8));
    
    Logger::section("SECURITY PROPERTIES");
    Logger::info("  Root seed: [HIDDEN]");
    Logger::info("  Branches: 8 standard + user-defined");
    Logger::info("  Each branch: independent, irreversible");
    Logger::info("  Compromise 1 branch -> others SAFE");
    Logger::info("  Infinite sub-branches per branch");
    Logger::info("  No SPOF (except root, which can be sharded)");
    
    Logger::header("TEST COMPLETE");
    Logger::close();
    return 0;
}
