package geomap

import (
	"github.com/golang/geo/s2"
	"math/rand"
	"testing"
)

func TestRoaringGeoMapReader(t *testing.T) {

	writer, err := NewRoaringGeoMapWriter(3)
	if err != nil {
		t.Fatalf("failed to create writer: %v", err)
	}
	defer writer.Close()

	// Example S2CellUnion
	cellUnion := s2.CellUnion{9260949627242122337}
	err = writer.Write(cellUnion, "example_key")
	if err != nil {
		t.Fatalf("failed to write S2CellUnion: %v", err)
	}

	err = writer.Write(cellUnion, "little_papi_2")
	if err != nil {
		t.Fatalf("failed to write S2CellUnion: %v", err)
	}

	err = writer.Build("test_map_2.rgm")
	if err != nil {
		t.Fatalf("failed to build map: %v", err)
	}

	reader, err := NewRoaringGeoMapReader("test_map.rgm")
	if err != nil {
		t.Fatalf("failed to create reader: %v", err)
	}
	defer reader.Close()

	results, err := reader.Contains(cellUnion)
	if err != nil {
		t.Fatalf("failed to query Contains: %v", err)
	}

	t.Logf("Contains results: %s", string(results[0]))
}

func randomLatLon() (float64, float64) {
	lat := rand.Float64()*180 - 90  // Latitude: -90 to 90
	lon := rand.Float64()*360 - 180 // Longitude: -180 to 180
	return lat, lon
}

func BenchmarkRoaringGeoMapReader(b *testing.B) {

	// Create writer
	writer, err := NewRoaringGeoMapWriter(3)
	if err != nil {
		b.Fatalf("failed to create writer: %v", err)
	}
	defer writer.Close()

	// Write random places to the map
	var points []s2.CellID
	for i := 0; i < b.N; i++ {
		lat, lon := randomLatLon()
		cellID := s2.CellIDFromLatLng(s2.LatLngFromDegrees(lat, lon))
		points = append(points, cellID)

		cellUnion := s2.CellUnion{cellID}
		err := writer.Write(cellUnion, string([]byte{byte(i % 256)}))
		if err != nil {
			b.Fatalf("failed to write S2CellUnion: %v", err)
		}
	}

	// Build the map
	err = writer.Build("benchmark_map.rgm")
	if err != nil {
		b.Fatalf("failed to build map: %v", err)
	}

	// Create reader
	reader, err := NewRoaringGeoMapReader("benchmark_map.rgm")
	if err != nil {
		b.Fatalf("failed to create reader: %v", err)
	}
	defer reader.Close()

	// Query random places
	for i := 0; i < b.N; i++ {

		cellUnion := s2.CellUnion{points[i]}
		_, err := reader.Contains(cellUnion)
		if err != nil {
			b.Fatalf("failed to query Contains: %v", err)
		}
	}
}
