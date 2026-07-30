#pragma once
#include "../core/constants.h"
#include "../utils/safe_math.h"
#include "../utils/logger.h"
#include "../crypto/hierarchical_seed.h"
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <sstream>

// ═══════════════════════════════════════════════════════════════
// CONFIGURABLE API ENDPOINTS
// ═══════════════════════════════════════════════════════════════

enum class EndpointAuth { NONE, API_KEY, HMAC, MULTI_FACTOR };

struct APIEndpoint {
    std::string path;
    std::string method;  // GET, POST, PUT, DELETE
    std::string description;
    EndpointAuth auth_level;
    bool enabled;
    int rate_limit_per_minute;
    std::string seed_branch;  // Which seed branch to use
};

struct APIConfig {
    std::string base_url;
    int port;
    bool https_enabled;
    std::vector<APIEndpoint> endpoints;
    std::map<std::string, std::string> api_keys;  // key -> user
    
    void add_endpoint(const std::string& path, const std::string& method,
                      const std::string& desc, EndpointAuth auth = EndpointAuth::NONE,
                      const std::string& branch = "default") {
        endpoints.push_back({path, method, desc, auth, true, 60, branch});
    }
    
    void add_api_key(const std::string& key, const std::string& user) {
        api_keys[key] = user;
    }
    
    bool validate_key(const std::string& key) {
        return api_keys.find(key) != api_keys.end();
    }
};

// ═══════════════════════════════════════════════════════════════
// CONFIGURABLE BRANCH REGISTRY
// ═══════════════════════════════════════════════════════════════

struct BranchConfig {
    std::string name;
    std::string description;
    bool use_phi;         // φ-branch or ψ-branch
    int branch_index;
    std::vector<std::string> sub_branches;
    bool allow_user_sub_branches;
    int max_depth;
};

struct BranchRegistry {
    HierarchicalSeedTree* seed_tree;
    std::map<std::string, BranchConfig> registered_branches;
    std::map<std::string, std::vector<double>> branch_seed_cache;
    
    void init(HierarchicalSeedTree* tree) {
        seed_tree = tree;
        register_standard_branches();
    }
    
    void register_standard_branches() {
        register_branch("encryption", "Fractal N-Encryption seeds", true, 0, true, 100);
        register_branch("fractal", "Fractal transform seeds", false, 1, true, 50);
        register_branch("refresh", "Auto-refresh controller seeds", true, 2, true, 30);
        register_branch("timing", "Emergent timing seeds", false, 3, false, 10);
        register_branch("fhe", "FHE context seeds", true, 4, false, 5);
        register_branch("adaptive", "Adaptive optimization seeds", false, 5, true, 20);
        register_branch("symmetric", "Symmetric reconstruction seeds", true, 6, false, 10);
        register_branch("batch", "Batch processing seeds", false, 7, true, 15);
        register_branch("default", "Default fallback branch", true, 99, false, 1);
    }
    
    void register_branch(const std::string& name, const std::string& desc,
                         bool use_phi, int index, bool allow_subs, int max_d) {
        BranchConfig cfg;
        cfg.name = name;
        cfg.description = desc;
        cfg.use_phi = use_phi;
        cfg.branch_index = index;
        cfg.allow_user_sub_branches = allow_subs;
        cfg.max_depth = max_d;
        registered_branches[name] = cfg;
        
        if (seed_tree) {
            seed_tree->create_branch(name, index, use_phi);
        }
    }
    
    // User registers their own branch
    void register_user_branch(const std::string& name, const std::string& desc,
                              bool use_phi, int index, int max_depth = 50) {
        if (registered_branches.find(name) != registered_branches.end()) {
            Logger::warn("Branch '" + name + "' already exists, overwriting");
        }
        register_branch(name, desc, use_phi, index, true, max_depth);
        Logger::info("User branch registered: " + name + " (" + desc + ")");
    }
    
    // Get seeds for a branch
    std::vector<double> get_seeds(const std::string& branch_name, int count) {
        auto it = registered_branches.find(branch_name);
        if (it == registered_branches.end()) {
            Logger::warn("Branch '" + branch_name + "' not found, using default");
            return seed_tree->get_seed_chain("default", count);
        }
        
        auto& cfg = it->second;
        if (!cfg.allow_user_sub_branches) {
            Logger::warn("Branch '" + branch_name + "' does not allow sub-branches");
            count = 1;
        }
        
        count = std::min(count, cfg.max_depth);
        return seed_tree->get_seed_chain(branch_name, count);
    }
    
