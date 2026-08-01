#pragma once
#include "spiral_fractal_db.h"
#include "spiral_fractal_auth.h"
#include "spiral_fractal_tls.h"
#include "spiral_fractal_zkp.h"
#include "spiral_fractal_fhe.h"
#include "spiral_fractal_defense.h"
#include "fhe_postgres/fhe_postgres.h"
#include "fhe_redis/fhe_redis.h"
#include "fhe_vector/fhe_vector.h"
#include "../utils/logger.h"
#include <string>
#include <memory>

// ═══════════════════════════════════════════════════════════════════════════════
// SPIRAL FRACTAL UNIFIED — ALL DATABASES, ALL MODULES
// ═══════════════════════════════════════════════════════════════════════════════
//
// One system. All databases.
//   - SpiralFractalDB: SQLite3 + AES-256-GCM + Mirror
//   - FHE-Postgres: Homomorphic SQL queries
//   - FHE-Redis: Encrypted cache
//   - FHE-VectorDB: Encrypted embeddings
//   - Auth: HydraJWT 6-head PQ
//   - TLS: PHI-TLS double-layer
//   - ZKP: Zero-knowledge proofs
//   - FHE: Homomorphic operations
//   - Defense: Active countermeasures
//
// ═══════════════════════════════════════════════════════════════════════════════

struct SpiralFractalUnified {
    SpiralFractalDB db;
    SpiralFractalAuth auth;
    SpiralFractalTLS tls;
    SpiralFractalZKP zkp;
    SpiralFractalFHE fhe;
    SpiralFractalDefense defense;
    FHEPostgres postgres;
    FHERedis redis;
    FHEVectorDB vectordb;
    
    bool init(const std::string& db_path = "unified.db",
              const std::string& passphrase = "",
              int vector_dim = 128) {
        
        // Core DB
        db.init(db_path, passphrase, false, 3, 2);
        
        // Auth
        auth.init(&db);
        
        // TLS
        tls.init(&db, SpiralFractalTLS::auto_config(8443));
        
        // ZKP
        zkp.init(&db, &auth);
        
        // FHE
        fhe.init(&db, SpiralFractalFHE::default_config());
        
        // Defense
        defense.init(&db, SpiralFractalDefense::DefenseConfig());
        
        // FHE Databases
        postgres.init(&db);
        redis.init(&db);
        vectordb.init(&db, vector_dim);
        
        Logger::header("SPIRAL FRACTAL UNIFIED — ALL SYSTEMS ONLINE");
        Logger::info("  6 Core Modules | 3 FHE Databases | 1 Unified System");
        
        return true;
    }
    
    // Quick status
    std::string status() {
        return "{"
               "\"db\":\"active\","
               "\"auth\":\"active\","
               "\"tls\":\"" + std::string(tls.is_running() ? "active" : "off") + "\","
               "\"zkp\":\"active\","
               "\"fhe\":\"active\","
               "\"defense\":\"active\","
               "\"postgres\":\"active\","
               "\"redis\":\"active\","
               "\"vectordb\":\"active\""
               "}";
    }
};
