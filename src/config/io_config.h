#pragma once
#include "../core/constants.h"
#include <string>
#include <cstdint>

// ═══════════════════════════════════════════════════════════════
// UNIVERSAL iO CONFIGURATION — Production-Grade
// ═══════════════════════════════════════════════════════════════

enum class iOMode {
    STRUCTURAL_IO,   // Hide circuit structure, preserve function (iO)
    BLACKHOLE,       // Total erasure — max security
    OFF              // No obfuscation
};

struct iOConfig {
    // ═══════════════════════════════════════════════════════════
    // Fractal Golden iO — Core Settings
    // ═══════════════════════════════════════════════════════════
    iOMode io_mode = iOMode::STRUCTURAL_IO;
    int fractal_io_depth = 3;           // Minimum stable depth for iO (≥3)
    int N_obfuscation_rounds = 5;       // Number of obfuscation layers
    bool enable_blackhole_defense = false;
    bool enable_sidechannel_defense = true;
    
    // ═══════════════════════════════════════════════════════════
    // Encryption layers
    // ═══════════════════════════════════════════════════════════
    int encryption_layers = 23;
    double master_seed = 0.123456789;

    // ═══════════════════════════════════════════════════════════
    // FHE parameters
    // ═══════════════════════════════════════════════════════════
    uint32_t ring_dim = 4096;
    uint32_t depth = 120;
    uint32_t batch_size = 256;

    // ═══════════════════════════════════════════════════════════
    // Batching
    // ═══════════════════════════════════════════════════════════
    enum BatchMode { SINGLE, BULK, STREAMING };
    BatchMode batch_mode = SINGLE;
    int bulk_size = 10;

    // ═══════════════════════════════════════════════════════════
    // Keys
    // ═══════════════════════════════════════════════════════════
    enum KeyMode { SINGLE_KEY, ROTATING, MULTI_KEY };
    KeyMode key_mode = SINGLE_KEY;
    int key_rotation_frequency = 10;

    // ═══════════════════════════════════════════════════════════
    // Seeds
    // ═══════════════════════════════════════════════════════════
    enum SeedMode { SINGLE_SEED, DERIVED, INDEPENDENT };
    SeedMode seed_mode = DERIVED;

    // ═══════════════════════════════════════════════════════════
    // Party
    // ═══════════════════════════════════════════════════════════
    enum PartyMode { SINGLE_PARTY, DUAL_PARTY, MULTI_PARTY };
    PartyMode party_mode = SINGLE_PARTY;
    int num_parties = 1;

    // ═══════════════════════════════════════════════════════════
    // Circuit topology
    // ═══════════════════════════════════════════════════════════
    enum Topology { CHAIN, TREE, GRAPH, CUSTOM };
    Topology topology = CHAIN;

    // ═══════════════════════════════════════════════════════════
    // Refresh strategy
    // ═══════════════════════════════════════════════════════════
    enum RefreshStrategy { FIXED, ADAPTIVE, CHAOS, MANUAL };
    RefreshStrategy refresh_strategy = ADAPTIVE;
    int fixed_refresh_interval = 25;

    // ═══════════════════════════════════════════════════════════
    // Fractal parameters
    // ═══════════════════════════════════════════════════════════
    int fractal_layers = 23;
    int fractal_depth = 7;
    double input_weight = 0.75;

    // ═══════════════════════════════════════════════════════════
    // Symmetric reconstruction
    // ═══════════════════════════════════════════════════════════
    enum SymmetricMode { SUM_ONLY, PROD_ONLY, HARMONIC, GEOMETRIC, ALL };
    SymmetricMode symmetric_mode = ALL;

    // ═══════════════════════════════════════════════════════════
    // Timing
    // ═══════════════════════════════════════════════════════════
    enum TimingMode { NO_DELAY, FIXED_DELAY, EMERGENT, CHAOS_DELAY };
    TimingMode timing_mode = EMERGENT;
    double fixed_delay_ms = 1.0;

    // ═══════════════════════════════════════════════════════════
    // Adaptive
    // ═══════════════════════════════════════════════════════════
    bool adaptive_enabled = true;
    int optimization_interval = 20;

    // ═══════════════════════════════════════════════════════════
    // Logging
    // ═══════════════════════════════════════════════════════════
    int log_interval = 25;
    bool verbose_refresh_log = true;

    // ═══════════════════════════════════════════════════════════
    // Utility — iO mode string
    // ═══════════════════════════════════════════════════════════
    std::string io_mode_string() const {
        switch(io_mode) {
            case iOMode::STRUCTURAL_IO: return "STRUCTURAL_iO";
            case iOMode::BLACKHOLE: return "BLACKHOLE";
            case iOMode::OFF: return "OFF";
        }
        return "UNKNOWN";
    }

    std::string batch_mode_string() const {
        switch(batch_mode) {
            case SINGLE: return "SINGLE"; case BULK: return "BULK"; case STREAMING: return "STREAMING";
        } return "UNKNOWN";
    }
    std::string key_mode_string() const {
        switch(key_mode) {
            case SINGLE_KEY: return "SINGLE"; case ROTATING: return "ROTATING"; case MULTI_KEY: return "MULTI";
        } return "UNKNOWN";
    }
    std::string seed_mode_string() const {
        switch(seed_mode) {
            case SINGLE_SEED: return "SINGLE"; case DERIVED: return "DERIVED"; case INDEPENDENT: return "INDEPENDENT";
        } return "UNKNOWN";
    }
    std::string party_mode_string() const {
        switch(party_mode) {
            case SINGLE_PARTY: return "SINGLE"; case DUAL_PARTY: return "DUAL"; case MULTI_PARTY: return "MULTI";
        } return "UNKNOWN";
    }
    std::string topology_string() const {
        switch(topology) {
            case CHAIN: return "CHAIN"; case TREE: return "TREE"; case GRAPH: return "GRAPH"; case CUSTOM: return "CUSTOM";
        } return "UNKNOWN";
    }
    std::string refresh_string() const {
        switch(refresh_strategy) {
            case FIXED: return "FIXED"; case ADAPTIVE: return "ADAPTIVE"; case CHAOS: return "CHAOS"; case MANUAL: return "MANUAL";
        } return "UNKNOWN";
    }
    std::string symmetric_string() const {
        switch(symmetric_mode) {
            case SUM_ONLY: return "SUM"; case PROD_ONLY: return "PRODUCT";
            case HARMONIC: return "HARMONIC"; case GEOMETRIC: return "GEOMETRIC"; case ALL: return "ALL";
        } return "UNKNOWN";
    }
    std::string timing_string() const {
        switch(timing_mode) {
            case NO_DELAY: return "NONE"; case FIXED_DELAY: return "FIXED";
            case EMERGENT: return "EMERGENT"; case CHAOS_DELAY: return "CHAOS";
        } return "UNKNOWN";
    }
};
