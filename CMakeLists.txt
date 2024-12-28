cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

project(RoaringGeoMaps VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install")
set(GOOGLETEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/cpp/vendor/googletest)
set(GOOGLETEST_VERSION 1.15.2)
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(ABSL_ENABLE_INSTALL ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
set(ROARING_BUILD_STATIC ON CACHE INTERNAL "")
set(ROARING_LINK_STATIC ON CACHE INTERNAL "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".dylib")

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpp/vendor/abseil-cpp EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpp/vendor/s2geometry EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpp/vendor/CRoaring EXCLUDE_FROM_ALL)

include_directories(cpp/vendor/s2geometry/src)
include_directories(cpp/vendor/CRoaring/cpp)
include_directories(cpp/vendor/CRoaring/include)
include_directories(cpp/vendor/SuRF/include)
include_directories(c/include)

# Add the new source files to the library
add_library(RoaringGeoMapsLib STATIC
        cpp/src/RoaringGeoMapWriter.cpp
        cpp/src/RoaringGeoMapReader.cpp
        cpp/src/ByteColumnWriter.cpp
        cpp/src/RoaringBitmapColumnWriter.cpp
        cpp/src/CellIdColumnWriter.cpp
        cpp/src/Header.cpp
        cpp/src/io/FileWriteBuffer.cpp
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
        cpp/src/Block.cpp
        cpp/src/Block.cpp
        cpp/benchmarks/main.cpp
        cpp/src/S2BlockIndexReader.cpp
        cpp/src/CellFilter.h
        cpp/src/CellFilter.cpp)

target_link_libraries(
    RoaringGeoMapsLib
    PRIVATE s2
    PRIVATE roaring
)



target_include_directories(RoaringGeoMapsLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cpp/src )

# Add the C interface library
add_library(RoaringGeoMapsCInterface STATIC
        c/src/roaring_geo_map_writer.cpp
        c/src/roaring_geo_map_reader.cpp
        )

target_link_libraries(
        RoaringGeoMapsCInterface
        PRIVATE
        RoaringGeoMapsLib
        s2
#        ${OPENSSL_SSL_LIBRARY}
#        ${OPENSSL_CRYPTO_LIBRARY}
        roaring
        absl::base
        absl::btree
        absl::check
        absl::config
        absl::core_headers
        absl::dynamic_annotations
        absl::endian
        absl::fixed_array
        absl::flags
        absl::flat_hash_map
        absl::flat_hash_set
        absl::hash
        absl::inlined_vector
        absl::int128
        absl::log
        absl::log_severity
        absl::memory
        absl::span
        absl::status
        absl::str_format
        absl::strings
        absl::type_traits
        absl::utility
        absl::vlog_is_on
)

set(STATIC_LIBS
    ${OPENSSL_SSL_LIBRARY}
    ${OPENSSL_CRYPTO_LIBRARY}
    $<TARGET_FILE:absl::base>
    $<TARGET_FILE:absl::synchronization>
    #$<TARGET_FILE:absl::log>
    $<TARGET_FILE:s2>
    $<TARGET_FILE:roaring>
    $<TARGET_FILE:RoaringGeoMapsLib>
    $<TARGET_FILE:RoaringGeoMapsCInterface>
        )

# Print the values of OpenSSL libraries
message(STATUS "OPENSSL_CRYPTO_LIBRARY: ${OPENSSL_CRYPTO_LIBRARY}")
message(STATUS "OPENSSL_SSL_LIBRARY: ${OPENSSL_SSL_LIBRARY}")
message(STATUS "OPENSSL_LIBRARY: ${OPENSSL_LIBRARIES}")

#
## Print the values of all Abseil libraries
#message(STATUS "ABSL_LIBS:")
#foreach(LIB ${ABSL_LIBS})
#    message(STATUS "  ${LIB}")
#endforeach()

set(OUTPUT_LIB roaringgeomaps.a)
add_custom_target(
        combined ALL
        COMMAND libtool -static -o ${OUTPUT_LIB} ${STATIC_LIBS}
        COMMENT "Combining static libraries into ${OUTPUT_LIB}"
)



# Install static libraries into build/lib directory
#install(TARGETS RoaringGeoMapsCInterface RoaringGeoMapsLib s2 roaring  ARCHIVE DESTINATION ${CMAKE_BINARY_DIR}/lib)
#install(FILES ${OPENSSL_LIBRARIES} DESTINATION ${CMAKE_BINARY_DIR}/lib)
#file(GLOB_RECURSE ABSEIL_LIBS "${CMAKE_BINARY_DIR}/cpp/vendor/abseil-cpp/libabsl*.a")
#install(FILES ${ABSEIL_LIBS} DESTINATION ${CMAKE_BINARY_DIR}/lib)
file(GLOB FAT_LIB "${CMAKE_BINARY_DIR}/roaringgeomaps.a.a")
install(FILES ${FAT_LIB} DESTINATION ${CMAKE_BINARY_DIR}/lib)
#file(GLOB_RECURSE OPENSSL_LIBS "${OPENSSL_LIBRARIES}/*.a")
#install(FILES ${OPENSSL_LIBS} DESTINATION ${CMAKE_BINARY_DIR}/lib)

# Copy library shared object to each consuming languages src or equivalent dir.
set(GO_ARTIFACT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/go/native/") ## TODO: copy values into package specific dirs at build time
# Copy all contents of the lib directory to the target directory
install(DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
        DESTINATION "${GO_ARTIFACT_DIR}"
        FILES_MATCHING  # Only copy matching files
        PATTERN "roaringgeomaps.a"   # Copy only static libraries
        )


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

add_executable(RoaringGeoMapsBenchmarks cpp/benchmarks/main.cpp c/include/roaring_geo_map_writer.h c/src/roaring_geo_map_writer.cpp c/src/roaring_geo_map_reader.cpp)
target_link_libraries(
    RoaringGeoMapsBenchmarks
    RoaringGeoMapsLib
    s2
    ${OPENSSL_LIBRARIES}
)


