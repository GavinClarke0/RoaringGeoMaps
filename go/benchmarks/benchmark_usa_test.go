package benchmarks

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"testing"

	"github.com/golang/geo/s2"
)

// GeoJSONFeature represents a single GeoJSON Feature
type GeoJSONFeature struct {
	Type       string                 `json:"type"`
	Properties map[string]interface{} `json:"properties"`
	Geometry   struct {
		Type        string          `json:"type"`
		Coordinates json.RawMessage `json:"coordinates"`
	} `json:"geometry"`
}

// ReadGeoJSON reads and parses a GeoJSON file
func ReadGeoJSON(filePath string) ([]GeoJSONFeature, error) {
	data, err := ioutil.ReadFile(filePath)
	if err != nil {
		return nil, fmt.Errorf("failed to read file: %v", err)
	}

	var features struct {
		Type     string           `json:"type"`
		Features []GeoJSONFeature `json:"features"`
	}
	if err := json.Unmarshal(data, &features); err != nil {
		return nil, fmt.Errorf("failed to parse GeoJSON: %v", err)
	}

	return features.Features, nil
}

// ParsePolygonCoordinates parses GeoJSON polygon coordinates into S2 loops
func ParsePolygonCoordinates(coordinates json.RawMessage) (*s2.Loop, error) {
	var rings [][][]float64
	if err := json.Unmarshal(coordinates, &rings); err != nil {
		return nil, fmt.Errorf("failed to parse polygon coordinates: %v", err)
	}

	if len(rings) == 0 {
		return nil, fmt.Errorf("polygon has no rings")
	}

	var points []s2.Point
	for _, coord := range rings[0] { // Only process the outer ring
		if len(coord) != 2 {
			return nil, fmt.Errorf("invalid coordinate pair")
		}
		lat, lng := coord[1], coord[0]
		points = append(points, s2.PointFromLatLng(s2.LatLngFromDegrees(lat, lng)))
	}

	loop := s2.LoopFromPoints(points)
	return loop, nil
}

// CoverRegionWithS2Cells covers an S2 region (polygon) with S2 cells
func CoverRegionWithS2Cells(region s2.Region, maxCells int) []s2.CellID {
	coverer := &s2.RegionCoverer{
		MinLevel: 10, // Minimum S2 level
		MaxLevel: 20, // Maximum S2 level
		MaxCells: maxCells,
	}

	return coverer.Covering(region)
}

// BenchmarkGeoJSONToS2 tests the conversion of GeoJSON polygons to S2 cells
func BenchmarkGeoJSONToS2(b *testing.B) {
	// Generate a synthetic GeoJSON polygon
	geoJSON := `
	{
		"type": "FeatureCollection",
		"features": [
			{
				"type": "Feature",
				"geometry": {
					"type": "Polygon",
					"coordinates": [[[100.0, 0.0], [101.0, 0.0], [101.0, 1.0], [100.0, 1.0], [100.0, 0.0]]]
				},
				"properties": {}
			}
		]
	}`

	var geoJSONData struct {
		Type     string           `json:"type"`
		Features []GeoJSONFeature `json:"features"`
	}

	if err := json.Unmarshal([]byte(geoJSON), &geoJSONData); err != nil {
		b.Fatalf("Failed to parse GeoJSON: %v", err)
	}

	features := geoJSONData.Features

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		for _, feature := range features {
			if feature.Geometry.Type != "Polygon" {
				continue
			}

			loop, err := ParsePolygonCoordinates(feature.Geometry.Coordinates)
			if err != nil {
				b.Fatalf("Failed to parse polygon: %v", err)
			}

			// Create an S2Polygon from the loop
			polygon := s2.PolygonFromLoops([]*s2.Loop{loop})

			// Generate S2 cell cover
			_ = CoverRegionWithS2Cells(polygon, 10)
		}
	}
}

func main() {
	fmt.Println("Run `go test -bench=.` to execute the benchmark.")
}
