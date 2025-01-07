#ifndef ROARINGGEOMAPS_FILEWRITEBUFFER_H
#define ROARINGGEOMAPS_FILEWRITEBUFFER_H

#include <fstream>
#include <cstddef>
#include <vector>

class FileWriteBuffer {
public:
    explicit FileWriteBuffer(const std::string &filename, uint64_t initialSize = 1024);

    ~FileWriteBuffer();

    std::pair<uint64_t, uint64_t> write(const char *data, uint64_t size);

    std::pair<uint64_t, uint64_t> write(const std::function<void(char *)> &func, uint64_t size);

    std::pair<uint64_t, uint64_t> write32ByteAligned(const std::function<void(char *)> &func, uint64_t size);

    void flush(uint64_t offset);  // Flushes any remaining data in the buffer to the file at position offset
    void seek(uint64_t interval); // moves the buffer forward or backwards by pos
    void reset();

    uint64_t offset() const; // current offset in the buffer that the next section of data will be written too
    uint64_t size() const;

private:
    std::ofstream fileStream;  // File stream for writing
    std::string filename;      // Filename of the file being written to
    std::vector<char> buffer;
    uint64_t currentPos;
};

#endif //ROARINGGEOMAPS_FILEWRITEBUFFER_H
