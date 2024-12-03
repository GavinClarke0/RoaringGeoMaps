#include "FileReadBuffer.h"
#include <fstream>
#include <stdexcept>


// Simple buffer that reads the entire index file into memory/
FileReadBuffer::FileReadBuffer(const std::string& filename) {
    std::ifstream fileStream(filename, std::ios::binary | std::ios::ate);
    if (!fileStream) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::streamsize fileSize = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);

    buffer_size = fileSize;
    buffer = static_cast<char*>(std::aligned_alloc(32, (fileSize + 31) & ~31));
    if (buffer == nullptr) {
        throw std::runtime_error("Failed to allocate memory");
    }

    if (!fileStream.read(buffer, fileSize)) {
        throw std::runtime_error("Failed to read file: " + filename);
    }
}

FileReadBuffer::~FileReadBuffer(){
    std::free(buffer);
}


const char* FileReadBuffer::data() const {
    return buffer;
}

uint64_t FileReadBuffer::size() const {
    return buffer_size;
}

const char* FileReadBuffer::view(uint64_t offset, uint64_t length) const {
    if (offset + length > buffer_size) {
        throw std::out_of_range("View range is out of buffer bounds");
    }
    return buffer + offset;
}
