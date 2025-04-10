# Accelerating Adler32 Checksum with Arm NEON Instructions

This project demonstrates how to significantly improve the performance of Adler32 checksum calculations using Arm NEON SIMD (Single Instruction, Multiple Data) instructions. The implementation shows how to integrate optimized C code with Go applications to achieve substantial performance gains over the standard library implementation.

## What is Adler32?

Adler32 is a checksum algorithm that was invented by Mark Adler in 1995. It's used in the zlib compression library and is faster than CRC32 but provides less reliable error detection. The algorithm works by calculating two 16-bit sums:

1. s1: A simple sum of all bytes
2. s2: A sum of all s1 values after each byte

The final checksum is (s2 << 16) | s1.

## Project Overview

This project includes:

1. A standard Go implementation using the `hash/adler32` package
2. A NEON-optimized C implementation that processes multiple bytes in parallel
3. CGO bindings to call the optimized C code from Go
4. Benchmarking code to compare performance between implementations

## Implementation Details

### Standard Go Implementation

The Go standard library provides an Adler32 implementation in the `hash/adler32` package. This serves as our baseline for performance comparison.

### NEON-Optimized Implementation

The NEON implementation leverages Arm's SIMD instructions to process 16 bytes at a time, significantly improving throughput. Key optimizations include:

- Processing data in 16-byte chunks using NEON vector instructions
- Handling blocks of 5552 bytes to avoid overflow (as per the Adler32 algorithm requirements)
- Carefully managing the position-dependent nature of the s2 calculation
- Falling back to scalar code for any remaining bytes

### Integration with Go

The project uses CGO to call the optimized C code from Go:

```go
// calculateNeonAdler32 calculates Adler32 using the C NEON implementation
func calculateNeonAdler32(data []byte) uint32 {
    cData := (*C.uint8_t)(unsafe.Pointer(&data[0]))
    cLen := C.size_t(len(data))
    return uint32(C.adler32_neon(cData, cLen))
}
```

## Performance Results

### Neoverse N1 Performance Results

The following performance results were measured on a system with a Neoverse N1 processor:

#### Implementation Comparison

The project includes three different implementations of Adler32:

1. **Simple**: A basic scalar implementation
2. **Block**: An optimized scalar implementation that processes data in blocks to avoid overflow
3. **NEON**: A SIMD-optimized implementation using Arm NEON instructions

Here are the performance results for each implementation:

| Data Size | Go Time | Simple Time | Block Time | NEON Time | NEON Speedup vs Go |
|-----------|---------|-------------|------------|-----------|-------------------|
| 1 KB      | 715ns   | 5.433µs     | 1.053µs    | 3.389µs   | 0.21×             |
| 10 KB     | 6.862µs | 44.755µs    | 9.209µs    | 2.057µs   | 3.34×             |
| 100 KB    | 68.965µs| 330.981µs   | 89.59µs    | 35.299µs  | 1.95×             |
| 1 MB      | 704.285µs| 3.349ms    | 917.258µs  | 368.478µs | 1.91×             |
| 10 MB     | 7.054ms | 33.495ms    | 9.176ms    | 3.706ms   | 1.90×             |

#### Compiler Comparison for NEON Implementation

The NEON implementation was compiled with both GCC and Clang to compare performance:

| Data Size | Go Time | GCC Time | Clang Time | Clang Speedup vs Go | Clang vs GCC |
|-----------|---------|----------|------------|---------------------|-------------|
| 1 KB      | 718ns   | 6.003µs  | 3.431µs    | 0.21×               | 1.75×       |
| 10 KB     | 10.972µs| 3.049µs  | 1.536µs    | 4.45×               | 1.98×       |
| 100 KB    | 89.965µs| 45.367µs | 25.816µs   | 2.67×               | 1.76×       |
| 1 MB      | 705.664µs| 367.438µs| 276.053µs | 2.55×               | 1.33×       |
| 10 MB     | 7.054ms | 3.695ms  | 2.691ms    | 2.62×               | 1.37×       |

*Note: Actual performance numbers may vary based on your specific ARM hardware. These results were measured on the Neoverse N1 test system.*

### Neoverse V2 Performance Results (April 2025)

We also tested the implementations on a Neoverse V2 processor, which showed different performance characteristics:

