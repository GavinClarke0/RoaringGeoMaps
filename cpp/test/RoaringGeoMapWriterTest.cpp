#include <gtest/gtest.h>
#include <s2/s2latlng.h>
#include <s2/s2cell_id.h>
#include <s2/s2region_coverer.h>
#include <s2/s2polygon.h>
#include <s2/s2loop.h>
#include "RoaringGeoMapWriter.h"
#include "RoaringGeoMapReader.h"


TEST(RoaringGeoMapWriterTest, WriteSingleCellId) {
    // Arrange
    RoaringGeoMapWriter writer(3);

    // Define a single S2CellId
    S2LatLng latlng = S2LatLng::FromDegrees(37.7749, -122.4194); // San Francisco
    S2CellId cellId(latlng.ToPoint());
    S2CellUnion cellUnion;
    cellUnion.Init({cellId});

    // Write the cell ID to the index
    writer.write(cellUnion, "single-id");

    // Build the index
    std::string testFilePath = "test_single_cell_id.roaring";
    bool result = writer.build(testFilePath);

    // Assert
    ASSERT_TRUE(result) << "Failed to build the RoaringGeoMapWriter file.";

    // Read and verify the result
    RoaringGeoMapReader reader(testFilePath);
    auto queryResults = reader.Contains(cellUnion);
    ASSERT_EQ(queryResults.size(), 1);
    std::string resultsStr(queryResults[0].begin(),queryResults[0].end());
    ASSERT_EQ(resultsStr, "single-id");

    // Clean up
    std::remove(testFilePath.c_str());
}

TEST(RoaringGeoMapWriterTest, WriteAndQueryPolygon) {
    // Arrange
    RoaringGeoMapWriter writer(3);

    // Define a square polygon
    std::vector<S2LatLng> vertices = {
            S2LatLng::FromDegrees(0, 0),
            S2LatLng::FromDegrees(0, 1),
            S2LatLng::FromDegrees(1, 1),
            S2LatLng::FromDegrees(1, 0)
    };

    std::vector<S2Point> s2_points;
    for (const auto& latlng : vertices) {
        s2_points.push_back(latlng.ToPoint());
    }

    std::vector<std::unique_ptr<S2Loop>> loops;
    loops.push_back(std::make_unique<S2Loop>(std::move(S2Loop(s2_points))));
    S2Polygon polygon(std::move(loops));

    // Cover the polygon
    S2RegionCoverer::Options options;
    options.set_max_cells(10);
    S2RegionCoverer coverer(options);
    std::vector<S2CellId> covering;
    coverer.GetCovering(polygon, &covering);

    S2CellUnion cellUnion;
    cellUnion.Init(covering);

    // Write to the index
    writer.write(cellUnion, "polygon-id");
    std::string testFilePath = "test_polygon.roaring";
    ASSERT_TRUE(writer.build(testFilePath));

    // Query with a point inside the polygon
    S2LatLng queryLatLng = S2LatLng::FromDegrees(0.5, 0.5);
    S2CellId queryCell(queryLatLng.ToPoint());
    S2CellUnion queryUnion;
    queryUnion.Init({queryCell});

    RoaringGeoMapReader reader(testFilePath);
    auto queryResults = reader.Contains(queryUnion);

    // Assert
    ASSERT_EQ(queryResults.size(), 1);
    std::string resultsStr(queryResults[0].begin(),queryResults[0].end());
    ASSERT_EQ(resultsStr, "polygon-id");

    // Clean up
    std::remove(testFilePath.c_str());
}

TEST(RoaringGeoMapWriterTest, QueryNonExistentCellId) {
    // Arrange
    RoaringGeoMapWriter writer(3);

    // Write some dummy data
    S2LatLng latlng = S2LatLng::FromDegrees(37.7749, -122.4194); // San Francisco
    S2CellId cellId(latlng.ToPoint());
    S2CellUnion cellUnion;
    cellUnion.Init({cellId});

    writer.write(cellUnion, "existing-id");
    std::string testFilePath = "test_non_existent.roaring";
    ASSERT_TRUE(writer.build(testFilePath));

    // Query with a cell ID not in the index
    S2LatLng queryLatLng = S2LatLng::FromDegrees(-90.0, 0.0); // South Pole (not indexed)
    S2CellId queryCell(queryLatLng.ToPoint());
    S2CellUnion queryUnion;
    queryUnion.Init({queryCell});

    RoaringGeoMapReader reader(testFilePath);
    auto queryResults = reader.Contains(queryUnion);

    // Assert
    ASSERT_EQ(queryResults.size(), 0);

    // Clean up
    std::remove(testFilePath.c_str());
}


// S2 test functions

