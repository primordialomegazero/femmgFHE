#pragma once
#include "../database/harmonized_obfuscation.h"
#include <string>
#include <sstream>

struct RESTAPI {
    HarmonizedObfuscation harm;
    
    bool init(const std::string& passphrase = "") { return harm.init("api.db", passphrase); }
    
    std::string handle(const std::string& method, const std::string& path, const std::string& body = "") {
        if (method == "PUT" && path.find("/pg/") == 0) {
            // PUT /pg/table/key → body is value
            auto parts = split(path, '/');
            if (parts.size() >= 4) {
                harm.pg_put(parts[2], parts[3], body);
                return "{\"status\":\"ok\"}";
            }
        }
        if (method == "GET" && path.find("/pg/") == 0) {
            auto parts = split(path, '/');
            if (parts.size() >= 4) return harm.pg_get(parts[2], parts[3]);
        }
        if (method == "GET" && path == "/status") return harm.status();
        return "{\"error\":\"not_found\"}";
    }
    
    static std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> r;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) if (!item.empty()) r.push_back(item);
        return r;
    }
};
