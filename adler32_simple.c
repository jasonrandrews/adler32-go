#include "adler32_neon.h"

#define MOD_ADLER 65521

// This is a simple, correct implementation without NEON optimizations
// It matches the Go implementation's behavior exactly
uint32_t adler32_neon(const uint8_t *data, size_t len) {
    uint32_t s1 = 1;
    uint32_t s2 = 0;
    
    // Process each byte sequentially, exactly as the standard algorithm requires
    for (size_t i = 0; i < len; i++) {
        s1 = (s1 + data[i]) % MOD_ADLER;
        s2 = (s2 + s1) % MOD_ADLER;
    }
    
    return (s2 << 16) | s1;
}
