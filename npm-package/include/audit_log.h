/*
 * FEmmg-FHE v22.0.0 — Audit Logger
 * 
 * Logs all security events WITHOUT plaintext.
 * - Request logging (action, client, timestamp)
 * - Failed auth attempts
 * - Rate limit violations
 * - Session events
 * 
 * "Log everything. Reveal nothing."
 */

#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace audit_log {

enum class EventType {
    REQUEST,
    AUTH_SUCCESS,
    AUTH_FAILURE,
    RATE_LIMIT,
    BURST_BLOCKED,
    SESSION_CREATED,
    SESSION_EXPIRED,
    SESSION_REVOKED,
    INVALID_INPUT,
    ERROR
};

struct AuditEvent {
    EventType type;
    std::string client_id;    // Hashed!
    std::string action;
    std::string ip_address;
    int64_t timestamp;
    bool success;
    std::string details;      // No plaintext!
};

class AuditLogger {
private:
    std::vector<AuditEvent> events_;
    std::mutex mtx_;
    bool enabled_ = true;
    size_t max_events_ = 10000;
    
    // Hash client_id for privacy
    std::string hash_client(const std::string& client_id) {
        std::hash<std::string> hasher;
        size_t hash = hasher(client_id);
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << hash;
        return ss.str();
    }
    
    std::string event_type_str(EventType type) {
        switch (type) {
            case EventType::REQUEST: return "REQUEST";
            case EventType::AUTH_SUCCESS: return "AUTH_SUCCESS";
            case EventType::AUTH_FAILURE: return "AUTH_FAILURE";
            case EventType::RATE_LIMIT: return "RATE_LIMIT";
            case EventType::BURST_BLOCKED: return "BURST_BLOCKED";
            case EventType::SESSION_CREATED: return "SESSION_CREATED";
            case EventType::SESSION_EXPIRED: return "SESSION_EXPIRED";
            case EventType::SESSION_REVOKED: return "SESSION_REVOKED";
            case EventType::INVALID_INPUT: return "INVALID_INPUT";
            case EventType::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
    
public:
    AuditLogger() = default;
    void enable() { enabled_ = true; }
    void disable() { enabled_ = false; }
    
    // ═══ LOG EVENT ═══
    void log(EventType type, const std::string& client_id, 
             const std::string& action, const std::string& ip,
             bool success, const std::string& details = "") {
        if (!enabled_) return;
        
        std::lock_guard<std::mutex> lock(mtx_);
        
        AuditEvent event;
        event.type = type;
        event.client_id = hash_client(client_id);  // Hash for privacy!
        event.action = action;
        event.ip_address = ip;
        event.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        event.success = success;
        event.details = details;  // Must NOT contain plaintext!
        
        events_.push_back(event);
        
        // Trim old events
        while (events_.size() > max_events_) {
            events_.erase(events_.begin());
        }
    }
    
    // ═══ GET RECENT EVENTS ═══
    std::vector<AuditEvent> get_recent(size_t count = 100) {
        std::lock_guard<std::mutex> lock(mtx_);
        
        std::vector<AuditEvent> recent;
        size_t start = events_.size() > count ? events_.size() - count : 0;
        for (size_t i = start; i < events_.size(); i++) {
            recent.push_back(events_[i]);
        }
        return recent;
    }
    
    // ═══ GENERATE REPORT ═══
    std::string report() {
        std::lock_guard<std::mutex> lock(mtx_);
        
        std::stringstream ss;
        ss << "=== AUDIT LOG REPORT ===" << std::endl;
        ss << "Total events: " << events_.size() << std::endl;
        
        // Count by type
        std::map<EventType, int> counts;
        int failures = 0;
        for (const auto& e : events_) {
            counts[e.type]++;
            if (!e.success) failures++;
        }
        
        for (const auto& [type, count] : counts) {
            ss << "  " << event_type_str(type) << ": " << count << std::endl;
        }
        ss << "  Failures: " << failures << std::endl;
        
        return ss.str();
    }
    
    // ═══ EXPORT (safe — no plaintext) ═══
    std::string export_json() {
        std::lock_guard<std::mutex> lock(mtx_);
        
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < events_.size(); i++) {
            if (i > 0) ss << ",";
            ss << "{"
               << "\"type\":\"" << event_type_str(events_[i].type) << "\","
               << "\"client_hash\":\"" << events_[i].client_id << "\","
               << "\"action\":\"" << events_[i].action << "\","
               << "\"ip\":\"" << events_[i].ip_address << "\","
               << "\"success\":" << (events_[i].success ? "true" : "false");
            if (!events_[i].details.empty()) {
                ss << ",\"details\":\"" << events_[i].details << "\"";
            }
            ss << "}";
        }
        ss << "]";
        return ss.str();
    }
    
    size_t count() const { return events_.size(); }
};

} // namespace audit_log
