#include "adler32_neon.h"

#define MOD_ADLER 65521

uint32_t adler32_neon(const uint8_t *data, size_t len) {
    if (len == 0) return 1;
    
    uint32_t s1 = 1;
    uint32_t s2 = 0;
    size_t i = 0;
    
    // Process 16 bytes at a time using NEON
    if (len >= 16) {
        // Initialize NEON vectors for s1 and s2
        uint32x4_t v_s1 = vdupq_n_u32(0);
        uint32x4_t v_s2 = vdupq_n_u32(0);
        
        // Create multipliers for s1 contribution to s2
        // When processing 16 bytes at once, each byte's s1 contribution to s2 depends on its position
        uint32x4_t v_16 = vdupq_n_u32(16);
        uint32x4_t v_15 = vdupq_n_u32(15);
        uint32x4_t v_14 = vdupq_n_u32(14);
        uint32x4_t v_13 = vdupq_n_u32(13);
        uint32x4_t v_12 = vdupq_n_u32(12);
        uint32x4_t v_11 = vdupq_n_u32(11);
        uint32x4_t v_10 = vdupq_n_u32(10);
        uint32x4_t v_9 = vdupq_n_u32(9);
        uint32x4_t v_8 = vdupq_n_u32(8);
        uint32x4_t v_7 = vdupq_n_u32(7);
        uint32x4_t v_6 = vdupq_n_u32(6);
        uint32x4_t v_5 = vdupq_n_u32(5);
        uint32x4_t v_4 = vdupq_n_u32(4);
        uint32x4_t v_3 = vdupq_n_u32(3);
        uint32x4_t v_2 = vdupq_n_u32(2);
        uint32x4_t v_1 = vdupq_n_u32(1);
        
        // Process blocks of 5552 bytes to avoid overflow
        const size_t BLOCK_SIZE = 5552;
        const size_t NEON_CHUNK_SIZE = 16;
        
        while (len >= BLOCK_SIZE) {
            size_t block_len = BLOCK_SIZE;
            
            // Reset accumulators for this block
            uint32x4_t block_s1 = vdupq_n_u32(0);
            uint32x4_t block_s2 = vdupq_n_u32(0);
            
            // Process 16 bytes at a time within this block
            while (block_len >= NEON_CHUNK_SIZE) {
                // Load 16 bytes
                uint8x16_t v_data = vld1q_u8(data + i);
                
                // Convert to 32-bit integers (first 4 bytes)
                uint32x4_t v_data1 = vmovl_u16(vget_low_u16(vmovl_u8(vget_low_u8(v_data))));
                
                // Convert to 32-bit integers (second 4 bytes)
                uint32x4_t v_data2 = vmovl_u16(vget_high_u16(vmovl_u8(vget_low_u8(v_data))));
                
                // Convert to 32-bit integers (third 4 bytes)
                uint32x4_t v_data3 = vmovl_u16(vget_low_u16(vmovl_u8(vget_high_u8(v_data))));
                
                // Convert to 32-bit integers (fourth 4 bytes)
                uint32x4_t v_data4 = vmovl_u16(vget_high_u16(vmovl_u8(vget_high_u8(v_data))));
                
                // Update s1 for this chunk
                block_s1 = vaddq_u32(block_s1, v_data1);
                block_s1 = vaddq_u32(block_s1, v_data2);
                block_s1 = vaddq_u32(block_s1, v_data3);
                block_s1 = vaddq_u32(block_s1, v_data4);
                
                // Update s2 for this chunk
                // Each byte contributes to s2 based on its position and the current s1
                block_s2 = vaddq_u32(block_s2, vmulq_u32(v_data1, v_16));
                block_s2 = vaddq_u32(block_s2, vmulq_u32(v_data2, v_12));
                block_s2 = vaddq_u32(block_s2, vmulq_u32(v_data3, v_8));
                block_s2 = vaddq_u32(block_s2, vmulq_u32(v_data4, v_4));
                
                i += NEON_CHUNK_SIZE;
                block_len -= NEON_CHUNK_SIZE;
            }
            
            // Horizontal sum of vectors
            uint32_t sum_s1 = vgetq_lane_u32(block_s1, 0) + vgetq_lane_u32(block_s1, 1) + 
                              vgetq_lane_u32(block_s1, 2) + vgetq_lane_u32(block_s1, 3);
            uint32_t sum_s2 = vgetq_lane_u32(block_s2, 0) + vgetq_lane_u32(block_s2, 1) + 
                              vgetq_lane_u32(block_s2, 2) + vgetq_lane_u32(block_s2, 3);
            
            // Update scalar s1 and s2
            s1 = (s1 + sum_s1) % MOD_ADLER;
            // For s2, we need to account for the s1 contribution
            s2 = (s2 + sum_s2 + BLOCK_SIZE * s1 - sum_s1 * (BLOCK_SIZE + 1) / 2) % MOD_ADLER;
            
            len -= BLOCK_SIZE;
        }
    }
    
    // Process remaining bytes with scalar code
    while (i < len) {
        s1 = (s1 + data[i]) % MOD_ADLER;
        s2 = (s2 + s1) % MOD_ADLER;
        i++;
    }
    
    return (s2 << 16) | s1;
}
