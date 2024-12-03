cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

project(RoaringGeoMaps VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3")
set(GOOGLETEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/cpp/vendor/googletest)
set(GOOGLETEST_VERSION 1.15.2)
set(ABSL_ENABLE_INSTALL ON)
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(ROARING_BUILD_STATIC ON CACHE INTERNAL "")
set(ROARING_LINK_STATIC ON CACHE INTERNAL "")

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpp/vendor/abseil-cpp EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpp/vendor/s2geometry EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpp/vendor/CRoaring EXCLUDE_FROM_ALL)

include_directories(cpp/vendor/s2geometry/src)
include_directories(cpp/vendor/CRoaring/cpp)
include_directories(cpp/vendor/CRoaring/include)

# Add the new source files to the library
add_library(RoaringGeoMapsLib STATIC 
    cpp/src/RoaringGeoMapWriter.cpp 
    cpp/src/RoaringGeoMapWriter.h 
    cpp/src/RoaringGeoMapReader.cpp
    cpp/src/ByteColumnWriter.cpp
    cpp/src/ByteColumnWriter.h
    cpp/src/RoaringBitmapColumnWriter.cpp
    cpp/src/CellIdColumnWriter.cpp
    cpp/src/CellIdColumnWriter.h
    cpp/src/Header.cpp
    cpp/src/Header.h
    cpp/src/BlockIndexReader.h
    cpp/src/io/FileWriteBuffer.cpp
    cpp/src/io/FileWriteBuffer.h
    cpp/src/io/FileReadBuffer.cpp
    cpp/src/io/FileReadBuffer.h
    cpp/src/ReaderHelpers.h
    cpp/src/RoaringGeoMapReader.h
    cpp/src/endian/endian.h
    cpp/src/CellIdColumnReader.cpp
    cpp/src/CellIdColumnReader.h
    cpp/src/RoaringBitmapColumnReader.cpp
    cpp/src/RoaringBitmapColumnReader.h
    cpp/src/ByteColumnReader.cpp
    cpp/src/ByteColumnReader.h
    cpp/src/VectorView.h
    cpp/src/BlockIndexWriter.cpp
    cpp/src/BlockIndexWriter.h
    cpp/src/Block.h
    cpp/src/BlockFilter.h cpp/src/Block.cpp cpp/src/Block.cpp cpp/benchmarks/main.cpp)

target_link_libraries(
    RoaringGeoMapsLib
    roaring
    s2
    ${OPENSSL_LIBRARIES}
)

target_include_directories(RoaringGeoMapsLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cpp/src )

add_executable(RoaringGeoMapsTests cpp/test/RoaringGeoMapWriterTest.cpp cpp/test/main.cpp)
target_link_libraries(
    RoaringGeoMapsTests
    RoaringGeoMapsLib
    gtest
    gtest_main
    roaring
    s2
    ${OPENSSL_LIBRARIES}
)

add_executable(RoaringGeoMapsBenchmarks cpp/benchmarks/main.cpp)
target_link_libraries(
        RoaringGeoMapsBenchmarks
        RoaringGeoMapsLib
        roaring
        s2
        ${OPENSSL_LIBRARIES}
)


