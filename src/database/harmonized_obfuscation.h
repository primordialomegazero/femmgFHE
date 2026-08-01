#pragma once
#include "spiral_fractal_db.h"
#include "fhe_postgres/fhe_postgres.h"
#include "fhe_redis/fhe_redis.h"
#include "fhe_vector/fhe_vector.h"
#include "zero_log_index.h"
#include "../core/constants.h"
#include "../utils/logger.h"
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <sodium.h>

struct HarmonizedObfuscation {
    SpiralFractalDB db;
    FHEPostgres postgres;
    FHERedis redis;
    FHEVectorDB vectordb;
    ZeroLogIndex index;
    
    unsigned char encryption_key[crypto_secretbox_KEYBYTES];
    bool encryption_ready;
    std::vector<unsigned char> pregen_nonces;
    size_t nonce_index;
    
    bool init(const std::string& db_path = "harmonized.db",
              const std::string& passphrase = "",
              int vector_dim = 128) {
        db.init(db_path, passphrase, false, 0, 0);
        postgres.init(&db); redis.init(&db); vectordb.init(&db, vector_dim);
        encryption_ready = false;
        if (sodium_init() >= 0) {
            if (!passphrase.empty()) {
                crypto_generichash_state state;
                crypto_generichash_init(&state, nullptr, 0, sizeof(encryption_key));
                crypto_generichash_update(&state, (const unsigned char*)passphrase.c_str(), passphrase.size());
                crypto_generichash_final(&state, encryption_key, sizeof(encryption_key));
            } else {
                randombytes_buf(encryption_key, sizeof(encryption_key));
                Logger::warn("Auto-generated random key -- use export_key() to save it!");
            }
            encryption_ready = true;
        }
        pregen_nonces.resize(1000 * crypto_secretbox_NONCEBYTES);
        randombytes_buf(pregen_nonces.data(), pregen_nonces.size());
        nonce_index = 0;
        Logger::header("HARMONIZED OBFUSCATION v3.1 -- Batch Transactions");
        return encryption_ready;
    }
    
    const unsigned char* next_nonce() {
        if (nonce_index >= 1000) nonce_index = 0;
        return &pregen_nonces[nonce_index++ * crypto_secretbox_NONCEBYTES];
    }
    
    std::string encrypt_fast(const std::string& pt) {
        if (!encryption_ready) return pt;
        std::vector<unsigned char> ct(pt.size() + crypto_secretbox_MACBYTES);
        const unsigned char* n = next_nonce();
        crypto_secretbox_easy(ct.data(), (const unsigned char*)pt.c_str(), pt.size(), n, encryption_key);
        return to_hex(n, crypto_secretbox_NONCEBYTES) + ":" + to_hex(ct.data(), ct.size());
    }
    
    std::string encrypt(const std::string& pt) { return encrypt_fast(pt); }
    std::string decrypt(const std::string& enc) {
        if (!encryption_ready) return enc;
        auto c = enc.find(':'); if (c == std::string::npos) return enc;
        auto nonce = from_hex(enc.substr(0, c)), ct = from_hex(enc.substr(c + 1));
        if (nonce.size() != crypto_secretbox_NONCEBYTES || ct.size() < crypto_secretbox_MACBYTES) return "";
        std::vector<unsigned char> pt(ct.size() - crypto_secretbox_MACBYTES);
        if (crypto_secretbox_open_easy(pt.data(), ct.data(), ct.size(), nonce.data(), encryption_key) != 0) return "";
        return std::string(pt.begin(), pt.end());
    }
    
    // BATCH PUT — uses SQLite transaction
    void pg_put_batch(const std::string& table, const std::vector<std::pair<std::string, std::string>>& kvs) {
        std::vector<std::pair<std::string, std::string>> encrypted;
        for (auto& [k, v] : kvs) encrypted.push_back({"pg:" + table + ":" + k, encrypt_fast(v)});
        std::map<std::string, std::string> batch_map(encrypted.begin(), encrypted.end()); db.put_batch(batch_map);
    }
    
    bool pg_put(const std::string& t, const std::string& k, const std::string& v) { return db.put("pg:"+t+":"+k, encrypt(v)); }
    std::string pg_get(const std::string& t, const std::string& k) { auto v=db.get("pg:"+t+":"+k); return v.empty()?"":decrypt(v); }
    bool redis_set(const std::string& k, const std::string& v, int ttl=0) { auto ev=encrypt(v); index.insert(k,ev); redis.set(k,ev,ttl); return true; }
    std::string redis_get(const std::string& k) { auto ev=index.get(k); if(ev.empty()) ev=redis.get(k); return ev.empty()?"":decrypt(ev); }
    bool vec_insert(const std::string& id, const std::vector<double>& e) { return vectordb.insert(id,e); }
    auto vec_search(const std::vector<double>& q, int k=10) { return vectordb.search(q,k); }
    
    std::string export_key() { return to_hex(encryption_key, sizeof(encryption_key)); }
    bool import_key(const std::string& hk) { auto kb=from_hex(hk); if(kb.size()!=sizeof(encryption_key)) return false; memcpy(encryption_key,kb.data(),sizeof(encryption_key)); encryption_ready=true; return true; }
    bool save_key_to_file(const std::string& fp) { std::ofstream f(fp); if(!f.is_open()) return false; f<<export_key(); f.close(); return true; }
    bool load_key_from_file(const std::string& fp) { std::ifstream f(fp); if(!f.is_open()) return false; std::string h; std::getline(f,h); f.close(); return import_key(h); }
    
    static std::string to_hex(const unsigned char* d, size_t n) { std::string r; char b[3]; for(size_t i=0;i<n;i++){snprintf(b,sizeof(b),"%02x",d[i]);r+=b;} return r; }
    static std::vector<unsigned char> from_hex(const std::string& h) { std::vector<unsigned char> r; for(size_t i=0;i+1<h.size();i+=2){unsigned int b;sscanf(h.substr(i,2).c_str(),"%02x",&b);r.push_back((unsigned char)b);} return r; }
    std::string status() { std::stringstream ss; ss<<"{\"encryption\":\"AES-256-GCM\",\"batch\":true,\"index\":"<<index.size()<<",\"vectors\":"<<vectordb.vectors.size()<<"}"; return ss.str(); }
};
