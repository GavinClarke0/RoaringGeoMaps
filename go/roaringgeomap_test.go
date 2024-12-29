package geomap

import (
	"testing"
)

func TestRoaringGeoMapWriter(t *testing.T) {
	writer, err := NewRoaringGeoMapWriter(10)
	if err != nil {
		t.Fatalf("failed to create writer: %v", err)
	}
	defer writer.Close()

	// Example S2CellUnion
	cellUnion := []uint64{12345, 67890}
	err = writer.Write(cellUnion, "example_key")
	if err != nil {
		t.Fatalf("failed to write S2CellUnion: %v", err)
	}

	err = writer.Build("test_map.rgm")
	if err != nil {
		t.Fatalf("failed to build map: %v", err)
	}
}

func TestRoaringGeoMapReader(t *testing.T) {
	reader, err := NewRoaringGeoMapReader("test_map.rgm")
	if err != nil {
		t.Fatalf("failed to create reader: %v", err)
	}
	defer reader.Close()

	cellUnion := []uint64{12345, 67890}
	results, err := reader.Contains(cellUnion)
	if err != nil {
		t.Fatalf("failed to query Contains: %v", err)
	}
	t.Logf("Contains results: %v", results)

	results, err = reader.Intersects(cellUnion)
	if err != nil {
		t.Fatalf("failed to query Intersects: %v", err)
	}
	t.Logf("Intersects results: %v", results)
}
