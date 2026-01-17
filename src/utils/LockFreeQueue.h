/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * LockFreeQueue.h - Lock-free SPSC queue
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <atomic>
#include <vector>

namespace ChoirV2 {

/**
 * @brief Lock-free single-producer-single-consumer queue
 *
 * Used for communication between audio thread and UI thread.
 * Implements lock-free ring buffer for real-time safety.
 */
template<typename T, int Size>
class LockFreeQueue {
public:
    LockFreeQueue() : write_index_(0), read_index_(0) {
        buffer_.resize(Size);
    }

    /**
     * @brief Push element (producer only)
     * @param item Element to push
     * @return true if successful (queue not full)
     */
    bool push(const T& item) {
        int current_write = write_index_.load(std::memory_order_relaxed);
        int next_write = (current_write + 1) % Size;

        if (next_write == read_index_.load(std::memory_order_acquire)) {
            return false;  // Queue full
        }

        buffer_[current_write] = item;
        write_index_.store(next_write, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pop element (consumer only)
     * @param item Output element
     * @return true if successful (queue not empty)
     */
    bool pop(T& item) {
        int current_read = read_index_.load(std::memory_order_relaxed);

        if (current_read == write_index_.load(std::memory_order_acquire)) {
            return false;  // Queue empty
        }

        item = buffer_[current_read];
        read_index_.store((current_read + 1) % Size, std::memory_order_release);
        return true;
    }

    /**
     * @brief Check if queue is empty
     */
    bool isEmpty() const {
        return read_index_.load(std::memory_order_acquire) ==
               write_index_.load(std::memory_order_acquire);
    }

private:
    std::vector<T> buffer_;
    std::atomic<int> write_index_;
    std::atomic<int> read_index_;
};

} // namespace ChoirV2
