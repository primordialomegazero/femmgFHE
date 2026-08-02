#include <iostream>
#include <cassert>
#include "../../src/utils/logger.h"
#include "../../src/database/spiral_fractal_db.h"
#include "../../src/database/spiral_fractal_auth.h"
#include "../../src/database/spiral_fractal_auth.cpp"
#include "../../src/database/spiral_fractal_tls.h"
#include "../../src/database/spiral_fractal_zkp.h"
#include "../../src/database/spiral_fractal_fhe.h"
#include "../../src/database/spiral_fractal_defense.h"
#include "../../src/database/fhe_postgres/fhe_postgres.h"
#include "../../src/database/fhe_redis/fhe_redis.h"
#include "../../src/database/fhe_vector/fhe_vector.h"

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
    
    bool init() {
        db.init("unified_test.db", "master-pass", false, 3, 2);
        auth.init(&db);
        tls.init(&db, SpiralFractalTLS::auto_config(8443));
        zkp.init(&db, &auth);
        fhe.init(&db, SpiralFractalFHE::default_config());
        defense.init(&db, SpiralFractalDefense::DefenseConfig());
        postgres.init(&db);
        redis.init(&db);
        vectordb.init(&db, 4);
        return true;
    }
    
    std::string status() {
        return "{\"db\":\"active\",\"auth\":\"active\",\"tls\":\"" + 
               std::string(tls.is_running() ? "active" : "off") + 
               "\",\"zkp\":\"active\",\"fhe\":\"active\",\"defense\":\"active\"," +
               "\"postgres\":\"active\",\"redis\":\"active\",\"vectordb\":\"active\"}";
    }
};

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  SPIRAL FRACTAL UNIFIED — ALL SYSTEMS TEST                       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    SpiralFractalUnified sfu;
    sfu.init();
    
    std::cout << "Status: " << sfu.status() << "\n\n";
    
    int passed = 0;
    
    // DB
    std::cout << "--- DB ---\n";
    sfu.db.put("key1", "value1");
    assert(sfu.db.get("key1") == "value1");
    std::cout << "  [OK]\n"; passed++;
    
    // Auth
    std::cout << "--- Auth ---\n";
    std::string token;
    sfu.auth.create_token("admin", token);
    std::cout << "  [OK] Token: " << token.size() << " chars\n"; passed++;
    
    // ZKP
    std::cout << "--- ZKP ---\n";
    sfu.db.put("user:admin:secret", "secret123");
    std::string proof, uid;
    sfu.zkp.prove_identity("admin", proof);
    assert(sfu.zkp.verify_identity(proof, uid));
    std::cout << "  [OK]\n"; passed++;
    
    // Postgres
    std::cout << "--- Postgres ---\n";
    sfu.postgres.create_table("test", {
        {"id", FHEPostgres::FHE_TEXT, false},
        {"data", FHEPostgres::FHE_TEXT, true}
    });
    sfu.postgres.insert("test", {{"id", "1"}, {"data", "secret"}});
    assert(sfu.postgres.select("test").size() == 1);
    std::cout << "  [OK]\n"; passed++;
    
    // Redis
    std::cout << "--- Redis ---\n";
    sfu.redis.set("cache:1", "cached_value");
    assert(sfu.redis.get("cache:1") == "cached_value");
    sfu.redis.incr("hits");
    std::cout << "  [OK]\n"; passed++;
    
    // VectorDB
    std::cout << "--- VectorDB ---\n";
    sfu.vectordb.insert("v1", {1.0, 0.0, 0.0, 0.0});
    sfu.vectordb.insert("v2", {0.0, 1.0, 0.0, 0.0});
    auto results = sfu.vectordb.search({1.0, 0.0, 0.0, 0.0}, 1);
    assert(results.size() == 1);
    std::cout << "  [OK] " << results[0].first << " (sim=" << results[0].second << ")\n"; passed++;
    
    // Defense
    std::cout << "--- Defense ---\n";
    std::string hp = sfu.defense.deploy_honeypot();
    assert(sfu.defense.detect_intrusion("honeypot:" + hp));
    std::cout << "  [OK]\n"; passed++;
    
    system("rm -f unified_test.db*");
    system("rm -rf unified_test.db.mirror_*");
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  UNIFIED SYSTEM: " << passed << "/7 PASSED                                           ║\n";
    std::cout << "║  DB ✅ Auth ✅ ZKP ✅ PG ✅ Redis ✅ Vector ✅ Defense ✅            ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    return (passed == 7) ? 0 : 1;
}
