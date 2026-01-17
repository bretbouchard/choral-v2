/*
 * Choir V2.0 - Universal Vocal Synthesis Platform
 *
 * MemoryPool.h - Lock-free memory pool
 *
 * Copyright (c) 2026 Bret Bouchard
 * All rights reserved.
 */

#pragma once

#include <memory>
#include <vector>

namespace ChoirV2 {

/**
 * @brief Lock-free memory pool
 *
 * Pre-allocates memory blocks to avoid real-time allocations.
 * Uses lock-free techniques for thread safety.
 *
 * Critical for real-time audio processing where allocations
 * in the audio thread are forbidden.
 */
class MemoryPool {
public:
    MemoryPool(size_t block_size, int num_blocks);
    ~MemoryPool();

    /**
     * @brief Allocate a block
     * @return Pointer to block, or nullptr if pool exhausted
     */
    void* allocate();

    /**
     * @brief Free a block
     * @param ptr Block to free
     */
    void deallocate(void* ptr);

    /**
     * @brief Get pool statistics
     */
    struct PoolStats {
        int total_blocks;
        int free_blocks;
        int allocated_blocks;
    };
    PoolStats getStats() const;

private:
    size_t block_size_;
    std::vector<std::unique_ptr<uint8_t[]>> blocks_;
    std::vector<void*> free_list_;

    // Lock-free state
    std::atomic<int> free_count_;
};

} // namespace ChoirV2
