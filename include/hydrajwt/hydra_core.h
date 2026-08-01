// ╔══════════════════════════════════════════════════════════════╗
// ║  HYDRAJWT — CORE ENGINE                                     ║
// ║  φ-weighted multi-head consensus                            ║
// ║  ΦΩ0 — I AM THAT I AM                                      ║
// ╚══════════════════════════════════════════════════════════════╝

#ifndef HYDRAJWT_CORE_H
#define HYDRAJWT_CORE_H

#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include "hydrajwt/hydra_head.h"
#include "hydrajwt/hydra_token.h"
#include "hydrajwt/phi_utils.h"

namespace hydrajwt {

struct ConsensusResult {
    bool passed;
    double total_weight;
    double valid_weight;
    double ratio;
    std::vector<int> verified_heads;
    std::vector<int> failed_heads;
    std::string message;
    
    ConsensusResult() : passed(false), total_weight(0.0),
                        valid_weight(0.0), ratio(0.0) {}
};

class HydraCore {
public:
    HydraCore();
    ~HydraCore() = default;
    
    // ═══════════════════════════════════════
    // HEAD MANAGEMENT
    // ═══════════════════════════════════════
    HydraError add_head(std::unique_ptr<HydraHead> head);
    HydraError remove_head(int head_id);
    HydraError regenerate_head(int head_id);
    size_t head_count() const { return heads_.size(); }
    
    // ═══════════════════════════════════════
    // SIGN & VERIFY
    // ═══════════════════════════════════════
    HydraError sign(const std::string& payload, std::string& token_out);
    HydraError verify(const std::string& token, ConsensusResult& result_out);
    
    // ═══════════════════════════════════════
    // CONFIG
    // ═══════════════════════════════════════
    void set_threshold(double threshold) { threshold_ = threshold; }
    double threshold() const { return threshold_; }
    uint64_t token_count() const { return token_count_.load(); }
    uint64_t verify_count() const { return verify_count_.load(); }
    
private:
    std::vector<std::unique_ptr<HydraHead>> heads_;
    HydraToken token_handler_;
    double threshold_;
    std::atomic<uint64_t> token_count_;
    std::atomic<uint64_t> verify_count_;
    
    ConsensusResult run_consensus(
        const std::vector<uint8_t>& message,
        const std::vector<TokenSignature>& signatures);
};

} // namespace hydrajwt

#endif // HYDRAJWT_CORE_H