// Function to generate a random latitude and longitude within the United States
std::pair<double, double> generateRandomLatLngInUS() {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    // Latitude range for the contiguous United States (approximate)
    std::uniform_real_distribution<> lat_dist(24.396308, 49.384358);
    // Longitude range for the contiguous United States (approximate)
    std::uniform_real_distribution<> lng_dist(-125.0, -66.93457);

    double lat = lat_dist(gen);
    double lng = lng_dist(gen);
    return {lat, lng};
}

// Function to generate an S2 point within the United States and convert it to an S2CellId at level 30
S2CellId generateS2CellIdInUS() {
    auto [lat, lng] = generateRandomLatLngInUS();

    // Convert latitude and longitude to an S2Point
    S2LatLng s2_latlng = S2LatLng::FromDegrees(lat, lng);
    S2Point s2_point = s2_latlng.ToPoint();

    // Convert the S2Point to an S2CellId at level 30
    auto cell_id = S2CellId(s2_point);

    return cell_id;
}

std::vector<S2CellId> convertTokensToCellIds(const std::vector<std::string>& tokens) {
    std::vector<S2CellId> cellIds;
    for (const auto& token : tokens) {
        // Convert each token to an S2CellId
        S2CellId cellId = S2CellId::FromToken(token);
        cellIds.push_back(cellId);
    }
    return cellIds;
}

TEST(RoaringGeoMapWriterTest, WriteS2RegionCover) {
    // Arrange
    RoaringGeoMapWriter writer(3);

    // Define a simple polygon (e.g., a triangle) using lat/lng points
    std::vector<S2LatLng> vertices = {
            S2LatLng::FromDegrees(37.7749, -122.4194),  // San Francisco
            S2LatLng::FromDegrees(34.0522, -118.2437),  // Los Angeles
            S2LatLng::FromDegrees(36.1699, -115.1398)   // Las Vegas
    };

    // Convert lat/lng points to S2Points
    std::vector<S2Point> s2_points;
    for (const auto& latlng : vertices) {
        s2_points.push_back(latlng.ToPoint());
    }

    // Convert the loop to a polygon
    std::vector<std::unique_ptr<S2Loop>> loops;
    loops.push_back(std::make_unique<S2Loop>(std::move(S2Loop(s2_points))));
    S2Polygon polygon(std::move(loops));

    // Set up the S2RegionCoverer
    S2RegionCoverer::Options options;
    options.set_max_cells(100);  // Limit the number of cells in the cover
    S2RegionCoverer coverer(options);

    // Get the covering of the polygon
    std::vector<S2CellId> covering;
    coverer.GetCovering(polygon, &covering);

    S2CellUnion cellUnion;
    cellUnion.Init(covering);
    writer.write(cellUnion, "shapefile-id");

    for (int i = 0; i < 2500; i++) {
        S2CellUnion pointCellUnion;
        std::vector<S2CellId> pointCells;
        pointCells.push_back(generateS2CellIdInUS());
        pointCellUnion.Init(pointCells);

        writer.write(pointCellUnion, std::to_string(i));
    }

    // Temporary file path for testing
    std::string testFilePath = "test_output_file.roaring";
    bool result = writer.build(testFilePath);

    // Assert
    ASSERT_TRUE(result) << "Failed to build the RoaringGeoMapWriter file.";

    std::vector<std::string> tokens = {
            "80c462b4", "80c462cc", "80c462d4", "80c462d9", "80c4633",
            "80c4634c", "80c46354", "80c464ac", "80c47854", "80c478d",
            "80c478f", "80c4794", "80c4799", "80c479b", "80c47a3",
            "80c47a5", "80c47ae2b", "80c47afc", "80c47b4", "80c47bc",
            "80c47d", "80c47e1", "80c47e3fc", "80c47e5", "80c47e7",
            "80c47ec", "80c47f04", "80c47f1c", "80c47f3", "80c4875c",
            "80c48764", "80c4877c", "80c48784", "80c4878c", "80c487f54"
    };

    // Convert tokens to S2CellIds
    std::vector<S2CellId> cellIds = convertTokensToCellIds(tokens);
    auto reader = RoaringGeoMapReader("test_output_file.roaring");

    S2CellUnion pointCellUnion;
    pointCellUnion.Init(cellIds);
    for (int i = 0; i < 500; i++) {
        S2CellUnion pointCellUnion;
        std::vector<S2CellId> pointCells;
        pointCells.insert(pointCells.end(), cellIds.begin(), cellIds.end());
        pointCells.push_back(generateS2CellIdInUS());
        pointCellUnion.Init(pointCells);

        auto queryResults = reader.Contains(pointCellUnion);
        ASSERT_TRUE(queryResults.size() > 0) << "Failed to build the RoaringGeoMapWriter file.";
    }
    std::remove(testFilePath.c_str());
}
