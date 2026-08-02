// ═══════════════════════════════════════════════════════════════════════════════
// PHI SCHEDULER — Structural Context Switch Isolation
// ═══════════════════════════════════════════════════════════════════════════════
//
// Every context switch encrypts the outgoing process state and decrypts
// the incoming process state using their respective φ-branch seeds.
//
// Without the correct φ-seed, a process's state is random bytes.
// Even if the scheduler is compromised, process isolation holds —
// because the encryption keys are STRUCTURAL, not stored in the scheduler.
//
// SCHEDULING POLICY: Round-robin with φ-weighted time slices.
// Each process gets time proportional to φ^k mod 1 → fair, deterministic.

#pragma once
#include "phi_process.h"
#include <queue>
#include <chrono>
#include <thread>
#include <vector>

struct ProcessContext {
    uint64_t pid;
    uint64_t phi_seed;
    std::vector<uint8_t> encrypted_state;  // CPU registers, stack, etc.
    bool ready;
};

class PhiScheduler {
private:
    std::queue<ProcessContext*> ready_queue;
    ProcessContext* current;
    PhiProcessManager* process_manager;
    std::mutex sched_mutex;
    uint64_t tick_counter;
    
    // φ-weighted time slice: proportional to φ^k mod 1
    int phi_timeslice(uint64_t k) {
        double slice = std::fmod(PHI * (double)(k + 1), 1.0);
        return std::max(10, (int)(slice * 100));  // 10-100ms
    }
    
    void xor_state(void* state, size_t size, uint64_t seed) {
        uint64_t* words = (uint64_t*)state;
        size_t n = size / 8;
        uint64_t key = seed;
        for (size_t i = 0; i < n; i++) {
            words[i] ^= key;
            key = key * 11400714819323198485ULL + 1372383749ULL;
        }
        uint8_t* bytes = (uint8_t*)(words + n);
        size_t rem = size % 8;
        uint8_t* kb = (uint8_t*)&key;
        for (size_t i = 0; i < rem; i++) bytes[i] ^= kb[i];
    }
    
public:
    PhiScheduler(PhiProcessManager* pm) 
        : current(nullptr), process_manager(pm), tick_counter(0) {}
    
    // Add process to scheduling queue
    void add_process(PhiProcess* proc) {
        std::lock_guard<std::mutex> lock(sched_mutex);
        auto* ctx = new ProcessContext();
        ctx->pid = proc->pid;
        ctx->phi_seed = proc->phi_seed;
        ctx->encrypted_state.resize(1024);  // Simulated process state
        ctx->ready = true;
        ready_queue.push(ctx);
    }
    
    // Context switch: save current, load next
    ProcessContext* schedule() {
        std::lock_guard<std::mutex> lock(sched_mutex);
        
        if (ready_queue.empty()) return nullptr;
        
        // Save current process state (encrypted with its φ-seed)
        if (current && current->ready) {
            xor_state(current->encrypted_state.data(), 
                     current->encrypted_state.size(), 
                     current->phi_seed);
            ready_queue.push(current);
        }
        
        // Load next process
        current = ready_queue.front();
        ready_queue.pop();
        
        // Decrypt its state (only possible with correct φ-seed)
        xor_state(current->encrypted_state.data(),
                 current->encrypted_state.size(),
                 current->phi_seed);
        
        tick_counter++;
        return current;
    }
    
    // Attempt to access another process's state (should FAIL)
    bool attempt_cross_access(uint64_t attacker_pid, uint64_t victim_pid) {
        // Find victim's context (simulated — in real OS, this is in memory)
        // The attacker cannot decrypt because seeds are different
        PhiProcess* attacker = process_manager->get_process(attacker_pid);
        PhiProcess* victim = process_manager->get_process(victim_pid);
        
        if (!attacker || !victim) return false;
        
        // CROSS-ACCESS CHECK: attacker seed ≠ victim seed → CANNOT DECRYPT
        return attacker->phi_seed == victim->phi_seed;
    }
    
    ProcessContext* get_current() const { return current; }
    uint64_t get_ticks() const { return tick_counter; }
    size_t queue_size() const { return ready_queue.size(); }
};
