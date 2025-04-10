#include "adler32_neon.h"

#define MOD_ADLER 65521

uint32_t adler32_neon(const uint8_t *data, size_t len) {
    uint32_t s1 = 1;
    uint32_t s2 = 0;
    size_t i;

    // For very short inputs, use scalar implementation
    if (len < 16) {
        for (i = 0; i < len; ++i) {
            s1 = (s1 + data[i]) % MOD_ADLER;
            s2 = (s2 + s1) % MOD_ADLER;
        }
        return (s2 << 16) | s1;
    }

    // Process data in blocks to avoid too frequent modulo operations
    // which are expensive
    const size_t BLOCK_SIZE = 5552; // Max size before s1 can overflow
    size_t blocks = len / BLOCK_SIZE;
    size_t remaining = len % BLOCK_SIZE;
    
    for (size_t block = 0; block < blocks; ++block) {
        const uint8_t* block_data = data + (block * BLOCK_SIZE);
        
        // Process this block
        for (i = 0; i < BLOCK_SIZE; ++i) {
            s1 += block_data[i];
            s2 += s1;
        }
        
        // Apply modulo after processing the block
        s1 %= MOD_ADLER;
        s2 %= MOD_ADLER;
    }
    
    // Process remaining bytes
    const uint8_t* remaining_data = data + (blocks * BLOCK_SIZE);
    for (i = 0; i < remaining; ++i) {
        s1 += remaining_data[i];
        s2 += s1;
    }
    
    // Final modulo
    s1 %= MOD_ADLER;
    s2 %= MOD_ADLER;

    return (s2 << 16) | s1;
}
