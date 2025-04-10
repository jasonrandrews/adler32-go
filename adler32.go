// Package main provides benchmarking and testing for Adler32 checksum implementations.
//
// This package demonstrates how to integrate optimized C code with Go applications
// to achieve substantial performance gains over the standard library implementation.
// It compares the performance of the standard Go Adler32 implementation with an
// optimized implementation using Arm NEON SIMD instructions.
package main

import (
	"fmt"
	"hash/adler32"
	"math/rand"
	"strings"
	"time"
	"unsafe"
)

/*
#cgo LDFLAGS: -L. -ladler32_neon
#include "adler32_neon.h"
*/
import "C"

// generateRandomString creates a random string of specified length.
//
// This function is used for testing and benchmarking purposes to generate
// random data of various sizes.
//
// Parameters:
//   - length: The desired length of the random string in bytes
//
// Returns:
//   - A byte slice containing random characters from the defined charset
func generateRandomString(length int) []byte {
	const charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}|;:,.<>?/"
	b := make([]byte, length)
	for i := range b {
		b[i] = charset[rand.Intn(len(charset))]
	}
	return b
}

// calculateNeonAdler32 calculates Adler32 using the C NEON implementation.
//
// This function leverages Arm NEON SIMD instructions for improved performance.
// It handles empty data correctly, returning 1 as per the Adler-32 specification.
//
// Parameters:
//   - data: The byte slice to calculate the checksum for
//
// Returns:
//   - The calculated Adler-32 checksum as a uint32
func calculateNeonAdler32(data []byte) uint32 {
	if len(data) == 0 {
		return 1 // Adler-32 of empty data is 1
	}
	
	// Convert Go byte slice to C pointer and length
	cData := (*C.uint8_t)(unsafe.Pointer(&data[0]))
	cLen := C.size_t(len(data))
	
	// Call the C function
	return uint32(C.adler32_neon(cData, cLen))
}

// benchmarkGoAdler32 runs the Go implementation multiple times and returns average duration.
//
// This function is used for performance comparison with the NEON implementation.
//
// Parameters:
//   - data: The byte slice to calculate the checksum for
//   - iterations: Number of times to run the benchmark
//
// Returns:
//   - The average duration per iteration
func benchmarkGoAdler32(data []byte, iterations int) time.Duration {
	start := time.Now()
	for i := 0; i < iterations; i++ {
		adler32.Checksum(data)
	}
	return time.Since(start) / time.Duration(iterations)
}

// benchmarkNeonAdler32 runs the C NEON implementation multiple times and returns average duration.
//
// This function is used for performance comparison with the Go implementation.
//
// Parameters:
//   - data: The byte slice to calculate the checksum for
//   - iterations: Number of times to run the benchmark
//
// Returns:
//   - The average duration per iteration
func benchmarkNeonAdler32(data []byte, iterations int) time.Duration {
	start := time.Now()
	for i := 0; i < iterations; i++ {
		calculateNeonAdler32(data)
	}
	return time.Since(start) / time.Duration(iterations)
}

// main runs a series of tests and benchmarks comparing the standard Go
// implementation of Adler-32 with the optimized NEON implementation.
//
// It performs:
// 1. A simple example test to verify correctness
// 2. 1000 tests with random data to ensure implementations match
// 3. Performance benchmarks with different data sizes
func main() {
	// Seed the random number generator
	rand.Seed(time.Now().UnixNano())
	
	// Simple example
	data := []byte("example data")
	goChecksum := adler32.Checksum(data)
	neonChecksum := calculateNeonAdler32(data)
	
	fmt.Printf("Example test:\n")
	fmt.Printf("Go Adler-32 Checksum: %d\n", goChecksum)
	fmt.Printf("NEON Adler-32 Checksum: %d\n", neonChecksum)
	fmt.Printf("Match: %v\n\n", goChecksum == neonChecksum)
	
	// Run 1000 tests with random data
	fmt.Println("Running 1000 tests with random data...")
	
	failures := 0
	for i := 0; i < 1000; i++ {
		// Generate random string length between 1 and 2000
		length := rand.Intn(2000) + 1
		testData := generateRandomString(length)
		
		// Calculate checksums using both implementations
		goResult := adler32.Checksum(testData)
		neonResult := calculateNeonAdler32(testData)
		
		// Compare results
		if goResult != neonResult {
			failures++
			fmt.Printf("Test %d FAILED: length=%d, Go=%d, NEON=%d\n", 
				i+1, length, goResult, neonResult)
		}
	}
	
	// Print summary
	if failures == 0 {
		fmt.Println("All 1000 tests PASSED! Both implementations produce identical results.")
	} else {
		fmt.Printf("Tests FAILED: %d out of 1000 tests produced different results.\n", failures)
	}
	
	// Performance benchmarking
	fmt.Println("\n=== Performance Benchmarking ===")
	
	// Test with different data sizes
	dataSizes := []int{1024, 10*1024, 100*1024, 1024*1024, 10*1024*1024}
	iterations := []int{10000, 1000, 100, 10, 5}
	
	fmt.Printf("%-15s %-15s %-15s %-15s\n", "Data Size", "Go Time", "NEON(C) Time", "Speedup")
	fmt.Println(strings.Repeat("-", 60))
	
	for i, size := range dataSizes {
		// Generate random data of specified size
		benchData := generateRandomString(size)
		
		// Run benchmarks
		iter := iterations[i]
		goTime := benchmarkGoAdler32(benchData, iter)
		neonTime := benchmarkNeonAdler32(benchData, iter)
		
		// Calculate speedup
		speedup := float64(goTime) / float64(neonTime)
		
		// Format size for display
		sizeStr := fmt.Sprintf("%d KB", size/1024)
		if size >= 1024*1024 {
			sizeStr = fmt.Sprintf("%d MB", size/(1024*1024))
		}
		
		// Print results
		fmt.Printf("%-15s %-15s %-15s %-15.2fx\n", 
			sizeStr, 
			goTime.String(), 
			neonTime.String(), 
			speedup)
	}
}
