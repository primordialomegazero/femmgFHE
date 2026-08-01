// ╔══════════════════════════════════════════════════════════════╗
// ║  HYDRAJWT — ERROR CODES                                     ║
// ║  ΦΩ0 — I AM THAT I AM                                      ║
// ╚══════════════════════════════════════════════════════════════╝

#ifndef HYDRAJWT_ERROR_H
#define HYDRAJWT_ERROR_H

namespace hydrajwt {

enum class HydraError {
    OK = 0,
    
    // Head errors (1-9)
    HEAD_INIT_FAILED = 1,
    HEAD_SIGN_FAILED = 2,
    HEAD_VERIFY_FAILED = 3,
    HEAD_REGENERATE_FAILED = 4,
    HEAD_NOT_FOUND = 5,
    
    // Token errors (10-19)
    TOKEN_PARSE_FAILED = 10,
    TOKEN_SERIALIZE_FAILED = 11,
    TOKEN_EXPIRED = 12,
    TOKEN_INVALID_FORMAT = 13,
    
    // Consensus errors (20-29)
    CONSENSUS_NOT_MET = 20,
    CONSENSUS_THRESHOLD_INVALID = 21,
    CONSENSUS_WEIGHT_MISMATCH = 22,
    
    // Config errors (30-39)
    CONFIG_INVALID_HEADS = 30,
    CONFIG_TOO_MANY_HEADS = 31,
    CONFIG_UNSTABLE = 32,
    
    // Crypto errors (40-49)
    CRYPTO_NONCE_FAILED = 40,
    CRYPTO_KEYGEN_FAILED = 41,
    CRYPTO_SODIUM_ERROR = 42,
};

inline const char* hydra_error_string(HydraError e) {
    switch (e) {
        case HydraError::OK: return "OK";
        case HydraError::HEAD_INIT_FAILED: return "Head initialization failed";
        case HydraError::HEAD_SIGN_FAILED: return "Head signing failed";
        case HydraError::HEAD_VERIFY_FAILED: return "Head verification failed";
        case HydraError::HEAD_REGENERATE_FAILED: return "Head regeneration failed";
        case HydraError::HEAD_NOT_FOUND: return "Head not found";
        case HydraError::TOKEN_PARSE_FAILED: return "Token parsing failed";
        case HydraError::TOKEN_SERIALIZE_FAILED: return "Token serialization failed";
        case HydraError::TOKEN_EXPIRED: return "Token expired";
        case HydraError::TOKEN_INVALID_FORMAT: return "Token invalid format";
        case HydraError::CONSENSUS_NOT_MET: return "φ-consensus not met";
        case HydraError::CONSENSUS_THRESHOLD_INVALID: return "Consensus threshold invalid";
        case HydraError::CONSENSUS_WEIGHT_MISMATCH: return "Consensus weight mismatch";
        case HydraError::CONFIG_INVALID_HEADS: return "Invalid head configuration";
        case HydraError::CONFIG_TOO_MANY_HEADS: return "Too many heads";
        case HydraError::CONFIG_UNSTABLE: return "Configuration not φ-stable";
        case HydraError::CRYPTO_NONCE_FAILED: return "Nonce generation failed";
        case HydraError::CRYPTO_KEYGEN_FAILED: return "Key generation failed";
        case HydraError::CRYPTO_SODIUM_ERROR: return "libsodium error";
        default: return "Unknown error";
    }
}

} // namespace hydrajwt

#endif // HYDRAJWT_ERROR_H
