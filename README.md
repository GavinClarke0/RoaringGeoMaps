# RoaringGeoMaps

---


RoaringGeoMaps is a high performance and compact single-file spatial index designed to index any geospatial data that can be 
represented by a sequence of bytes and a [S2 region cover](http://s2geometry.io/devguide/examples/coverings.html). Users 
first construct the index by inserting bytes, and it's s2 region cover geospatial representation into the index. Once the 
index is constructed, users are able to preform `Contains`, `Intersects`, and `K Nearest Neighbors` queries to find the 
sets of byte sequences which geospatial representation matches the query. As roaring bitmap places no constraints on the 
byte sequences it indexes, users can use it to store anything they like, for example the keys to data stored in a different 
data store or even the geospatial data itself.

RoaringGeoMaps builds on the great work or idea's of the following projects: 
1. https://roaringbitmap.org/ (Compressed Integer Sets)
2. http://s2geometry.io/ (Integer based geospatial indexing )
3. https://github.com/ClickHouse/ClickHouse (Column Query Patterns)


## How the Index Works

### Index Structure

1. **Core Bitmaps**:  
   The index relies on two primary bitmaps to determine if a set of query `cellIds` match any `cellIds` or data stored in the file.
    - If a match is found, additional sections of the index are queried to map these `cellIds` to their corresponding keys/data.

2. **Bitmap Details**:
    - **Cover Bitmap**:  
      Stores the region cover of each key indexed, represented at parent cell levels that are multiples of a configurable modulo.
        - Example: If the modulo is `3`, a `cellId` of `8` would also be indexed at its parent levels `6` and `3`.
        - Purpose: Tracks what `cellIds` cover data in the index.
    - **Intersection Bitmap**:  
      Stores the region cover of each `cellId` at the next greatest cell level that is a multiple of the modulo.
        - Example: If the region cover contains a `cellId` of `5`, it would be indexed at level `6`.
        - Purpose: Determines if query `cellIds` intersect with any indexed region.

    

### Querying the Index

1. **Normalize Query CellIds**:  
   The set of query `cellIds` is first normalized to the index's configured modulo.

2. **Perform Bitmap Intersections**:
    - **Step 1**:  
      An intersection is performed between the **Cover Bitmap** and the normalized query `cellIds`.
        - Since the Cover Bitmap indexes all parent `cellIds` that match or cover the original indexed `cellIds`, any covering `cellIds` present in the query will be found in this intersection.
    - **Step 2**:  
      For each query `cellId`, its parent `cellIds` are located at each indexed level (according to the modulo). These are then compared to the **Intersection Bitmap**.
        - This operation determines if any query `cellIds` intersect with indexed cells at greater levels.

3. **Map CellIds to Keys**:
    - For each intersecting `cellId` identified above, a binary search is performed to map `cellIds` to their corresponding `key_ids` (represented as `uint32`).
    - A `key_id` is the index of the byte sequence as it is stored in the byte sequence or `key` column. 
    - Using Roaring Bitmaps, this mapping is compactly stored and efficiently queried.

4. **Retrieve and Return Keys**:
    - For each `key_id`, retrieve its associated key/byte sequence by retrieving the data at index `key_id`. 
    - Return the set of unique keys or data matching the query.

Following these steps for each cell that maps to a key in the index we preform `log(keys #) + log(cell ids) + 2 * roaring bitset inclusion search` operations.

## File Format Specification 

*Note the following specification is a work in progress and is no way fixed. It is subject to change at any time.*

Some notes/thesis on the design: 
1. All int values are little Endian
2. All values in the index will have the same in memory representation as on disk, which will allow you to mmap the file.
   This will be similar in implementation to flatbuffers and allows us to skip the latency of a deserialize step. This
   is made possible through the use of frozen roaring bitmaps. See https://github.com/RoaringBitmap/CRoaring/blob/master/cpp/roaring.hh#L709
3. Optional block level compression for the keyId and Cell maps sections may be added to allow the indexes to be efficient 
   in transport and usable when memory < index size. 

#### File Format 

```
<start of file>
    [Header] # contains offset and other file meta data
    [S2 cell hierarchical cover roaring bitmap] # Allows us to quickly deduce if a key matches the provided query without searching the entire s2. We can then use s2 cells present in index to quickly search present values keys. 
    [S2 cell intersection roaring bitmap ]
    [Key/Byte Sequence Column]
    [CellID Column] # Aligned with Key_id column. I.e they have the same number of values and CellIds at index x contain keys_ids stored at index x in the Bitmap Key_Id column. 
    [Bitmap Key_Id Column]
<end of file>
```

#### Header File Format

```
<start of header>
    [uint64 header size N bytes]
    [uint64 s2 cell hierarchical cover roaring bitmap offset N bytes]
    [uint64 s2 cell hierarchical cover roaring bitmap size N bytes]

    [uint64 s2 cell intersection roaring bitmap offset N bytes]
    [uint64 s2 cell intersection roaring bitmap size N bytes]
    [uint64 key/byte sequence column offset N bytes]
    [uint64 key/byte sequence column size N bytes]
    [uint64 cellId column offset N bytes]
    [uint64 cellId column size N bytes]
    [uint64 bitmap key_id column offset N bytes]
    [uint64 bitmap key_id column size N bytes]
    
    [uint32 key/byte sequence column entries]
    [uint32 cellId column entries offset] # since bitmap key_id column has the same size this value is used for both columns
    
    [uint32 int32 key_id to key sorted map offset N bytes] # sorted key to value map of tiny bit id's that represent each key. We store the relation between s2 cells and these int keys to save space. 
    [uint64 cell to key_id mapping offset N bytes]
    [uint8 s2 cell intersection/cover roaring bit map modulo] 
    [uint16 max # of entries in blocks in each column type] 
    [28 bytes for furture use] # Reserved bytes for future use, such as indicating block level compression schema
<end of header>
```

#### Key/Byte Sequence Column

The key/byte sequence column stores all key/byte_sequences present in the RoaringGeoMap indexed by their absolute position
in the column. Data is stored into blocks of a configurable size, 

```
<start Key/Byte Sequence Column section>

    <start Block Offsets> # blocks can be compressed so we have to store the offset of the block in the column. 
        [Block 1 offset uint64] 
        ...
        [Block N offset uint64]
    <end Block Offsets>

    <start Key/Byte Sequence block> # blocks can be compressed, below is uncompressed representation
        [1 offset key/bytes uint64] 
        ...
        [N offset uint64]
        [1 key bytes] # size is determined by the offset at index
        ...
        [N key bytes] 
    <end Key/Byte Sequence block>
     ... repeat blocks as needed
     [N Key/Byte Sequence block]
<end Key/Byte Sequence Column>
```

#### CellId Column

The CellId column stores an ordered list of all S2CellIds indexed in the index. It is aligned with the BitMap Key_Id/Byte 
Sequence Index Column. This means that CellId at position x maps to the bitmap which stores all Byte Sequence Ids in position
x in the BitMap Key_Id Index Column. 

```
<start CellId Column>
    
    <start Block Index> # Used to determine which blocks contains the values we are searching for. 
        [Block 1 Max Value uint64] 
        ...
        [Block N Max Valye uint64]
    <end Block Index>
    <start Block Offsets> # blocks can be compressed so we have to store the offset of the block in the column. 
        [Block 1 offset uint64] 
        ...
        [Block N offset uint64]
    <end Block Offsets>
    <start CellId block> # blocks can be compressed, below is uncompressed representation
        [1 CellId uint64] # size is determined by the offset at index
        ...
        [N CellId uint64] 
    <end CellId block>
     ... repeat blocks as needed
     [N CellId block]
<end CellId Column>
```

#### BitMap Key_Id/Byte Sequence Index Column

Stores the indexes of Key/Byte Sequence Column present in the cellId at the same index as the value

```
<start BitMap Key_Id/Byte Sequence Column>

    <start Block Offsets> # blocks can be compressed so we have to store the offset of the block in the column. 
        [Block 1 offset uint64] 
        ...
        [Block N offset uint64]
    <end Block Offsets>

    <start Bitmap Block> # blocks can be compressed, below is uncompressed representation
        [1 offset key/bytes uint64] 
        ...
        [N offset uint64]
        [1 Bitmap bytes] # size is determined by the offset at index
        ...
        [N Bitmap bytes] 
    <end  Bitmap  block>
     ... repeat blocks as needed
     [N  Bitmap  block]
<end BitMap Key_Id/Byte Sequence Column>
```

## Public C++ API 

TODO: 




