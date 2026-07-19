#ifndef FRACTAL_ZKP_H
#define FRACTAL_ZKP_H

#include <vector>
#include <string>
#include <openssl/ec.h>
#include <openssl/sha.h>

class FractalZKP {
public:
    static constexpr double PHI = 1.6180339887498948482;
    static constexpr size_t MAX_DEPTH = 7;
    static constexpr size_t MAX_BRANCHES = 5;
    
    struct Proof {
        std::string commitment_R, challenge_c, response_s, public_key_Y;
        size_t depth, branch_id;
        std::vector<Proof> children;
    };
    
    struct FractalTree {
        Proof root;
        size_t total_proofs, max_depth;
        bool verified;
    };
    
    static Proof prove_single(const std::string& data, size_t depth, size_t branch);
    static bool verify_single(const Proof& proof);
    static FractalTree prove_recursive(const std::string& data, size_t depth = 0, 
                                        size_t max_depth = 3, size_t branches = 2);
    static bool verify_tree(const Proof& node);
    static bool verify_tree(const FractalTree& tree);
};
#endif
