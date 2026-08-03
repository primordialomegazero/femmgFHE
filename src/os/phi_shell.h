// ═══════════════════════════════════════════════════════════════════════════════
// PHI SHELL — Structural OS Command Interface
// ═══════════════════════════════════════════════════════════════════════════════

#pragma once
#include "phi_kernel.h"
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#include <map>

template<typename KernelType>
class PhiShell {
private:
    KernelType* kernel;
    bool running;
    std::map<std::string, std::function<void(std::vector<std::string>)>> commands;
    
    void cmd_help(std::vector<std::string> args) {
        std::cout << "\n  Available commands:\n";
        std::cout << "  ┌────────────────────┬──────────────────────────────────┐\n";
        std::cout << "  │ help               │ Show this help                   │\n";
        std::cout << "  │ ps                 │ List running processes            │\n";
        std::cout << "  │ run <file> [args]  │ Execute obfuscated program        │\n";
        std::cout << "  │ spawn <n>          │ Spawn n concurrent processes      │\n";
        std::cout << "  │ mem                │ Show memory stats                 │\n";
        std::cout << "  │ phi                │ Show φ-configuration              │\n";
        std::cout << "  │ clear              │ Clear screen                      │\n";
        std::cout << "  │ exit               │ Shutdown and exit                 │\n";
        std::cout << "  └────────────────────┴──────────────────────────────────┘\n\n";
    }
    
    void cmd_ps(std::vector<std::string> args) {
        std::cout << "\n  Active processes: " << kernel->process_count() << "\n";
        std::cout << "  Max: " << KernelType::max_processes() << "\n\n";
    }
    
    void cmd_run(std::vector<std::string> args) {
        if (args.size() < 1) {
            std::cout << "  Usage: run <file.obf> [arg1] [arg2] ...\n";
            return;
        }
        std::vector<double> inputs;
        for (size_t i = 1; i < args.size(); i++) {
            inputs.push_back(std::stod(args[i]));
        }
        if (inputs.empty()) inputs = {1.0};
        kernel->execute(args[0], inputs);
    }
    
    void cmd_spawn(std::vector<std::string> args) {
        int n = args.size() > 0 ? std::stoi(args[0]) : 3;
        kernel->run_concurrent(n);
    }
    
    void cmd_mem(std::vector<std::string> args) {
        std::cout << "\n  Memory per process: " << KernelType::memory_size() / 1024 << " KB\n";
        std::cout << "  Total addressable: " << (KernelType::memory_size() * KernelType::max_processes()) / (1024*1024) << " MB\n";
        std::cout << "  Time slice: " << KernelType::time_slice_ms() << " ms\n\n";
    }
    
    void cmd_phi(std::vector<std::string> args) {
        std::cout << "\n  φ = 1.6180339887498948482\n";
        std::cout << "  ψ = -0.6180339887498948482\n";
        std::cout << "  φ·ψ = -1 (structural security guarantee)\n";
        std::cout << "  φ + ψ = 1 (DualGate projection identity)\n\n";
    }
    
    void cmd_clear(std::vector<std::string> args) {
        std::cout << "\033[2J\033[H";
    }
    
    void cmd_exit(std::vector<std::string> args) {
        running = false;
    }
    
public:
    PhiShell(KernelType* k) : kernel(k), running(false) {
        commands["help"] = [this](auto a) { cmd_help(a); };
        commands["ps"] = [this](auto a) { cmd_ps(a); };
        commands["run"] = [this](auto a) { cmd_run(a); };
        commands["spawn"] = [this](auto a) { cmd_spawn(a); };
        commands["mem"] = [this](auto a) { cmd_mem(a); };
        commands["phi"] = [this](auto a) { cmd_phi(a); };
        commands["clear"] = [this](auto a) { cmd_clear(a); };
        commands["exit"] = [this](auto a) { cmd_exit(a); };
    }
    
    void start() {
        running = true;
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║  PHI SHELL — Structural OS Command Interface                 ║\n";
        std::cout << "║  Type 'help' for available commands                          ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
        
        std::string input;
        while (running) {
            std::cout << "φ> ";
            std::getline(std::cin, input);
            
            if (input.empty()) continue;
            
            std::istringstream iss(input);
            std::string cmd;
            iss >> cmd;
            
            std::vector<std::string> args;
            std::string arg;
            while (iss >> arg) args.push_back(arg);
            
            auto it = commands.find(cmd);
            if (it != commands.end()) {
                it->second(args);
            } else {
                std::cout << "  Unknown command: " << cmd << " (type 'help')\n";
            }
        }
    }
};