#### Simple Implementation (Scalar)
| Data Size | Go Time | Simple Time | Speedup vs Go |
|-----------|---------|-------------|--------------|
| 1 KB      | 392ns   | 2.628µs     | 0.15×        |
| 10 KB     | 3.935µs | 25.772µs    | 0.15×        |
| 100 KB    | 39.679µs| 257.517µs   | 0.15×        |
| 1 MB      | 403.678µs| 2.630887ms | 0.15×        |
| 10 MB     | 4.025969ms| 26.309335ms| 0.15×        |

#### Block Implementation (Optimized Scalar)
| Data Size | Go Time | Block Time | Speedup vs Go |
|-----------|---------|------------|--------------|
| 1 KB      | 393ns   | 645ns      | 0.61×        |
| 10 KB     | 3.927µs | 5.867µs    | 0.67×        |
| 100 KB    | 39.25µs | 58.176µs   | 0.67×        |
| 1 MB      | 401.945µs| 594.972µs | 0.68×        |
| 10 MB     | 4.028003ms| 5.957824ms| 0.68×        |

#### NEON Implementation (Clang vs GCC)
| Data Size | Go Time | Clang Time | GCC Time | GCC Speedup vs Go |
|-----------|---------|------------|----------|-------------------|
| 1 KB      | 392ns   | 2.631µs    | 2.627µs  | 0.15×             |
| 10 KB     | 3.944µs | 1.19µs     | 908ns    | 4.38×             |
| 100 KB    | 39.234µs| 19.8µs     | 15.267µs | 2.60×             |
| 1 MB      | 402.348µs| 206.185µs | 159.349µs| 2.52×             |
| 10 MB     | 4.024462ms| 2.079317ms| 1.578822ms| 2.55×           |

Key observations:
- For small data sizes (1 KB), all C implementations are slower than Go due to function call overhead
- The NEON implementation is significantly faster than both scalar implementations for data sizes ≥ 10 KB
- On the original test system, Clang consistently produces faster code than GCC for the NEON implementation, with up to 98% better performance
- On the Neoverse V2 system, GCC produces faster code than Clang for the NEON implementation, with approximately 30% better performance
- The Block implementation is slower than Go on the Neoverse V2 system, unlike the results on the original test system
- The optimized implementations show the greatest advantage at the 10 KB data size

## Building and Running

### Prerequisites

- Go 1.15 or later
- GCC or Clang compiler (Clang is used by default as it produces faster code)
- Arm processor with NEON support

### Build Instructions

1. Clone the repository
2. Build the shared library:
   ```
   make compile
   ```
3. Run the benchmark:
   ```
   make run
   ```

### Makefile Targets

The project includes a flexible Makefile that allows you to easily switch between different implementations:

- `make compile` - Builds the library using the default implementation (adler32_neon.c)
- `make run` - Runs the benchmark using the compiled library
- `make all` - Combines compile and run targets

You can specify which implementation to use:

- `make simple` - Builds using the simple scalar implementation (adler32_simple.c)
- `make block` - Builds using the block-based implementation (adler32_block.c)
- `make neon` - Builds using the NEON-optimized implementation (adler32_neon.c)

You can also specify the implementation directly:
```
make IMPL=adler32_simple.c compile
```

To build and run with a specific implementation in one command:
```
make IMPL=adler32_block.c all
```
or
```
make simple run
```

## How It Works

The NEON implementation works by:

1. Processing 16 bytes at a time using NEON vector registers
2. Calculating s1 and s2 in parallel for these 16 bytes
3. Handling the position-dependent nature of s2 calculation with vector multiplications
4. Processing data in blocks of 5552 bytes to prevent overflow
5. Combining the results and applying the modulo operation

## Conclusion

This project demonstrates how leveraging Arm NEON instructions can significantly improve the performance of checksum calculations. The techniques shown here can be applied to other algorithms that process data sequentially and can benefit from SIMD parallelism.

By using hardware-specific optimizations while maintaining a clean Go API, we can achieve the best of both worlds: the development experience of Go with the performance of optimized C code.

## License

This project is released into the public domain under the [Unlicense](LICENSE).

This means you can copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means, without any attribution or restrictions.
