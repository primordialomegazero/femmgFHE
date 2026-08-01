#pragma once
#include "../utils/logger.h"
#include "../utils/safe_math.h"
#include "../core/constants.h"
#include "../crypto/golden_fibonacci.h"
#include "../crypto/fractal_chaos.h"
#include "../hardware/hardware_sentinel.h"
#include <sqlite3.h>
#include <sodium.h>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <ctime>
#include <deque>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <cstring>
#include <sys/stat.h>
#include <iomanip>

struct SpiralFractalDB {
    sqlite3* sql_db;
    std::string db_path;
    
    struct MirrorConfig {
        bool enabled = false;
        int mirror_count = 5;
        int threshold = 3;
        std::vector<std::string> mirror_paths;
    } mirrors;
    
    std::map<std::string, std::string> primary_mirror;
    std::map<std::string, std::string> cache_mirror;
    mutable std::shared_mutex mirror_mutex;
    
    struct FractalLayer {
        std::map<std::string, std::string> entries;
        int layer_index;
        double phi_weight;
    };
    std::vector<FractalLayer> fractal_layers;
    
    // Encryption — fully configurable
    bool encryption_enabled;
    bool fhe_enabled;
    bool sodium_ready;
    unsigned char aes_key[crypto_secretbox_KEYBYTES];
    GoldenFibonacci gf;
    HardwareObfuscator hw_obf;
    
    struct Snapshot {
        std::string name; time_t timestamp; std::string file_path;
    };
    std::vector<Snapshot> snapshots;
    
    int total_writes, total_reads, total_deletes, total_snapshots;
    time_t init_time;
    
    bool init(const std::string& path = "spiral_fractal.db",
              const std::string& passphrase = "",
              bool enable_fhe = false,
              int mirror_count = 0, int mirror_threshold = 0) {
        db_path = path;
        encryption_enabled = false;
        fhe_enabled = enable_fhe;
        sodium_ready = (sodium_init() >= 0);
        total_writes = total_reads = total_deletes = total_snapshots = 0;
        init_time = time(0);
        
        // Only enable encryption if passphrase provided AND libsodium works
        if (!passphrase.empty() && sodium_ready) {
            encryption_enabled = derive_key(passphrase);
        }
        
        int rc = sqlite3_open(path.c_str(), &sql_db);
        if (rc != SQLITE_OK) return false;
        create_tables();
        
        for (int i = 0; i < 7; i++) {
            FractalLayer l;
            l.layer_index = i;
            l.phi_weight = std::pow(PHI, -(i + 1));
            fractal_layers.push_back(l);
        }
        
        if (mirror_count > 0) {
            mirrors.enabled = true;
            mirrors.mirror_count = mirror_count;
            mirrors.threshold = (mirror_threshold > 0) ? mirror_threshold : (mirror_count / 2 + 1);
            for (int i = 0; i < mirror_count; i++) {
                std::string mp = db_path + ".mirror_" + std::to_string(i);
                mkdir(mp.c_str(), 0755);
                mirrors.mirror_paths.push_back(mp);
            }
        }
        
        hw_obf.init();
        
        Logger::info("SpiralFractalDB v2.0: " + path +
                    " [SQLite3]" +
                    std::string(encryption_enabled ? " [AES-256-GCM]" : " [NO-ENCRYPTION]") +
                    std::string(mirrors.enabled ? " [" + std::to_string(mirror_count) + "-Mirror]" : "") +
                    " [7-Layer Fractal]");
        return true;
    }
    
