#ifndef ROARINGGEOMAPS_VECTORVIEW_H
#define ROARINGGEOMAPS_VECTORVIEW_H

#include "io/FileReadBuffer.h"
#include <vector>
#include <concepts>
#include <cstdint>
#include <iterator>

//// VectorView returns a vector like and iterator data structure over a section of the read buffer that contains a series of
//// integrals. Data accessed through value operators is endian safe and does not preform any copies of the underlaying
//// data.
template<std::integral T>
class VectorView {
public:
    VectorView(FileReadBuffer &f, uint64_t pos, uint64_t size);

    // Iterator class
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T;
        using reference = T;

        // Constructor initializes pos_ as a pointer to the start of the range in `VectorView`
        Iterator(VectorView *view, pointer pos) : pos_(pos), _view(view) {}

        // Dereference operator returns a pointer to the element at the current position
        pointer operator*() const { return (*_view)[pos_]; }

        pointer operator->() const { return (*_view)[pos_]; }

        // Pre-increment
        Iterator &operator++() {
            ++pos_;
            return *this;
        }

        // Post-increment
        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        Iterator &operator--() {
            --pos_;
            return *this;
        }

        // Post-increment
        Iterator operator--(int) {
            Iterator temp = *this;
            --(*this);
            return temp;
        }

        bool operator==(const Iterator &other) const { return pos_ == other.pos_; }

        bool operator!=(const Iterator &other) const { return pos_ != other.pos_; }

        bool operator<(const Iterator &other) const { return pos_ < other.pos_; }

    private:
        pointer pos_;  // Pointer to the current element in `values`
        VectorView *_view; //TODO: may want to use multi pointer for memory safety.
    };

    Iterator begin() { return Iterator(this, 0); }

    Iterator end() { return Iterator(this, values.size()); }

    // Element access
    T operator[](uint64_t i) { return littleEndian(values[i]); }

    const T operator[](uint64_t i) const { return littleEndian(values[i]); }

    uint64_t size() { return values.size(); };

private:
    std::vector<T> values;
};

template<std::integral T>
VectorView<T>::VectorView(FileReadBuffer &f, uint64_t pos, uint64_t size) {
    const auto *tPtr = reinterpret_cast<const T *>(f.view(pos, size * sizeof(T)));
    values = std::vector<T>(tPtr, tPtr + size);
}

#endif //ROARINGGEOMAPS_VECTORVIEW_H

