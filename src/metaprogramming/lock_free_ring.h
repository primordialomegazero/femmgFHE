#pragma once
#include <atomic>
#include <array>

template<typename T, size_t Capacity>
struct LockFreeRingBuffer {
    static_assert(Capacity > 0 && (Capacity & (Capacity - 1)) == 0, 
                  "Capacity must be power of 2");
    
    std::array<T, Capacity> buffer;
    std::atomic<size_t> write_pos{0};
    std::atomic<size_t> read_pos{0};
    
    // Single-producer push (lock-free)
    bool try_push(const T& item) {
        size_t w = write_pos.load(std::memory_order_relaxed);
        size_t next = (w + 1) & (Capacity - 1);
        
        if (next == read_pos.load(std::memory_order_acquire)) {
            return false;  // Full
        }
        
        buffer[w] = item;
        write_pos.store(next, std::memory_order_release);
        return true;
    }
    
    // Single-consumer pop (lock-free)
    bool try_pop(T& item) {
        size_t r = read_pos.load(std::memory_order_relaxed);
        
        if (r == write_pos.load(std::memory_order_acquire)) {
            return false;  // Empty
        }
        
        item = buffer[r];
        read_pos.store((r + 1) & (Capacity - 1), std::memory_order_release);
        return true;
    }
    
    size_t size() const {
        size_t w = write_pos.load(std::memory_order_acquire);
        size_t r = read_pos.load(std::memory_order_acquire);
        return (w >= r) ? (w - r) : (Capacity - r + w);
    }
    
    bool empty() const { return size() == 0; }
    bool full() const { return size() == Capacity; }
};

// ═══════════════════════════════════════════════════════════════
// LOCK-FREE CHECKPOINT QUEUE (Async writes to FractalDB)
// ═══════════════════════════════════════════════════════════════

struct CheckpointEntry {
    int gates_completed;
    int refreshes_done;
    double phi, psi;
    char phase;  // 'T'=truth, 'S'=stats, 'C'=chain
};

template<size_t QueueSize = 16>
struct LockFreeCheckpointQueue {
    LockFreeRingBuffer<CheckpointEntry, QueueSize> queue;
    
    bool enqueue(const CheckpointEntry& entry) {
        return queue.try_push(entry);
    }
    
    bool dequeue(CheckpointEntry& entry) {
        return queue.try_pop(entry);
    }
};