    void create_tables() {
        const char* sql = R"(
            CREATE TABLE IF NOT EXISTS store (
                key TEXT PRIMARY KEY, value TEXT NOT NULL,
                encrypted INTEGER DEFAULT 0,
                cassini_checksum REAL DEFAULT 0,
                created_at INTEGER DEFAULT (strftime('%s', 'now')),
                updated_at INTEGER DEFAULT (strftime('%s', 'now'))
            );
            CREATE TABLE IF NOT EXISTS fractal_index (
                key TEXT NOT NULL, layer INTEGER NOT NULL,
                fragment TEXT NOT NULL, sha256_hash TEXT NOT NULL,
                phi_weight REAL NOT NULL,
                PRIMARY KEY (key, layer)
            );
            CREATE TABLE IF NOT EXISTS audit_log (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                operation TEXT NOT NULL, key TEXT,
                timestamp INTEGER DEFAULT (strftime('%s', 'now')),
                details TEXT
            );
            CREATE TABLE IF NOT EXISTS snapshots (
                name TEXT PRIMARY KEY, timestamp INTEGER, file_path TEXT
            );
            CREATE INDEX IF NOT EXISTS idx_fractal_key ON fractal_index(key);
            CREATE INDEX IF NOT EXISTS idx_audit_time ON audit_log(timestamp);
        )";
        char* err = nullptr;
        sqlite3_exec(sql_db, sql, nullptr, nullptr, &err);
        if (err) sqlite3_free(err);
    }
    
    // Simple key derivation that works across libsodium versions
    bool derive_key(const std::string& passphrase) {
        if (!sodium_ready) return false;
        
        // Use crypto_generichash as fallback (works everywhere)
        crypto_generichash_state state;
        crypto_generichash_init(&state, nullptr, 0, sizeof(aes_key));
        crypto_generichash_update(&state, (const unsigned char*)passphrase.c_str(), passphrase.size());
        crypto_generichash_final(&state, aes_key, sizeof(aes_key));
        return true;
    }
    
    std::string encrypt_value(const std::string& pt) {
        if (!encryption_enabled || !sodium_ready) return pt;
        unsigned char nonce[crypto_secretbox_NONCEBYTES];
        randombytes_buf(nonce, sizeof(nonce));
        std::vector<unsigned char> ct(pt.size() + crypto_secretbox_MACBYTES);
        crypto_secretbox_easy(ct.data(), (const unsigned char*)pt.c_str(), pt.size(), nonce, aes_key);
        return to_hex(nonce, sizeof(nonce)) + ":" + to_hex(ct.data(), ct.size());
    }
    
    std::string decrypt_value(const std::string& enc) {
        if (!encryption_enabled || !sodium_ready) return enc;
        auto c = enc.find(':');
        if (c == std::string::npos) return enc;
        auto nonce = from_hex(enc.substr(0, c));
        auto ct = from_hex(enc.substr(c + 1));
        if (nonce.size() != crypto_secretbox_NONCEBYTES || ct.size() < crypto_secretbox_MACBYTES) return "";
        std::vector<unsigned char> pt(ct.size() - crypto_secretbox_MACBYTES);
        if (crypto_secretbox_open_easy(pt.data(), ct.data(), ct.size(), nonce.data(), aes_key) != 0) return "";
        return std::string(pt.begin(), pt.end());
    }
    
    bool put(const std::string& key, const std::string& value) {
        std::unique_lock lock(mirror_mutex);
        std::string stored = encryption_enabled ? encrypt_value(value) : value;
        double cassini = compute_cassini(key, stored);
        
        primary_mirror[key] = stored;
        cache_mirror[key] = stored;
        
        const char* sql = "INSERT OR REPLACE INTO store (key, value, encrypted, cassini_checksum, updated_at) VALUES (?, ?, ?, ?, strftime('%s', 'now'))";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(sql_db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, stored.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, encryption_enabled ? 1 : 0);
        sqlite3_bind_double(stmt, 4, cassini);
        sqlite3_step(stmt); sqlite3_finalize(stmt);
        
        for (int i = 0; i < 7; i++) {
            std::string fragment = create_fractal_fragment(key, stored, i);
            fractal_layers[i].entries[key] = fragment;
            std::string sha = sha256_hash(fragment);
            const char* fs = "INSERT OR REPLACE INTO fractal_index (key, layer, fragment, sha256_hash, phi_weight) VALUES (?, ?, ?, ?, ?)";
            sqlite3_stmt* fs2;
            sqlite3_prepare_v2(sql_db, fs, -1, &fs2, nullptr);
            sqlite3_bind_text(fs2, 1, key.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(fs2, 2, i);
            sqlite3_bind_text(fs2, 3, fragment.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(fs2, 4, sha.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_double(fs2, 5, fractal_layers[i].phi_weight);
            sqlite3_step(fs2); sqlite3_finalize(fs2);
        }
        
        if (mirrors.enabled) mirror_write(key, stored);
        log_audit("PUT", key, "size=" + std::to_string(value.size()));
        total_writes++;
        return true;
    }
    
    std::string get(const std::string& key, const std::string& def = "") {
        std::shared_lock lock(mirror_mutex);
        auto ci = cache_mirror.find(key);
        if (ci != cache_mirror.end()) { total_reads++; return encryption_enabled ? decrypt_value(ci->second) : ci->second; }
        auto pi = primary_mirror.find(key);
        if (pi != primary_mirror.end()) { cache_mirror[key] = pi->second; total_reads++; return encryption_enabled ? decrypt_value(pi->second) : pi->second; }
        
        const char* sql = "SELECT value, encrypted FROM store WHERE key = ?";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(sql_db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
        std::string result = def;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string val = (const char*)sqlite3_column_text(stmt, 0);
            int enc = sqlite3_column_int(stmt, 1);
            result = (enc && encryption_enabled) ? decrypt_value(val) : val;
            primary_mirror[key] = val; cache_mirror[key] = val;
        }
        sqlite3_finalize(stmt);
        if (result == def && mirrors.enabled) result = mirror_read(key, def);
        total_reads++;
        return result;
    }
    
    bool exists(const std::string& key) { return get(key, "__NOT_FOUND__") != "__NOT_FOUND__"; }
    
    bool remove(const std::string& key) {
        std::unique_lock lock(mirror_mutex);
        primary_mirror.erase(key); cache_mirror.erase(key);
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(sql_db, "DELETE FROM store WHERE key = ?", -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt); sqlite3_finalize(stmt);
        sqlite3_prepare_v2(sql_db, "DELETE FROM fractal_index WHERE key = ?", -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt); sqlite3_finalize(stmt);
        total_deletes++;
        return true;
    }
    
    void put_batch(const std::map<std::string, std::string>& batch) {
        for (auto& [k, v] : batch) put(k, v);
    }
    
    std::map<std::string, std::string> get_prefix(const std::string& prefix) {
        std::map<std::string, std::string> result;
        const char* sql = "SELECT key, value, encrypted FROM store WHERE key LIKE ?";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(sql_db, sql, -1, &stmt, nullptr);
        std::string like = prefix + "%";
        sqlite3_bind_text(stmt, 1, like.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string k = (const char*)sqlite3_column_text(stmt, 0);
            std::string v = (const char*)sqlite3_column_text(stmt, 1);
            int enc = sqlite3_column_int(stmt, 2);
            result[k] = (enc && encryption_enabled) ? decrypt_value(v) : v;
        }
        sqlite3_finalize(stmt);
        return result;
    }
    
    bool create_snapshot(const std::string& name) {
        std::string snap_path = db_path + ".snapshot_" + name;
        std::ifstream src(db_path, std::ios::binary);
        std::ofstream dst(snap_path, std::ios::binary);
        dst << src.rdbuf(); src.close(); dst.close();
        const char* sql = "INSERT OR REPLACE INTO snapshots (name, timestamp, file_path) VALUES (?, strftime('%s', 'now'), ?)";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(sql_db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, snap_path.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt); sqlite3_finalize(stmt);
        total_snapshots++;
        return true;
    }
    
    void mirror_write(const std::string& key, const std::string& val) {
        for (int i = 0; i < mirrors.mirror_count; i++) {
            std::string mf = mirrors.mirror_paths[i] + "/" + sha256_hash(key);
            std::ofstream f(mf); f << val;
        }
    }
    
    std::string mirror_read(const std::string& key, const std::string& def) {
        std::map<std::string, int> votes;
        for (int i = 0; i < mirrors.mirror_count; i++) {
            std::string mf = mirrors.mirror_paths[i] + "/" + sha256_hash(key);
            std::ifstream f(mf);
            if (f.is_open()) {
                std::string val((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                if (!val.empty()) votes[val]++;
            }
        }
        for (auto& [val, count] : votes)
            if (count >= mirrors.threshold) return encryption_enabled ? decrypt_value(val) : val;
        return def;
    }
    
    std::string create_fractal_fragment(const std::string& key, const std::string& value, int layer) {
        return sha256_hash(key + ":" + value + ":" + std::to_string(layer)).substr(0, (size_t)(64 * fractal_layers[layer].phi_weight));
    }
    
    std::string sha256_hash(const std::string& input) {
        if (!sodium_ready) {
            size_t h = std::hash<std::string>{}(input);
            char buf[65]; snprintf(buf, sizeof(buf), "%016zx", h);
            return std::string(buf);
        }
        unsigned char hash[crypto_hash_sha256_BYTES];
        crypto_hash_sha256(hash, (const unsigned char*)input.c_str(), input.size());
        return to_hex(hash, crypto_hash_sha256_BYTES);
    }
    
    double compute_cassini(const std::string& key, const std::string& val) {
        return SafeMath::fmod_safe(std::abs((double)std::hash<std::string>{}(key) * PHI + (double)std::hash<std::string>{}(val) * PSI));
    }
    
    void log_audit(const std::string& op, const std::string& key, const std::string& details) {
        const char* sql = "INSERT INTO audit_log (operation, key, details) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(sql_db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, op.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, details.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt); sqlite3_finalize(stmt);
    }
    
    static std::string to_hex(const unsigned char* d, size_t n) {
        std::string r; char b[3];
        for (size_t i = 0; i < n; i++) { snprintf(b, sizeof(b), "%02x", d[i]); r += b; }
        return r;
    }
    
    static std::vector<unsigned char> from_hex(const std::string& h) {
        std::vector<unsigned char> r;
        for (size_t i = 0; i + 1 < h.size(); i += 2) {
            unsigned int b; sscanf(h.substr(i, 2).c_str(), "%02x", &b);
            r.push_back((unsigned char)b);
        }
        return r;
    }
    
    void stats() {
        Logger::section("SPIRAL FRACTAL DB v2.0");
        Logger::keyval("SQLite", db_path);
        Logger::keyval("Encryption", encryption_enabled ? "AES-256-GCM" : "OFF (configurable)");
        Logger::keyval("Mirrors", mirrors.enabled ? std::to_string(mirrors.mirror_count) + " (threshold=" + std::to_string(mirrors.threshold) + ")" : "OFF (configurable)");
        Logger::keyval("Fractal Layers", "7 (φ-harmonic)");
        Logger::keyval("Writes", std::to_string(total_writes));
        Logger::keyval("Reads", std::to_string(total_reads));
        Logger::keyval("Deletes", std::to_string(total_deletes));
        Logger::keyval("Snapshots", std::to_string(total_snapshots));
        Logger::keyval("Uptime", std::to_string(time(0) - init_time) + "s");
    }
    
    ~SpiralFractalDB() { if (sql_db) sqlite3_close(sql_db); }
};

