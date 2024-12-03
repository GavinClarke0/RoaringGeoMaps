#include "Header.h"
#include "WriteHelpers.h"
#include "ReaderHelpers.h"
#include "io/FileReadBuffer.h"

/*
 *
 * Header Classes reads and writes key header data to RoaringGeomaps File. It also provides
 * helper methods for retrieving offsets of values
 * ..
 *
 * Header Format
 * [Header Size uint32]
 * [coverBitMap offset uint64]
 * [containsBitMap offset uint64]
 * [KeyIndex offset uint64]
 * [cellIndexOffset offset uint64]
 * [keyIndex entries count uint32]
 * [cellIndex entries count uint32]
 * [block size uint_16] # A block is the maximum number of rows in 1 block. key and cell indexes are re-indexed via a skip index with a per block entry
 * [levelIndexBucketRange uint_8] # levelIndexBucketRange of the file
 * [file type uint_8] # unused for now value to indicate the file type, this is included to allow future variations of the file format
 * [16 reserved bytes for future use]
 *
 */


Header::Header(int levelIndexBucketRange): levelIndexBucketRange(levelIndexBucketRange), blockSize(512), fileType(0) {
}
Header::Header() = default;

void Header::writeToFile(FileWriteBuffer &buffer) const {

    writeLittleEndianUint64(buffer, HEADER_SIZE);
    writeLittleEndianUint64(buffer, coverBitmapOffset);
    writeLittleEndianUint64(buffer, coverBitmapSize);
    writeLittleEndianUint64(buffer, containsBitmapOffset);
    writeLittleEndianUint64(buffer, containsBitmapSize);

    writeLittleEndianUint64(buffer, keyIndexOffset);
    writeLittleEndianUint64(buffer, keyIndexSize);
    writeLittleEndianUint64(buffer, cellIndexOffset);
    writeLittleEndianUint64(buffer, cellIndexSize);
    writeLittleEndianUint64(buffer, roaringIndexOffset);
    writeLittleEndianUint64(buffer, roaringIndexSize);

    writeLittleEndianUint32(buffer, keyIndexEntries);
    writeLittleEndianUint32(buffer, cellIndexEntries);

    writeLittleEndianUint8(buffer, levelIndexBucketRange);
    writeLittleEndianUint16(buffer, blockSize);
    writeLittleEndianUint8(buffer, fileType); // Type 1 currently means standard non block compressed format.
    buffer.write(std::vector<char>(28, 0).data(), 28);
}

Header Header::readFromFile(FileReadBuffer &buffer) {

    Header header = Header();
    auto headerOffset = readLittleEndianUint64(buffer, 0); // TODO: figure out a case where we will need this
    header.coverBitmapOffset = readLittleEndianUint64(buffer, 8);
    header.coverBitmapSize = readLittleEndianUint64(buffer, 16);
    header.containsBitmapOffset = readLittleEndianUint64(buffer, 24);
    header.containsBitmapSize = readLittleEndianUint64(buffer, 32);

    header.keyIndexOffset = readLittleEndianUint64(buffer, 40);
    header.keyIndexSize = readLittleEndianUint64(buffer, 48);
    header.cellIndexOffset = readLittleEndianUint64(buffer, 56);
    header.cellIndexSize = readLittleEndianUint64(buffer, 64);
    header.roaringIndexOffset = readLittleEndianUint64(buffer, 72);
    header.roaringIndexSize = readLittleEndianUint64(buffer, 80);

    header.keyIndexEntries = readLittleEndianUint32(buffer, 88);
    header.cellIndexEntries = readLittleEndianUint32(buffer, 92);

    header.levelIndexBucketRange = readLittleEndianUint8(buffer, 96);
    header.blockSize = readLittleEndianUint16(buffer, 97);
    header.fileType = readLittleEndianUint8(buffer, 99); // Type 1 currently means standard non block compressed format.
    return header;
}


std::pair<uint64_t, uint64_t> Header::getCoverBitmapOffset() const {
    return {coverBitmapOffset, coverBitmapSize};
}


void Header::setCoverBitmapOffset(uint64_t offset, uint64_t size) {
    Header::coverBitmapOffset = offset;
    Header::coverBitmapSize = size;
}


std::pair<uint64_t, uint64_t> Header::getContainsBitmapOffset() const {
    return {containsBitmapOffset, containsBitmapSize};
}

void Header::setContainsBitmapOffset(uint64_t offset, uint64_t size) {
    Header::containsBitmapOffset = offset;
    Header::containsBitmapSize = size;
}


std::pair<uint64_t, uint64_t> Header::getKeyIndexPos() const {
    return {keyIndexOffset , keyIndexSize} ;
}

std::pair<uint64_t, uint64_t> Header::getCellIndexPos() const {
    return {cellIndexOffset, cellIndexSize};
}

std::pair<uint64_t, uint64_t> Header::getBitmapPos() const {
    return {roaringIndexOffset, roaringIndexOffset};
}

void Header::setKeyIndexOffset(uint64_t offset, uint64_t size) {
    Header::keyIndexOffset = offset;
    Header::keyIndexSize = size;
}

void Header::setCellIndexOffset(uint64_t offset, uint64_t size) {
    Header::cellIndexOffset = offset;
    Header::cellIndexSize = size;
}

void Header::setBitmapOffset(uint64_t offset, uint64_t size) {
    Header::roaringIndexOffset = offset;
    Header::roaringIndexSize = size;
}

uint32_t Header::getKeyIndexEntries() const {
    return keyIndexEntries;
}

void Header::setKeyIndexEntries(uint32_t size) {
    Header::keyIndexEntries = size;
}

uint32_t Header::getCellIndexEntries() const {
    return cellIndexEntries;
}

void Header::setCellIndexEntries(uint32_t size) {
    Header::cellIndexEntries = size;
}

uint8_t Header::getLevelIndexBucketRange() const {
    return levelIndexBucketRange;
}

uint16_t Header::getBlockSize() const {
    return blockSize;
}

uint8_t Header::getFileType() const {
    return fileType;
}

