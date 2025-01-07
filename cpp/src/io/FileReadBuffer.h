#ifndef ROARINGGEOMAPS_FILEREADBUFFER_H
#define ROARINGGEOMAPS_FILEREADBUFFER_H

#include <string>
#include <vector>
#include <string_view>

class FileReadBuffer {
public:
    explicit FileReadBuffer(const std::string &filename);

    ~FileReadBuffer();

    const char *data() const;

    uint64_t size() const;

    const char *view(uint64_t offset, uint64_t length) const;

private:
    char *buffer;
    uint64_t buffer_size;
};

#endif //ROARINGGEOMAPS_FILEREADBUFFER_H
