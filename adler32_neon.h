#ifndef ADLER32_NEON_H
#define ADLER32_NEON_H

#include <stdint.h>
#include <stddef.h>
#include <arm_neon.h>

uint32_t adler32_neon(const uint8_t *data, size_t len);

#endif // ADLER32_NEON_H
