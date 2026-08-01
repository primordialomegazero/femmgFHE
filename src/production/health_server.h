#pragma once
#include "../utils/logger.h"
#include "../database/spiral_fractal_db.h"
#include "../database/spiral_fractal_auth.h"
#include "../database/spiral_fractal_defense.h"
#include <string>
#include <sstream>
#include <ctime>

// ═══════════════════════════════════════════════════════════════════════════════
// HEALTH CHECK SERVER — /health /ready /metrics endpoints
// ═══════════════════════════════════════════════════════════════════════════════

struct HealthServer {
    SpiralFractalDB* db;
    SpiralFractalAuth* auth;
    SpiralFractalDefense* defense;
    time_t start_time;
    
    bool init(SpiralFractalDB* database, SpiralFractalAuth* auth_layer = nullptr,
              SpiralFractalDefense* defense_layer = nullptr) {
        db = database;
        auth = auth_layer;
        defense = defense_layer;
        start_time = time(0);
        return true;
    }
    
    // GET /health — basic liveness
    std::string health() {
        std::stringstream ss;
        ss << "{"
           << "\"status\":\"healthy\""
           << ",\"uptime\":" << (time(0) - start_time)
           << ",\"timestamp\":" << time(0)
           << "}";
        return ss.str();
    }
    
    // GET /ready — readiness check (DB + Auth + Defense)
    std::string ready() {
        bool db_ok = (db != nullptr);
        bool auth_ok = (auth != nullptr && auth->initialized);
        bool defense_ok = (defense != nullptr && defense->active.load());
        bool all_ok = db_ok && auth_ok && defense_ok;
        
        std::stringstream ss;
        ss << "{"
           << "\"status\":\"" << (all_ok ? "ready" : "not_ready") << "\""
           << ",\"checks\":{"
           << "\"database\":" << (db_ok ? "true" : "false") << ","
           << "\"auth\":" << (auth_ok ? "true" : "false") << ","
           << "\"defense\":" << (defense_ok ? "true" : "false")
           << "}"
           << ",\"uptime\":" << (time(0) - start_time)
           << "}";
        return ss.str();
    }
    
    // GET /metrics — Prometheus-compatible metrics
    std::string metrics() {
        std::stringstream ss;
        ss << "# HELP femmgfhe_uptime_seconds System uptime\n";
        ss << "# TYPE femmgfhe_uptime_seconds gauge\n";
        ss << "femmgfhe_uptime_seconds " << (time(0) - start_time) << "\n";
        
        if (db) {
            ss << "# HELP femmgfhe_db_writes_total Total writes\n";
            ss << "# TYPE femmgfhe_db_writes_total counter\n";
            ss << "femmgfhe_db_writes_total " << db->total_writes << "\n";
            
            ss << "# HELP femmgfhe_db_reads_total Total reads\n";
            ss << "# TYPE femmgfhe_db_reads_total counter\n";
            ss << "femmgfhe_db_reads_total " << db->total_reads << "\n";
        }
        
        if (auth) {
            ss << "# HELP femmgfhe_auth_active Active (1=yes)\n";
            ss << "# TYPE femmgfhe_auth_active gauge\n";
            ss << "femmgfhe_auth_active " << (auth->initialized ? 1 : 0) << "\n";
        }
        
        if (defense) {
            ss << "# HELP femmgfhe_defense_attacks_total Total attacks blocked\n";
            ss << "# TYPE femmgfhe_defense_attacks_total counter\n";
            ss << "femmgfhe_defense_attacks_total " << defense->attack_count.load() << "\n";
            
            ss << "# HELP femmgfhe_defense_decoys_total Total honeypots deployed\n";
            ss << "# TYPE femmgfhe_defense_decoys_total counter\n";
            ss << "femmgfhe_defense_decoys_total " << defense->decoys_deployed.load() << "\n";
        }
        
        return ss.str();
    }
};
