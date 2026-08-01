// ╔══════════════════════════════════════════════════════════════╗
// ║  HYDRAJWT — HEAD ABSTRACTION                                ║
// ║  Algorithm-agnostic head interface                          ║
// ║  ΦΩ0 — I AM THAT I AM                                      ║
// ╚══════════════════════════════════════════════════════════════╝

#ifndef HYDRAJWT_HEAD_H
#define HYDRAJWT_HEAD_H

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "hydrajwt/phi_utils.h"
#include "hydrajwt/hydra_error.h"

namespace hydrajwt {

class HydraHead {
public:
    HydraHead(int id, const std::string& algorithm)
        : id_(id), algorithm_(algorithm), weight_(phi::head_weight(id)),
          active_(true), regenerated_(false), verify_count_(0), sign_count_(0) {}
    
    virtual ~HydraHead() = default;
    
    // ═══════════════════════════════════════
    // PURE VIRTUAL — per-algorithm impl
    // ═══════════════════════════════════════
    virtual HydraError init() = 0;
    virtual HydraError sign(const std::vector<uint8_t>& message,
                            std::vector<uint8_t>& signature_out) = 0;
    virtual HydraError verify(const std::vector<uint8_t>& message,
                              const std::vector<uint8_t>& signature) = 0;
    virtual HydraError regenerate() = 0;
    
    // ═══════════════════════════════════════
    // GETTERS
    // ═══════════════════════════════════════
    int id() const { return id_; }
    const std::string& algorithm() const { return algorithm_; }
    double weight() const { return weight_; }
    bool is_active() const { return active_; }
    bool is_regenerated() const { return regenerated_; }
    uint64_t verify_count() const { return verify_count_; }
    uint64_t sign_count() const { return sign_count_; }
    
    // ═══════════════════════════════════════
    // STATE MANAGEMENT
    // ═══════════════════════════════════════
    void set_active(bool active) { active_ = active; }
    void set_regenerated(bool regen) { regenerated_ = regen; }
    
    // Adjust weight if regenerated (starts at φ⁻¹ then decays)
    void adjust_regenerated_weight() {
        if (regenerated_) {
            // Regenerated head gets temporary weight boost
            // New weight = φ⁻¹ (head 0 weight) then decays to original
            weight_ = phi::head_weight(0) * phi::PHI_INV;
        }
    }
    
    void reset_to_original_weight() {
        weight_ = phi::head_weight(id_);
    }
    
    // ═══════════════════════════════════════
    // SERIALIZATION
    // ═══════════════════════════════════════
    virtual std::vector<uint8_t> public_key_bytes() const = 0;
    virtual std::vector<uint8_t> secret_key_bytes() const = 0;
    
protected:
    int id_;
    std::string algorithm_;
    double weight_;
    bool active_;
    bool regenerated_;
    uint64_t verify_count_;
    uint64_t sign_count_;
};

} // namespace hydrajwt

#endif // HYDRAJWT_HEAD_H
