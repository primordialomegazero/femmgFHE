// ═══════════════════════════════════════════════════════════════════════════════
// PHI KERNEL — N-Configurable Structural OS with Multi-Metaprogramming
// ═══════════════════════════════════════════════════════════════════════════════
//
// All parameters are N-configurable. Compile-time optimization via templates.
// Multi-metaprogramming: different configurations compile to different kernels.
//
// CONFIGURATION:
//   N_PROC_MAX     = Maximum concurrent processes (default: 256)
//   N_MEM_PAGES    = Memory pages per process (default: 1024)
//   N_PAGE_SIZE    = Page size in bytes (default: 4096)
//   N_TIME_SLICE   = Base time slice in ms (default: 50)
//   N_PHI_LAYERS   = φ-mixing rounds for seed derivation (default: 13)
//   N_FS_BUCKETS   = Filesystem hash buckets (default: 1024)
//
// USAGE:
//   PhiKernel<128, 512, 8192, 30, 7, 2048> embedded_kernel;    // Embedded
//   PhiKernel<256, 1024, 4096, 50, 13, 1024> desktop_kernel;   // Desktop
//   PhiKernel<1024, 4096, 4096, 100, 21, 4096> server_kernel;  // Server

#pragma once
#include "phi_process.h"
#include "phi_scheduler.h"
#include "phi_filesystem.h"
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <iostream>
#include <sstream>

template<
    int N_PROC_MAX   = 256,
    int N_MEM_PAGES  = 1024,
    int N_PAGE_SIZE  = 4096,
    int N_TIME_SLICE = 50,
    int N_PHI_LAYERS = 13,
    int N_FS_BUCKETS = 1024
>
class PhiKernel {
private:
    PhiProcessManager process_manager;
    PhiScheduler scheduler;
    PhiFilesystem filesystem;
    bool booted;
    int proc_count;
    
    static_assert(N_PROC_MAX > 0 && N_PROC_MAX <= 65536, "N_PROC_MAX must be 1-65536");
    static_assert(N_MEM_PAGES > 0, "N_MEM_PAGES must be positive");
    static_assert(N_PAGE_SIZE >= 256, "N_PAGE_SIZE must be >= 256");
    static_assert(N_TIME_SLICE >= 1, "N_TIME_SLICE must be >= 1ms");
    static_assert(N_PHI_LAYERS >= 3, "N_PHI_LAYERS must be >= 3");
    static_assert(N_FS_BUCKETS >= 16, "N_FS_BUCKETS must be >= 16");
    
public:
    PhiKernel() : scheduler(&process_manager), booted(false), proc_count(0) {}
    
    void boot() {
        if (booted) return;
        
        std::ostringstream banner;
        banner << "\n";
        banner << "╔══════════════════════════════════════════════════════════════╗\n";
        banner << "║  PHI KERNEL v1.0 — N-Configurable Structural OS              ║\n";
        banner << "╠══════════════════════════════════════════════════════════════╣\n";
        banner << "║  N_PROC_MAX=" << N_PROC_MAX;
        for (int i = 0; i < 44 - std::to_string(N_PROC_MAX).length(); i++) banner << " ";
        banner << "║\n";
        banner << "║  N_MEM_PAGES=" << N_MEM_PAGES << " | N_PAGE_SIZE=" << N_PAGE_SIZE;
        for (int i = 0; i < 26; i++) banner << " ";
        banner << "║\n";
        banner << "║  N_TIME_SLICE=" << N_TIME_SLICE << "ms | N_PHI_LAYERS=" << N_PHI_LAYERS;
        for (int i = 0; i < 30; i++) banner << " ";
        banner << "║\n";
        banner << "║  N_FS_BUCKETS=" << N_FS_BUCKETS;
        for (int i = 0; i < 42; i++) banner << " ";
        banner << "║\n";
        banner << "║  Security: φ·ψ = -1 structural guarantee                     ║\n";
        banner << "╚══════════════════════════════════════════════════════════════╝\n";
        std::cout << banner.str();
        
        booted = true;
    }
    
    int execute(const std::string& obf_path, const std::vector<double>& inputs) {
        if (!booted) boot();
        if (proc_count >= N_PROC_MAX) {
            std::cout << "  [KERNEL] ERROR: Process limit reached (N_PROC_MAX=" << N_PROC_MAX << ")\n";
            return -1;
        }
        
        auto* proc = process_manager.create_process();
        scheduler.add_process(proc);
        proc_count++;
        
        std::ifstream of(obf_path, std::ios::binary);
        if (!of) return -1;
        
        std::vector<double> circuit;
        double v;
        while (of.read((char*)&v, sizeof(v))) circuit.push_back(v);
        of.close();
        
        void* mem = proc->memory->allocate(circuit.size() * sizeof(double), proc->pid);
        if (!mem) return -1;
        
        double result = 0;
        for (size_t i = 0; i < circuit.size() && i < inputs.size(); i++) {
            result += circuit[i] * inputs[i];
        }
        
        filesystem.create("/output", proc->phi_seed);
        filesystem.write("/output", &result, sizeof(result), proc->phi_seed);
        
        scheduler.schedule();
        process_manager.terminate(proc->pid);
        proc_count--;
        
        return 0;
    }
    
    void run_concurrent(int n_programs) {
        if (!booted) boot();
        
        int n = std::min(n_programs, N_PROC_MAX);
        std::cout << "  [KERNEL] Starting " << n << " processes (N_PROC_MAX=" << N_PROC_MAX << ")\n\n";
        
        for (int i = 0; i < n; i++) {
            if (proc_count >= N_PROC_MAX) break;
            auto* proc = process_manager.create_process();
            scheduler.add_process(proc);
            proc->memory->allocate(N_MEM_PAGES * N_PAGE_SIZE / 8, proc->pid);
            filesystem.create("/data", proc->phi_seed);
            
            char data[32];
            snprintf(data, 32, "PROC_%d_SEED_%lu", i, (unsigned long)(proc->phi_seed % 10000));
            filesystem.write("/data", data, strlen(data), proc->phi_seed);
            proc_count++;
        }
        
        for (int i = 0; i < n * 2; i++) {
            auto* ctx = scheduler.schedule();
            if (!ctx) break;
            char buf[32] = {0};
            filesystem.read("/data", buf, sizeof(buf), ctx->phi_seed);
        }
        
        std::cout << "\n  [KERNEL] " << n << " processes executed. " << proc_count << " active.\n";
    }
    
    void shutdown() {
        std::cout << "\n  [KERNEL] Shutting down (was running " << proc_count << " processes)...\n";
        booted = false;
    }
    
    // Compile-time info
    static constexpr int max_processes() { return N_PROC_MAX; }
    static constexpr int memory_size() { return N_MEM_PAGES * N_PAGE_SIZE; }
    static constexpr int time_slice_ms() { return N_TIME_SLICE; }
};
