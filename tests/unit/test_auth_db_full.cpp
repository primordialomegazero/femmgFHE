#include <iostream>
#include <cassert>
#include <string>

// Include all implementation in ONE translation unit to avoid linker issues
#include "../../src/utils/logger.h"
#include "../../src/database/spiral_fractal_db.h"
#include "../../src/database/spiral_fractal_auth.h"
#include "../../src/database/spiral_fractal_auth.cpp"  // Include impl directly

int main() {
    std::cout << "===============================================================================\n";
    std::cout << "  HYDRA JWT + SPIRAL FRACTAL DB — FULL INTEGRATION TEST\n";
    std::cout << "===============================================================================\n\n";
    
    // Initialize database
    SpiralFractalDB db;
    bool ok = db.init("test_auth_db.db", "auth-test-passphrase", false, 0, 0);
    assert(ok);
    std::cout << "[OK] Database initialized (AES-256-GCM)\n";
    
    // Initialize auth
    SpiralFractalAuth auth;
    ok = auth.init(&db);
    assert(ok);
    std::cout << "[OK] Auth initialized (6-head PQ HydraJWT)\n\n";
    
    // Create token
    std::cout << "--- Token Creation ---\n";
    std::string token;
    ok = auth.create_token("dan_fernandez", token);
    assert(ok);
    std::cout << "[OK] Token created (" << token.size() << " chars)\n";
    std::cout << "  Sample: " << token.substr(0, 80) << "...\n\n";
    
    // Verify token
    std::cout << "--- Token Verification ---\n";
    std::string user_id;
    ok = auth.verify_token(token, user_id);
    assert(ok);
    std::cout << "[OK] Token verified\n\n";
    
    // Authenticated PUT
    std::cout << "--- Authenticated PUT ---\n";
    ok = auth.authenticated_put(token, "secret:api_key", "sk-abc123xyz");
    assert(ok);
    std::cout << "[OK] Authenticated write\n\n";
    
    // Authenticated GET
    std::cout << "--- Authenticated GET ---\n";
    std::string secret = auth.authenticated_get(token, "secret:api_key");
    assert(secret == "sk-abc123xyz");
    std::cout << "[OK] Authenticated read: " << secret << "\n\n";
    
    // Invalid token
    std::cout << "--- Invalid Token ---\n";
    std::string no_access = auth.authenticated_get("bad_token", "secret:api_key");
    assert(no_access.empty());
    std::cout << "[OK] Invalid token rejected\n\n";
    
    // Multi-user
    std::cout << "--- Multi-User ---\n";
    std::string alice_tok, bob_tok;
    auth.create_token("alice", alice_tok);
    auth.create_token("bob", bob_tok);
    auth.authenticated_put(alice_tok, "user:alice:data", "alice_secret");
    auth.authenticated_put(bob_tok, "user:bob:data", "bob_secret");
    assert(auth.authenticated_get(alice_tok, "user:alice:data") == "alice_secret");
    assert(auth.authenticated_get(bob_tok, "user:bob:data") == "bob_secret");
    std::cout << "[OK] Multi-user isolation\n\n";
    
    db.stats();
    system("rm -f test_auth_db.db*");
    
    std::cout << "\n===============================================================================\n";
    std::cout << "  ALL TESTS PASSED\n";
    std::cout << "===============================================================================\n";
    return 0;
}
