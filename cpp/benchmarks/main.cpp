#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <algorithm>
#include "RoaringGeoMapWriter.h"
#include "RoaringGeoMapReader.h"
#include "s2/s2earth.h"
#include <s2/s2latlng.h>
#include <s2/s2cell_id.h>
#include <s2/s2region_coverer.h>
#include <s2/s2cap.h>

#include <random>


// Function to generate random latitude and longitude within the United States
std::pair<double, double> generateRandomLatLngInUS() {
    static std::random_device rd;
    static std::mt19937 gen(rd()); // Seed mt19937 with random_device

    // Latitude range for the contiguous United States (approximate)
    static std::uniform_real_distribution<> lat_dist(24.396308, 49.384358);
    // Longitude range for the contiguous United States (approximate)
    static std::uniform_real_distribution<> lng_dist(-125.0, -66.93457);

    double lat = lat_dist(gen);
    double lng = lng_dist(gen);
    return {lat, lng};
}

// Function to generate a circle (S2Cap) with a random center and a fixed radius
S2Cap generateRandomCircle(double radius_meters) {
    auto [lat, lng] = generateRandomLatLngInUS();
    S2LatLng s2_latlng = S2LatLng::FromDegrees(lat, lng);

    // Create a circle (S2Cap) centered at the random point with the given radius
    S2Cap circle = S2Cap::FromCenterHeight(s2_latlng.ToPoint(), S2Earth::MetersToRadians(radius_meters));
    return circle;
}

// Function to generate cellIds for a circle
std::vector<S2CellId> coverCircleWithCells(S2Cap& circle) {
    S2RegionCoverer::Options options;
    options.set_max_cells(15);  // Limit the number of cells in the cover
    S2RegionCoverer coverer(options);

    std::vector<S2CellId> covering;
    coverer.GetCovering(circle, &covering);
    return covering;
}

// Function to benchmark writing circles to the index
void benchmarkWriteCircles(RoaringGeoMapWriter& writer, int numCircles, double radius_meters, std::vector<std::vector<S2CellId>>& indexedCellIds) {
    for (int i = 0; i < numCircles; ++i) {
        S2Cap circle = generateRandomCircle(radius_meters);
        std::vector<S2CellId> covering = coverCircleWithCells(circle);
        S2CellUnion cellUnion;
        cellUnion.Init(covering);
        writer.write(cellUnion, std::to_string(i));

        // Store the indexed cellIds for future queries
        indexedCellIds.push_back(covering);
    }
}

// Function to benchmark querying the index with circles
void benchmarkQueryExecution(RoaringGeoMapReader& reader, int numQueries, const std::vector<std::vector<S2CellId>>& indexedCellIds) {
    // Store query execution times
    std::vector<long long> execution_times;

    for (int i = 0; i < numQueries; ++i) {
        // Select a circle randomly from the indexed circles
        const auto& covering = indexedCellIds[i % indexedCellIds.size()]; // Ensure we query an indexed circle
        S2CellUnion cellUnion;
        cellUnion.Init(covering);

        // Measure the query execution time
        auto start_time = std::chrono::high_resolution_clock::now();
        auto queryResults = reader.Contains(cellUnion);
        auto end_time = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        execution_times.push_back(duration);
    }

    // Compute mean and p99
    long long sum = 0;
    for (const auto& time : execution_times) {
        sum += time;
    }
    double mean = static_cast<double>(sum) / execution_times.size();

    // Sort to calculate p99
    std::sort(execution_times.begin(), execution_times.end());
    double p99 = execution_times[static_cast<int>(execution_times.size() * 0.99)];

    std::cout << "Mean query execution time: " << mean << " microseconds\n";
    std::cout << "99th percentile (p99) query execution time: " << p99 << " microseconds\n";
}

int main() {
    // Create a writer and reader for the benchmark

    auto fileName = "benchmark_file.roaring";
    RoaringGeoMapWriter writer(3);

    // Number of circles and radius of circles in meters for benchmarking
    auto circleCountTestCases = {5000, 10000, 50000};
    auto radiusMetersTestCases = {5, 10, 100, 1000};

    for (auto circleCount : circleCountTestCases) {
        for (auto radiusMeters: radiusMetersTestCases) {
            // Vector to store indexed cellIds
            std::vector<std::vector<S2CellId>> indexedCellIds;

            std::cout << "Bench Mark: [Circles: " << circleCount << "] [Radius: " << radiusMeters << "m]\n\n";

            // Benchmark writing circles
            auto start_write = std::chrono::high_resolution_clock::now();
            benchmarkWriteCircles(writer, circleCount, radiusMeters, indexedCellIds);
            auto end_write = std::chrono::high_resolution_clock::now();
            auto write_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_write - start_write).count();
            std::cout << "Write benchmark completed in " << write_duration << " ms.\n";

            auto start_build = std::chrono::high_resolution_clock::now();
            writer.build(fileName);
            auto end_build= std::chrono::high_resolution_clock::now();
            auto build_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_build - start_build).count();
            std::cout << "Build benchmark completed in " << build_duration << " ms.\n";

            // Benchmark querying the index with circles
            auto start_init = std::chrono::high_resolution_clock::now();
            RoaringGeoMapReader reader(fileName);
            auto end_init = std::chrono::high_resolution_clock::now();
            auto init_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_init - start_init).count();
            std::cout << "Init benchmark completed in " << init_duration << " ms.\n";


            auto start_query = std::chrono::high_resolution_clock::now();
            benchmarkQueryExecution(reader, 1000, indexedCellIds);
            auto end_query = std::chrono::high_resolution_clock::now();
            auto query_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_query - start_query).count();
            std::cout << "Query benchmark completed in " << query_duration << " ms.\n";
        }
    }
    return 0;
}