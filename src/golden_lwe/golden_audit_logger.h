#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>
#include <vector>
#include <mutex>
#include <atomic>
#include <iomanip>

namespace GoldenAuditLogger {

// Immutable Audit Entry
struct AuditEntry {
    std::string timestamp;
    std::string level;
    std::string message;
    uint64_t sequence;
    std::string hash;
};

// Immutable Audit Trail
class AuditTrail {
private:
    std::vector<AuditEntry> entries;
    std::mutex mtx;
    std::atomic<uint64_t> sequence_counter{0};
    std::string chain_hash;

    std::string compute_hash(const AuditEntry& entry) {
        // Simple hash: combine timestamp, level, message, sequence
        std::string combined = entry.timestamp + entry.level + entry.message + std::to_string(entry.sequence);
        uint64_t hash = 5381;
        for (char c : combined) {
            hash = ((hash << 5) + hash) + c;
        }
        // Isama ang nakaraang hash para sa immutability
        hash ^= std::hash<std::string>{}(chain_hash);
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << hash;
        return ss.str();
    }

public:
    void append(const std::string& level, const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx);

        AuditEntry entry;
        entry.timestamp = current_timestamp();
        entry.level = level;
        entry.message = message;
        entry.sequence = sequence_counter++;
        entry.hash = compute_hash(entry);

        entries.push_back(entry);
        chain_hash = entry.hash;
    }

    std::string current_timestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        ss << "." << std::setw(3) << std::setfill('0') << ms.count();
        return ss.str();
    }

    std::vector<AuditEntry> get_entries() const {
        return entries;
    }

    std::string get_chain_hash() const {
        return chain_hash;
    }

    void verify_integrity() const {
        std::string prev_hash = "";
        for (const auto& entry : entries) {
            // I-recompute ang hash para sa bawat entry
            std::string combined = entry.timestamp + entry.level + entry.message + std::to_string(entry.sequence);
            uint64_t hash = 5381;
            for (char c : combined) {
                hash = ((hash << 5) + hash) + c;
            }
            hash ^= std::hash<std::string>{}(prev_hash);

            std::stringstream ss;
            ss << std::hex << std::setw(16) << std::setfill('0') << hash;

            if (ss.str() != entry.hash) {
                std::cerr << "[AUDIT] INTEGRITY VIOLATION AT SEQUENCE " << entry.sequence << "\n";
                return;
            }
            prev_hash = entry.hash;
        }
        std::cout << "[AUDIT] Integrity verified: " << entries.size() << " entries\n";
    }
};

// Global Audit Trail
static AuditTrail g_audit_trail;

class Logger {
public:
    static void info(const std::string& msg) {
        std::string timestamp = g_audit_trail.current_timestamp();
        std::cout << "[" << timestamp << "] [INFO] " << msg << "\n";
        g_audit_trail.append("INFO", msg);
    }

    static void error(const std::string& msg) {
        std::string timestamp = g_audit_trail.current_timestamp();
        std::cerr << "[" << timestamp << "] [ERROR] " << msg << "\n";
        g_audit_trail.append("ERROR", msg);
    }

    static void warn(const std::string& msg) {
        std::string timestamp = g_audit_trail.current_timestamp();
        std::cout << "[" << timestamp << "] [WARN] " << msg << "\n";
        g_audit_trail.append("WARN", msg);
    }

    static void audit(const std::string& msg) {
        std::string timestamp = g_audit_trail.current_timestamp();
        std::cout << "[" << timestamp << "] [AUDIT] " << msg << "\n";
        g_audit_trail.append("AUDIT", msg);
    }

    static AuditTrail& get_audit_trail() {
        return g_audit_trail;
    }
};

} // namespace GoldenAuditLogger
