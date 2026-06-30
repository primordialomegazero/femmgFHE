#pragma once
#include <string>

// COMPLETELY DISABLED FOR LOCALHOST
class TripleAntiMatter {
private:
    bool enabled;
public:
    TripleAntiMatter(bool e=true) : enabled(e) {}
    bool allow(const std::string& /*ip*/) { 
        // Always allow - bypass for localhost
        return true; 
    }
    void set_enabled(bool e) { enabled = e; }
};
