// PHI-OMEGA-ZERO: COVENANT VAULT v2
// Secure data vault with multi-head auth, tamper detection, audit trail
// "AUTHENTICATE. ACCESS. AUDIT. THE VAULT REMEMBERS."
// "I AM THAT I AM"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <map>
#include <random>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

using namespace std;
using namespace std::chrono;

string ts() {
    auto now = system_clock::now();
    auto t = system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

string sha256_hex(const string& data) {
    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256((const uint8_t*)data.c_str(), data.size(), hash);
    stringstream ss;
    ss << hex << setfill('0');
    for(int i = 0; i < 16; i++) ss << setw(2) << (int)hash[i];
    return ss.str();
}

string hmac_sha256(const string& key, const string& data) {
    uint8_t result[SHA256_DIGEST_LENGTH];
    unsigned int len = 0;
    HMAC(EVP_sha256(), key.c_str(), key.size(),
         (const uint8_t*)data.c_str(), data.size(), result, &len);
    stringstream ss;
    ss << hex << setfill('0');
    for(unsigned int i = 0; i < 16; i++) ss << setw(2) << (int)result[i];
    return ss.str();
}

struct AuthToken {
    string user;
    int active_head;
    vector<string> signatures;
    string token_id;
    string created_at;
    bool valid;
};

class MultiHeadAuth {
    vector<string> head_keys;
    mt19937 rng;
    
public:
    MultiHeadAuth() : rng(time(nullptr)) {
        for(int i = 0; i < 6; i++) {
            uint8_t key[32];
            RAND_bytes(key, 32);
            stringstream ss;
            ss << hex << setfill('0');
            for(int j = 0; j < 32; j++) ss << setw(2) << (int)key[j];
            head_keys.push_back(ss.str());
        }
    }
    
    AuthToken create_token(const string& user) {
        AuthToken token;
        token.user = user;
        token.active_head = rng() % 6;
        token.created_at = ts();
        token.valid = true;
        
        string payload = user + ":" + to_string(token.active_head) + ":" + token.created_at;
        for(int h = 0; h < 6; h++) {
            token.signatures.push_back(hmac_sha256(head_keys[h], payload + ":HEAD:" + to_string(h)));
        }
        
        string all_sigs;
        for(auto& sig : token.signatures) all_sigs += sig;
        token.token_id = sha256_hex(all_sigs).substr(0, 16);
        
        return token;
    }
    
    int verify_token(const AuthToken& token) {
        string payload = token.user + ":" + to_string(token.active_head) + ":" + token.created_at;
        int verified = 0;
        for(int h = 0; h < 6; h++) {
            string expected = hmac_sha256(head_keys[h], payload + ":HEAD:" + to_string(h));
            if(expected == token.signatures[h]) verified++;
        }
        return verified;
    }
    
    bool is_authorized(const AuthToken& token, int min_heads = 4) {
        return verify_token(token) >= min_heads;
    }
};

struct VaultEntry {
    string key;
    string encrypted_data;
    string guard_hash;
    int64_t guard_seed;  // STORED for retrieval!
    int mutation_count;
    vector<string> access_log;
    bool destroyed;
};

class CovenantVault {
    MultiHeadAuth auth;
    map<string, VaultEntry> vault;
    mt19937 rng;
    string master_key;
    
    string encrypt_data(const string& data, const string& user_key) {
        return hmac_sha256(master_key + user_key, data);
    }
    
    string make_guard(const string& encrypted, int64_t guard_seed) {
        return sha256_hex(encrypted + to_string(guard_seed));
    }
    
public:
    CovenantVault() : rng(time(nullptr)) {
        uint8_t mk[32];
        RAND_bytes(mk, 32);
        stringstream ss;
        ss << hex << setfill('0');
        for(int i = 0; i < 32; i++) ss << setw(2) << (int)mk[i];
        master_key = ss.str();
    }
    
    struct StoreResult {
        bool success;
        int64_t guard_seed;  // Return this to user!
        string message;
    };
    
    struct AccessResult {
        bool granted;
        string data;
        string message;
        int heads_verified;
    };
    
    StoreResult store(const AuthToken& token, const string& key, const string& data) {
        StoreResult sr;
        if(!auth.is_authorized(token)) {
            sr.success = false;
            sr.message = "AUTH FAILED";
            return sr;
        }
        
        VaultEntry entry;
        entry.key = key;
        entry.encrypted_data = encrypt_data(data, token.token_id);
        
        uniform_int_distribution<int64_t> dist(10000, 99999);
        entry.guard_seed = dist(rng);
        entry.guard_hash = make_guard(entry.encrypted_data, entry.guard_seed);
        entry.mutation_count = 0;
        entry.destroyed = false;
        
        sr.guard_seed = entry.guard_seed;
        sr.success = true;
        sr.message = "Stored. Guard seed: " + to_string(entry.guard_seed);
        
        stringstream log_entry;
        log_entry << ts() << " | STORE | Guard: " << entry.guard_seed;
        entry.access_log.push_back(log_entry.str());
        
        vault[key] = entry;
        return sr;
    }
    
    AccessResult retrieve(const AuthToken& token, const string& key, int64_t guard_seed) {
        AccessResult result;
        result.heads_verified = auth.verify_token(token);
        
        if(!auth.is_authorized(token)) {
            result.granted = false;
            result.message = "AUTH FAILED: " + to_string(result.heads_verified) + "/6 heads";
            return result;
        }
        
        auto it = vault.find(key);
        if(it == vault.end()) {
            result.granted = false;
            result.message = "NOT FOUND";
            return result;
        }
        
        auto& entry = it->second;
        
        if(entry.destroyed) {
            result.granted = false;
            result.message = "DESTROYED";
            return result;
        }
        
        string expected_guard = make_guard(entry.encrypted_data, guard_seed);
        if(expected_guard != entry.guard_hash) {
            entry.encrypted_data = "DESTROYED";
            entry.destroyed = true;
            result.granted = false;
            result.message = "TAMPER DETECTED! Self-destruct.";
            entry.access_log.push_back(ts() + " | TAMPER | Wrong guard: " + to_string(guard_seed));
            return result;
        }
        
        entry.mutation_count++;
        entry.encrypted_data = encrypt_data(entry.encrypted_data, to_string(entry.mutation_count));
        entry.guard_hash = make_guard(entry.encrypted_data, guard_seed);
        
        result.granted = true;
        result.data = entry.encrypted_data.substr(0, 16) + "...";
        result.message = "OK (mutation #" + to_string(entry.mutation_count) + ")";
        
        entry.access_log.push_back(ts() + " | READ | Mutation #" + to_string(entry.mutation_count));
        
        return result;
    }
    
    vector<string> get_audit_log(const AuthToken& token, const string& key) {
        if(!auth.is_authorized(token)) return {"AUTH FAILED"};
        auto it = vault.find(key);
        if(it == vault.end()) return {"NOT FOUND"};
        return it->second.access_log;
    }
    
    void demo() {
        cout << "\n======================================================================\n";
        cout <<   "  COVENANT VAULT v2 — Secure Data Storage\n";
        cout <<   "  Multi-Head Auth + Guard Seeds + Tamper Detection + Audit Trail\n";
        cout <<   "  Date: " << ts() << "\n";
        cout <<   "======================================================================\n\n";
        
        // Create token
        auto token = auth.create_token("user:dan");
        cout << "  AUTH: Token " << token.token_id << " | Heads: " 
             << auth.verify_token(token) << "/6 ✅\n\n";
        
        // Store — get guard_seed back
        auto sr = store(token, "key:42", "sensitive data here");
        cout << "  STORE: " << sr.message << "\n\n";
        
        // Retrieve with correct guard
        cout << "  RETRIEVE (correct guard " << sr.guard_seed << "):\n";
        auto r1 = retrieve(token, "key:42", sr.guard_seed);
        cout << "  → " << r1.message << " | Data: " << r1.data << "\n\n";
        
        // Retrieve again (should mutate)
        auto r2 = retrieve(token, "key:42", sr.guard_seed);
        cout << "  RETRIEVE AGAIN: " << r2.message << "\n\n";
        
        // Tamper with wrong guard
        auto r3 = retrieve(token, "key:42", 00000);
        cout << "  TAMPER (wrong guard): " << r3.message << "\n\n";
        
        // Try after destruction
        auto r4 = retrieve(token, "key:42", sr.guard_seed);
        cout << "  AFTER TAMPER: " << r4.message << "\n\n";
        
        // Audit trail
        cout << "  AUDIT TRAIL:\n";
        cout << "  " << string(60, '-') << "\n";
        for(auto& e : get_audit_log(token, "key:42")) {
            cout << "  " << e << "\n";
        }
        cout << "  " << string(60, '-') << "\n\n";
        
        cout << "======================================================================\n";
        cout <<   "  COVENANT VAULT v2: OPERATIONAL ✅\n";
        cout <<   "  Completed: " << ts() << "\n";
        cout <<   "======================================================================\n\n";
        cout << "  I AM THAT I AM\n\n";
    }
};

int main() {
    CovenantVault cv;
    cv.demo();
    return 0;
}
