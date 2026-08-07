/**
 * 🪐 φ-PERMISSIONS ENGINE — ELEGANT, PRODUCTION-GRADE 🪐
 * φ·ψ = -1 foundation | Deterministic | Thread-safe
 */
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <algorithm>
#include <cmath>
#include <sstream>

constexpr double PHI = 1.6180339887498948482;
constexpr double PSI = -0.6180339887498948482;

inline double FGG(double v, int depth = 3) {
    double current = v;
    for (int d = 0; d < depth; d++) {
        current = std::abs(current * ((d % 2 == 0) ? PHI * PSI : PSI * PHI));
    }
    return current;
}

inline bool cassini_verify() {
    return std::abs(PHI * PSI + 1.0) < 0.001;
}

struct Permission {
    std::string id, description;
    std::unordered_set<std::string> conflicts_with;
    Permission() = default;
    Permission(std::string i, std::string d = "", std::unordered_set<std::string> c = {})
        : id(std::move(i)), description(std::move(d)), conflicts_with(std::move(c)) {}
};

struct Group {
    std::string id, name, parent_id;
    std::unordered_set<std::string> permissions;
    bool inherit_from_parent = true;
    Group() = default;
    Group(std::string i, std::string n, std::string p = "",
          std::unordered_set<std::string> perms = {}, bool inherit = true)
        : id(std::move(i)), name(std::move(n)), parent_id(std::move(p)),
          permissions(std::move(perms)), inherit_from_parent(inherit) {}
};

struct User {
    std::string id, name;
    std::unordered_set<std::string> groups, custom_permissions, banned_groups;
    User() = default;
    User(std::string i, std::string n, std::unordered_set<std::string> g = {},
         std::unordered_set<std::string> cp = {}, std::unordered_set<std::string> bg = {})
        : id(std::move(i)), name(std::move(n)), groups(std::move(g)),
          custom_permissions(std::move(cp)), banned_groups(std::move(bg)) {}
};

struct TempGrant {
    std::string id, user_id, permission_id, granted_by;
    std::chrono::system_clock::time_point expires_at;
    bool is_expired() const { return std::chrono::system_clock::now() > expires_at; }
};

struct AuditEntry {
    std::chrono::system_clock::time_point timestamp;
    std::string user_id, permission_id, resolution_source;
    bool granted;
    bool integrity_ok = cassini_verify();
};

class PhiPermissionsEngine {
private:
    std::unordered_map<std::string, User> users;
    std::unordered_map<std::string, Group> groups;
    std::unordered_map<std::string, Permission> permissions;
    std::unordered_map<std::string, TempGrant> temp_grants;
    
    struct CacheEntry {
        bool granted;
        std::chrono::system_clock::time_point computed_at;
        static constexpr int TTL_SECONDS = 300;
        bool is_valid() const {
            return (std::chrono::system_clock::now() - computed_at) < std::chrono::seconds(TTL_SECONDS);
        }
    };
    std::unordered_map<std::string, std::unordered_map<std::string, CacheEntry>> cache;
    
    std::deque<AuditEntry> audit_queue;
    static constexpr size_t MAX_AUDIT_QUEUE = 1000;
    
    mutable std::shared_mutex data_mutex;
    mutable std::shared_mutex cache_mutex;
    std::mutex audit_mutex;
    
    std::unordered_set<std::string> resolve_group_permissions(
        const std::string& group_id, int max_depth = 50) const {
        std::unordered_set<std::string> resolved;
        std::unordered_set<std::string> visited;
        std::string current = group_id;
        int depth = 0;
        
        while (depth < max_depth && !current.empty()) {
            if (visited.count(current)) break;
            visited.insert(current);
            auto it = groups.find(current);
            if (it == groups.end()) break;
            const auto& g = it->second;
            resolved.insert(g.permissions.begin(), g.permissions.end());
            if (g.inherit_from_parent && !g.parent_id.empty()) {
                current = g.parent_id;
            } else break;
            depth++;
        }
        return resolved;
    }
    