    // Get single seed
    double get_seed(const std::string& branch_name, int sub_index = 0) {
        auto seeds = get_seeds(branch_name, sub_index + 1);
        return seeds.empty() ? 0.5 : seeds.back();
    }
    
    void list_branches() {
        Logger::section("REGISTERED BRANCHES");
        for (auto& [name, cfg] : registered_branches) {
            std::string type = cfg.use_phi ? "phi" : "psi";
            Logger::info("  " + name + " [" + type + "-branch] depth=" + 
                        std::to_string(cfg.max_depth) + " subs=" + 
                        (cfg.allow_user_sub_branches ? "yes" : "no"));
        }
    }
};

// ═══════════════════════════════════════════════════════════════
// API ROUTER
// ═══════════════════════════════════════════════════════════════

struct APIRouter {
    APIConfig config;
    BranchRegistry* branches;
    std::map<std::string, std::function<std::string(std::map<std::string,std::string>)>> handlers;
    
    void init(BranchRegistry* br, int port = 8080) {
        branches = br;
        config.base_url = "http://localhost";
        config.port = port;
        config.https_enabled = false;
        register_standard_endpoints();
    }
    
    void register_standard_endpoints() {
        config.add_endpoint("/init", "POST", "Initialize iO system", EndpointAuth::API_KEY, "default");
        config.add_endpoint("/encrypt", "POST", "Encrypt plaintext", EndpointAuth::API_KEY, "encryption");
        config.add_endpoint("/chain/run", "POST", "Run iO chain", EndpointAuth::API_KEY, "fhe");
        config.add_endpoint("/chain/status", "GET", "Get chain status", EndpointAuth::NONE, "default");
        config.add_endpoint("/refresh/trigger", "POST", "Manual refresh trigger", EndpointAuth::API_KEY, "refresh");
        config.add_endpoint("/fractal/config", "PUT", "Update fractal config", EndpointAuth::API_KEY, "fractal");
        config.add_endpoint("/adaptive/status", "GET", "Get adaptive metrics", EndpointAuth::API_KEY, "adaptive");
        config.add_endpoint("/seeds/generate", "POST", "Generate seed chain", EndpointAuth::API_KEY, "default");
        config.add_endpoint("/branches/list", "GET", "List all branches", EndpointAuth::NONE, "default");
        config.add_endpoint("/branches/register", "POST", "Register custom branch", EndpointAuth::API_KEY, "default");
    }
    
    // User registers custom endpoint
    void register_endpoint(const std::string& path, const std::string& method,
                           const std::string& desc, const std::string& branch,
                           EndpointAuth auth = EndpointAuth::API_KEY,
                           int rate_limit = 60) {
        config.add_endpoint(path, method, desc, auth, branch);
        config.endpoints.back().rate_limit_per_minute = rate_limit;
        Logger::info("Custom endpoint registered: " + method + " " + path + 
                    " -> branch:" + branch);
    }
    
    void set_handler(const std::string& path, 
                     std::function<std::string(std::map<std::string,std::string>)> handler) {
        handlers[path] = handler;
    }
    
    void add_api_key(const std::string& key, const std::string& user) {
        config.add_api_key(key, user);
        Logger::info("API key added for user: " + user);
    }
    
    void list_endpoints() {
        Logger::section("API ENDPOINTS");
        Logger::info("  Base URL: " + config.base_url + ":" + std::to_string(config.port));
        Logger::info("  HTTPS: " + std::string(config.https_enabled ? "yes" : "no"));
        Logger::info("  Endpoints:");
        for (auto& ep : config.endpoints) {
            std::string auth_str;
            switch(ep.auth_level) {
                case EndpointAuth::NONE: auth_str = "public"; break;
                case EndpointAuth::API_KEY: auth_str = "api_key"; break;
                case EndpointAuth::HMAC: auth_str = "hmac"; break;
                case EndpointAuth::MULTI_FACTOR: auth_str = "mfa"; break;
            }
            Logger::info("    " + ep.method + " " + ep.path + 
                        " -> " + ep.seed_branch + 
                        " [" + auth_str + "] " + 
                        (ep.enabled ? "" : "(disabled)"));
        }
    }
};
