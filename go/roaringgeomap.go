package geomap

/*
#cgo CFLAGS: -g -Wall
#cgo darwin,arm64 LDFLAGS: -lstdc++ ${SRCDIR}/native/Darwin/arm64/roaringgeomaps.a -ldl -lm

#include <stdlib.h>
#include "../c/include/roaring_geo_map_writer.h"
#include "../c/include/roaring_geo_map_reader.h"
*/
import "C"
import (
	"errors"
	"unsafe"
)

type RoaringGeoMapWriter struct {
	writer *C.RoaringGeoMapWriter
}

// NewRoaringGeoMapWriter creates a new instance of RoaringGeoMapWriter.
func NewRoaringGeoMapWriter(levelIndexBucketRange int) (*RoaringGeoMapWriter, error) {
	w := C.RoaringGeoMapWriter_New(C.int(levelIndexBucketRange))
	if w == nil {
		return nil, errors.New("failed to create RoaringGeoMapWriter")
	}
	return &RoaringGeoMapWriter{writer: w}, nil
}

// Write adds an S2 region and key to the map.
func (r *RoaringGeoMapWriter) Write(cellUnion []uint64, key string) error {
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	// Convert Go slice to C array
	cCellUnion := (*C.uint64_t)(unsafe.Pointer(&cellUnion[0]))
	cSize := C.ulonglong(len(cellUnion))

	if C.RoaringGeoMapWriter_Write(r.writer, cCellUnion, cSize, cKey) == C.bool(false) {
		return errors.New("failed to write S2 region and key")
	}
	return nil
}

// Build finalizes the map and writes it to the file.
func (r *RoaringGeoMapWriter) Build(filePath string) error {
	cFilePath := C.CString(filePath)
	defer C.free(unsafe.Pointer(cFilePath))

	if C.RoaringGeoMapWriter_Build(r.writer, cFilePath) == C.bool(false) {
		return errors.New("failed to build RoaringGeoMap")
	}
	return nil
}

// Close cleans up the RoaringGeoMapWriter.
func (r *RoaringGeoMapWriter) Close() {
	if r.writer != nil {
		C.RoaringGeoMapWriter_Delete(r.writer)
		r.writer = nil
	}
}

// RoaringGeoMapReader wraps the C RoaringGeoMapReader.
type RoaringGeoMapReader struct {
	reader *C.RoaringGeoMapReader
}

// NewRoaringGeoMapReader creates a new instance of RoaringGeoMapReader.
func NewRoaringGeoMapReader(filePath string) (*RoaringGeoMapReader, error) {
	cFilePath := C.CString(filePath)
	defer C.free(unsafe.Pointer(cFilePath))

	r := C.RoaringGeoMapReader_New(cFilePath)
	if r == nil {
		return nil, errors.New("failed to create RoaringGeoMapReader")
	}
	return &RoaringGeoMapReader{reader: r}, nil
}

// Contains checks if the provided S2CellUnion is fully contained.
func (r *RoaringGeoMapReader) Contains(cellUnion []uint64) ([][]byte, error) {
	cCellUnion := (*C.uint64_t)(unsafe.Pointer(&cellUnion[0]))
	cSize := C.ulonglong(len(cellUnion))

	var bytesPtr *C.char
	var size C.ulonglong

	// Call the C function directly
	if C.RoaringGeoMapReader_Contains(r.reader, cCellUnion, cSize, &bytesPtr, &size) == 0 {
		return nil, errors.New("failed to query Contains")
	}
	defer C.free(unsafe.Pointer(bytesPtr))

	return cBytesToGo(bytesPtr, size), nil
}

// Intersects checks if the provided S2CellUnion intersects with the map.
func (r *RoaringGeoMapReader) Intersects(cellUnion []uint64) ([][]byte, error) {
	cCellUnion := (*C.uint64_t)(unsafe.Pointer(&cellUnion[0]))
	cSize := C.ulonglong(len(cellUnion))

	var bytesPtr *C.char
	var size C.ulonglong

	// Call the C function directly
	if C.RoaringGeoMapReader_Intersects(r.reader, cCellUnion, cSize, &bytesPtr, &size) == 0 {
		return nil, errors.New("failed to query Intersects")
	}
	defer C.free(unsafe.Pointer(bytesPtr))

	return cBytesToGo(bytesPtr, size), nil
}

// Close cleans up the RoaringGeoMapReader.
func (r *RoaringGeoMapReader) Close() {
	if r.reader != nil {
		C.RoaringGeoMapReader_Delete(r.reader)
		r.reader = nil
	}
}

// Helper function to convert raw bytes and size to Go [][]byte.
func cBytesToGo(bytesPtr *C.char, size C.ulonglong) [][]byte {
	goBytes := C.GoBytes(unsafe.Pointer(bytesPtr), C.int(size))
	// Split into [][]byte if applicable (e.g., null-terminated or prefixed-length encoding)
	// For simplicity, assuming here the bytes represent serialized entries.
	return [][]byte{goBytes}
}
