#include <iomanip>
#pragma once
#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <sstream>
#include <csignal>
#include <atomic>

// ============================================
// GOLDEN LOGGER - Full blown logging system
// ============================================

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

class GoldenLogger {
private:
    static GoldenLogger* instance;
    static std::mutex mutex;
    static std::atomic<bool> shutdown_flag;
    static LogLevel current_level;
    static std::ofstream log_file;
    static bool file_logging_enabled;
    static int log_count;
    
    GoldenLogger() = default;
    
    static std::string level_to_string(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::FATAL: return "FATAL";
        }
        return "UNKNOWN";
    }
    
    static std::string current_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
public:
    static GoldenLogger& get_instance() {
        static GoldenLogger instance;
        return instance;
    }
    
    static void set_level(LogLevel level) {
        current_level = level;
    }
    
    static void enable_file_logging(const std::string& filename) {
        log_file.open(filename, std::ios::app);
        file_logging_enabled = true;
    }
    
    static void log(LogLevel level, const std::string& message, 
                    const char* file = "", int line = 0) {
        if (level < current_level) return;
        if (shutdown_flag.load()) return;
        
        std::lock_guard<std::mutex> lock(mutex);
        log_count++;
        
        std::stringstream ss;
        ss << "[" << current_timestamp() << "] "
           << "[" << level_to_string(level) << "] ";
        
        if (file && file[0]) {
            ss << "[" << file << ":" << line << "] ";
        }
        
        ss << message;
        
        std::cout << ss.str() << std::endl;
        
        if (file_logging_enabled && log_file.is_open()) {
            log_file << ss.str() << std::endl;
        }
        
        if (level == LogLevel::FATAL) {
            std::cerr << "FATAL ERROR - initiating graceful shutdown" << std::endl;
            initiate_shutdown();
        }
    }
    
    // Convenience methods
    static void trace(const std::string& msg, const char* file = "", int line = 0) {
        log(LogLevel::TRACE, msg, file, line);
    }
    static void debug(const std::string& msg, const char* file = "", int line = 0) {
        log(LogLevel::DEBUG, msg, file, line);
    }
    static void info(const std::string& msg, const char* file = "", int line = 0) {
        log(LogLevel::INFO, msg, file, line);
    }
    static void warn(const std::string& msg, const char* file = "", int line = 0) {
        log(LogLevel::WARN, msg, file, line);
    }
    static void error(const std::string& msg, const char* file = "", int line = 0) {
        log(LogLevel::ERROR, msg, file, line);
    }
    static void fatal(const std::string& msg, const char* file = "", int line = 0) {
        log(LogLevel::FATAL, msg, file, line);
    }
    
    // Graceful shutdown
    static void initiate_shutdown() {
        shutdown_flag.store(true);
    }
    
    static bool is_shutting_down() {
        return shutdown_flag.load();
    }
    
    static void signal_handler(int sig) {
        switch (sig) {
            case SIGINT:
                warn("SIGINT received - initiating graceful shutdown...");
                initiate_shutdown();
                break;
            case SIGTERM:
                warn("SIGTERM received - initiating graceful shutdown...");
                initiate_shutdown();
                break;
            case SIGSEGV:
                fatal("SIGSEGV received - segmentation fault!");
                break;
        }
    }
    
    static void install_signal_handlers() {
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
    }
    
    static int get_log_count() {
        return log_count;
    }
    
    static void close() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }
};

// Static member initialization
GoldenLogger* GoldenLogger::instance = nullptr;
std::mutex GoldenLogger::mutex;
std::atomic<bool> GoldenLogger::shutdown_flag(false);
LogLevel GoldenLogger::current_level = LogLevel::INFO;
std::ofstream GoldenLogger::log_file;
bool GoldenLogger::file_logging_enabled = false;
int GoldenLogger::log_count = 0;

// Macros for easy logging
#define LOG_TRACE(msg) GoldenLogger::trace(msg, __FILE__, __LINE__)
#define LOG_DEBUG(msg) GoldenLogger::debug(msg, __FILE__, __LINE__)
#define LOG_INFO(msg)  GoldenLogger::info(msg, __FILE__, __LINE__)
#define LOG_WARN(msg)  GoldenLogger::warn(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) GoldenLogger::error(msg, __FILE__, __LINE__)
#define LOG_FATAL(msg) GoldenLogger::fatal(msg, __FILE__, __LINE__)
