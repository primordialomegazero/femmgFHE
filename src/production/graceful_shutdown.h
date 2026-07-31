#pragma once
#include "../utils/logger.h"
#include <csignal>
#include <atomic>
#include <functional>
#include <vector>

struct GracefulShutdown {
    static std::atomic<bool> shutdown_requested;
    static std::atomic<bool> drain_complete;
    static std::vector<std::function<void()>> cleanup_handlers;
    static std::vector<std::function<void()>> drain_handlers;
    
    static void init() {
        shutdown_requested = false;
        drain_complete = false;
        
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGHUP, signal_handler);
        
        Logger::info("Graceful shutdown handlers registered (SIGINT, SIGTERM, SIGHUP)");
    }
    
    static void signal_handler(int signal) {
        Logger::warn("Received signal " + std::to_string(signal) + " — initiating graceful shutdown");
        shutdown_requested = true;
        
        // Run drain handlers (flush queues, save state)
        for (auto& handler : drain_handlers) {
            handler();
        }
        drain_complete = true;
        
        // Run cleanup handlers
        for (auto& handler : cleanup_handlers) {
            handler();
        }
        
        Logger::info("Graceful shutdown complete");
        exit(0);
    }
    
    static void on_drain(std::function<void()> handler) {
        drain_handlers.push_back(handler);
    }
    
    static void on_cleanup(std::function<void()> handler) {
        cleanup_handlers.push_back(handler);
    }
    
    static bool is_shutting_down() { return shutdown_requested; }
};

std::atomic<bool> GracefulShutdown::shutdown_requested(false);
std::atomic<bool> GracefulShutdown::drain_complete(false);
std::vector<std::function<void()>> GracefulShutdown::cleanup_handlers;
std::vector<std::function<void()>> GracefulShutdown::drain_handlers;
