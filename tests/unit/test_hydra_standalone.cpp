#include <iostream>
#include <cassert>
#include <hydrajwt/hydra_core.h>
#include <hydrajwt/heads/head_schnorr.h>
#include <hydrajwt/heads/head_phi_hmac.h>

int main() {
    std::cout << "===============================================================================\n";
    std::cout << "  HYDRA JWT — STANDALONE TEST\n";
    std::cout << "===============================================================================\n\n";
    
    std::cout << "--- Creating HydraCore with 2 heads ---\n";
    hydrajwt::HydraCore core;
    
    // Add Schnorr head
    auto schnorr = std::make_unique<hydrajwt::HeadSchnorr>(0);
    hydrajwt::HydraError err = schnorr->init();
    if (err == hydrajwt::HydraError::OK) {
        std::cout << "[OK] Schnorr initialized\n";
        core.add_head(std::move(schnorr));
    } else {
        std::cout << "[FAIL] Schnorr init failed\n";
    }
    
    // Add Phi-HMAC head
    auto phi_hmac = std::make_unique<hydrajwt::HeadPhiHMAC>(1);
    err = phi_hmac->init();
    if (err == hydrajwt::HydraError::OK) {
        std::cout << "[OK] φ-HMAC initialized\n";
        core.add_head(std::move(phi_hmac));
    } else {
        std::cout << "[FAIL] φ-HMAC init failed\n";
    }
    
    std::cout << "\n--- Heads: " << core.head_count() << " ---\n";
    
    if (core.head_count() > 0) {
        core.set_threshold(1.0 / 1.618);  // 1/φ
        
        std::cout << "\n--- Signing ---\n";
        std::string token;
        err = core.sign("test-payload", token);
        if (err == hydrajwt::HydraError::OK) {
            std::cout << "[OK] Token created (" << token.size() << " chars)\n";
            
            std::cout << "\n--- Verifying ---\n";
            hydrajwt::ConsensusResult result;
            err = core.verify(token, result);
            if (err == hydrajwt::HydraError::OK && result.passed) {
                std::cout << "[OK] Token verified\n";
                std::cout << "  Ratio: " << result.ratio << "\n";
                std::cout << "  Verified heads: " << result.verified_heads.size() << "\n";
            } else {
                std::cout << "[FAIL] Verification failed\n";
            }
        } else {
            std::cout << "[FAIL] Signing failed\n";
        }
    }
    
    std::cout << "\n===============================================================================\n";
    std::cout << "  HYDRA JWT STANDALONE TEST COMPLETE\n";
    std::cout << "===============================================================================\n";
    return 0;
}