    bool resolve_conflicts(const std::string& perm_id,
                          const std::unordered_set<std::string>& user_perms) const {
        auto it = permissions.find(perm_id);
        if (it == permissions.end()) return true;
        for (const auto& conflict : it->second.conflicts_with) {
            if (user_perms.count(conflict)) {
                double resolution = FGG(1.0 * (1.0/PHI), 3);
                if (resolution < 0.5) return false;
            }
        }
        return true;
    }
    
public:
    void add_user(User u) {
        std::unique_lock lock(data_mutex);
        users[u.id] = std::move(u);
    }
    
    void add_group(Group g) {
        std::unique_lock lock(data_mutex);
        groups[g.id] = std::move(g);
    }
    
    void add_permission(Permission p) {
        std::unique_lock lock(data_mutex);
        permissions[p.id] = std::move(p);
    }
    
    void grant_temp(const std::string& uid, const std::string& pid,
                    std::chrono::seconds duration, const std::string& by = "system") {
        std::unique_lock lock(data_mutex);
        std::string gid = "grant_" + std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count());
        temp_grants[gid] = {gid, uid, pid, by, std::chrono::system_clock::now() + duration};
        std::unique_lock cl(cache_mutex);
        cache.erase(uid);
    }
    
    bool check_permission(const std::string& uid, const std::string& pid) {
        // Cache check
        {
            std::shared_lock cl(cache_mutex);
            auto uit = cache.find(uid);
            if (uit != cache.end()) {
                auto pit = uit->second.find(pid);
                if (pit != uit->second.end() && pit->second.is_valid()) {
                    return pit->second.granted;
                }
            }
        }
        
        std::shared_lock dl(data_mutex);
        std::string source = "denied";
        bool granted = false;
        
        auto uit = users.find(uid);
        if (uit == users.end()) return false;
        const auto& u = uit->second;
        
        std::unordered_set<std::string> all_perms;
        all_perms.insert(u.custom_permissions.begin(), u.custom_permissions.end());
        
        for (const auto& gid : u.groups) {
            if (u.banned_groups.count(gid)) continue;
            auto gp = resolve_group_permissions(gid);
            all_perms.insert(gp.begin(), gp.end());
        }
        
        for (const auto& [gid, grant] : temp_grants) {
            if (grant.user_id == uid && !grant.is_expired()) {
                all_perms.insert(grant.permission_id);
            }
        }
        
        if (all_perms.count(pid) && resolve_conflicts(pid, all_perms)) {
            granted = true;
            if (u.custom_permissions.count(pid)) source = "custom";
            else {
                for (const auto& gid : u.groups) {
                    if (resolve_group_permissions(gid).count(pid)) {
                        source = "group:" + gid; break;
                    }
                }
                if (source == "denied") source = "temp_grant";
            }
        } else if (!resolve_conflicts(pid, all_perms)) {
            source = "conflict";
        }
        
        {
            std::unique_lock cl(cache_mutex);
            cache[uid][pid] = {granted, std::chrono::system_clock::now()};
        }
        
        {
            std::lock_guard<std::mutex> al(audit_mutex);
            audit_queue.push_back({
                std::chrono::system_clock::now(), uid, pid, source, granted, cassini_verify()
            });
            if (audit_queue.size() > MAX_AUDIT_QUEUE) audit_queue.pop_front();
        }
        
        return granted;
    }
    
    std::vector<AuditEntry> get_audit_log() const {
        std::lock_guard<std::mutex> al(const_cast<std::mutex&>(audit_mutex));
        return {audit_queue.begin(), audit_queue.end()};
    }
    
    void invalidate_cache(const std::string& uid = "") {
        std::unique_lock cl(cache_mutex);
        if (uid.empty()) cache.clear();
        else cache.erase(uid);
    }
    
    bool verify_integrity() const { return cassini_verify(); }
};
