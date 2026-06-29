#pragma once
#include <cmath>
#include <cstdint>
#include <chrono>
#include <array>
#include <atomic>
#include <map>
#include <mutex>

namespace antimatter {

constexpr double PHI = 1.6180339887498948482;
constexpr double OCC = 0.6180339887498948482;
constexpr double SCHUMANN = 7.83;
constexpr int DIMS = 7;
constexpr double WINDOW_SECONDS = 60.0;

struct ClientRateState {
    double last_request_time = 0;
    double phi_spiral_phase = 0;
    std::array<double, DIMS> chaos_state;
    uint64_t request_count = 0;
    uint64_t blocked_count = 0;
    bool initialized = false;
};

class TripleAntiMatter {
private:
    std::map<std::string, ClientRateState> clients;
    std::mutex m;
    uint64_t total_blocked = 0;
    bool schumann_enabled = false;

    bool check_phi_spiral(ClientRateState& s, double now) {
        if(!s.initialized) return true;
        double delta = now - s.last_request_time;
        double expected = OCC * (1.0 + s.phi_spiral_phase);
        if(delta < expected * 0.2) {
            s.phi_spiral_phase += OCC * OCC;
            return s.phi_spiral_phase <= 3.0;
        }
        s.phi_spiral_phase = s.phi_spiral_phase * OCC + OCC * (1.0 - OCC);
        return true;
    }

    bool check_chaos(ClientRateState& s, double now) {
        if(!s.initialized) {
            for(int d=0;d<DIMS;d++) s.chaos_state[d]=std::fmod(now*(d+1)*PHI,1.0);
            return true;
        }
        std::array<double,DIMS> ns; double cs=0;
        for(int d=0;d<DIMS;d++) {
            double self=PHI*s.chaos_state[d]*(1.0-s.chaos_state[d]);
            double coupling=0;
            for(int j=0;j<DIMS;j++) if(j!=d) coupling+=OCC*(s.chaos_state[j]-s.chaos_state[d])/(1+std::abs(d-j));
            ns[d]=self+OCC*coupling;
            if(std::fabs(ns[d]-s.chaos_state[d])<0.001) cs+=1.0;
        }
        for(int d=0;d<DIMS;d++) s.chaos_state[d]=ns[d];
        return cs<4.0;
    }

    bool check_schumann(double now) {
        if(!schumann_enabled) return true;
        double w=std::sin(2.0*M_PI*SCHUMANN*now)+std::sin(2.0*M_PI*14.3*now)*0.5+
                 std::sin(2.0*M_PI*20.8*now)*0.3+std::sin(2.0*M_PI*27.3*now)*0.2;
        return w>-0.3;
    }

public:
    TripleAntiMatter(bool schumann=false) : schumann_enabled(schumann) {}

    bool allow(const std::string& cid) {
        std::lock_guard<std::mutex> l(m);
        auto& s = clients[cid];
        double now = std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        
        if(s.initialized && (now-s.last_request_time)>WINDOW_SECONDS) s=ClientRateState{};
        
        bool first = !s.initialized;
        bool ok = check_phi_spiral(s,now) && check_chaos(s,now) && check_schumann(now);
        
        s.last_request_time = now;
        s.request_count++;
        s.initialized = true;
        
        if(!ok && !first) { s.blocked_count++; total_blocked++; }
        return ok || first;  // First request always allowed
    }

    struct RateStats { uint64_t requests,blocked; double phi_phase,chaos_entropy; };
    RateStats get_stats(const std::string& cid) {
        std::lock_guard<std::mutex> l(m);
        auto it=clients.find(cid);
        if(it==clients.end()) return {0,0,0,0};
        auto& s=it->second; double e=0;
        for(int d=0;d<DIMS;d++) e+=s.chaos_state[d];
        return {s.request_count,s.blocked_count,s.phi_spiral_phase,e/DIMS};
    }
    uint64_t get_total_blocked() const { return total_blocked; }
};

} // namespace antimatter
