// FileWriteBuffer.cpp

#include "FileWriteBuffer.h"
#include <cstring>
#include <iostream>


FileWriteBuffer::FileWriteBuffer(const std::string& filename, uint64_t initialSize) : filename(filename), buffer(initialSize), currentPos(0) {
    fileStream.open(filename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    if (!fileStream) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
}

std::pair<uint64_t, uint64_t> FileWriteBuffer::write(const char* data, uint64_t size) {
    auto offset = currentPos;
    if (offset + size > buffer.size()) {
        buffer.resize(offset + size);
    }
    std::memcpy(buffer.data() + offset, data, size);
    currentPos = offset + size;
    return {offset, size};
}

std::pair<uint64_t, uint64_t> FileWriteBuffer::write(const std::function<void(char*)>& func, uint64_t size) {
    auto offset = currentPos;
    if (offset + size > buffer.size()) {
        buffer.resize(offset + size);
    }
    // invoke the writer func on the buffer, this allows us to use functions that directly
    // write to a buffer instead of giving direct access to the buffer.
    func(buffer.data() + offset);
    currentPos = offset + size;
    return {offset, size};
}

std::pair<uint64_t, uint64_t> FileWriteBuffer::write32ByteAligned(const std::function<void(char*)>& func, uint64_t size) {
    // find next set of bytes that is 32 byte aligned;
    auto offset = (currentPos + 31) & ~31; // Find index that is 32 byte aligned.
    if (offset + size > buffer.size()) {
        buffer.resize(offset + size);
    }
    // invoke the writer func on the buffer, this allows us to use functions that directly
    // write to a buffer instead of giving direct access to the buffer.
    func(buffer.data() + offset);
    currentPos = offset + size;
    return {offset, size};
}


FileWriteBuffer::~FileWriteBuffer() {
    fileStream.close();
}

void FileWriteBuffer::reset() {
    currentPos = 0;
}

void FileWriteBuffer::seek(uint64_t interval) {

    if (currentPos + interval < 0) {
        throw std::runtime_error("seeking before start of buffer");
    }
    if (currentPos + interval > buffer.size()) {
        buffer.resize(currentPos + interval);
    }

    currentPos += interval;
}

uint64_t FileWriteBuffer::size() const {
    return buffer.size();
}

uint64_t FileWriteBuffer::offset() const {
    return currentPos;
}

void FileWriteBuffer::flush(uint64_t offset) {

    fileStream.seekp(offset);
    fileStream.write(buffer.data(), buffer.size());
    fileStream.flush();
    currentPos = 0;
}
