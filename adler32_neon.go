// Package main provides a Go wrapper for the optimized Adler-32 implementation.
//
// This file contains the CGO bindings to call the optimized C implementation
// of the Adler-32 checksum algorithm using Arm NEON instructions.
package main

/*
#cgo CFLAGS: -I${SRCDIR}
#cgo LDFLAGS: -L${SRCDIR} -ladler32_neon
#include "adler32_neon.h"
*/
import "C"
import (
    "unsafe"
)

// Adler32NEON computes the Adler-32 checksum using the optimized C implementation.
//
// This function provides a simple interface to the NEON-optimized Adler-32
// implementation. It handles the conversion between Go and C data types.
//
// Parameters:
//   - data: The byte slice to calculate the checksum for
//
// Returns:
//   - The calculated Adler-32 checksum as a uint32
func Adler32NEON(data []byte) uint32 {
    cData := (*C.uchar)(unsafe.Pointer(&data[0]))
    cLen := C.size_t(len(data))
    checksum := C.adler32_neon(cData, cLen)
    return uint32(checksum)
}

// main demonstrates the basic usage of the Adler32NEON function.
//
// This is a simple example showing how to calculate the Adler-32 checksum
// of a string using the optimized implementation.
func main() {
    data := []byte("example data")
    checksum := Adler32NEON(data)
    println("Adler-32 Checksum:", checksum)
}
