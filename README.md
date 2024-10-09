# RoaringGeoMaps
---

A single file spatial index that maps between geospatial queries and arbitary sets of keys. These aribtary keys then can be used to look up
data assoicated with the result key set, allowing us to seperate the index from the storage medium of the data the index allows queries on.

RoaringGeoMaps builds on the great work or idea's of the following projects: 
1. https://roaringbitmap.org/
2. http://s2geometry.io/
3. https://github.com/ClickHouse/ClickHouse

## File Format Specification 

*Note the following specification is a work in progress and is no way fixed. It is subject to change at any time.*

All int values are little Endian 

```
<start of file>
   <start of header>
    <uint32 header size N bytes>
    <uint32 s2 cell inclusion roaring bitmap offset N bytes>
    <uint32 int32 key_id to key sorted map offset N bytes> # sorted key to value map of tiny bit id's that represent each key. We store the relation between s2 cells and these int keys to save space. 
    <uint64 cell to key_id mapping offset N bytes>
    <uint8 s2 cell inclusion roaring bit map modulo> # 
 See https://github.com/golang/geo/blob/6adc5660321723185f04b66d66a5563b29228236/s2/cellunion.go#L271 We stratify the region cover of all shapes in the index to a set of known levels to reduce the search space when checking if a value is present. We store the interval at which we stratify to in this value in the header. 
   <end of header of header>
   <roaring bitmap uint64 of all s2 cells present in index> # Allows us to quickly deduce if a key matches the provided query without searching the entire index. We can then use s2 cells present in index to quickly search present values keys. 
   <start key_id to key map>
      <key_id to key row start>
         <uint32 key_id>
         <uint16 key size N Bytes>
          <key bytes>
          .... repeated until we reach max row size. 
      <key_id to key row end>
      ... repeat for all key_id to key mappings. List is sorted allowing binary search. 
   <end key_id to key map>
   <start cell to key_id map>
      <s2 cell_id to key_id row start>
         <uint64 cell_id>
         <uint32 key_id 1 > # may use a compressed bitmap here as well if and only if cells often map to multiple keys
          .... repeated until we reach max row size.  cell_id
      <s2 cell_id to key_id rowend>
      ... repeat for all s2 cell to key_id mappings. If a cell maps to keys exceedng the row size, we have a duplicate rows with the same
   <end cell to key_id map>
<end of file>
```

## How it works 

Note, All queries that the index supports are expressible as a set of S2 cells. 

1. Identify the Relevant Parent S2 Cell: For each S2 cell in the query, locate the parent cell whose level corresponds to the nearest smaller multiple of the modulo used in the index. For instance, if the index's modulo is 5 and the query cell is at level 6, the parent cell at level 5 is selected.
2. Search Across Stratified Levels: Perform a search on the Roaring Bitmap at each stratified level (modulo) of the index that is less than the query cell's level. If the parent cell of the current query cell at any level is present in the bitmap, it indicates a potential match between the query cell and some key.
3. Store Matched Cell Identifiers: For each query cell, record the cell identifiers (cell-ids) that match.
4. Perform Binary Search on Mappings: For each matched cell-id, conduct a binary search on the cell to key_id row mappings. Since the rows have a fixed size, this search can be executed in 
𝑂(log 𝑛) time. Store the key_ids that correspond to each matched cell-id.
5. Retrieve and Return Keys: For each key_id, retrieve the associated key from the key_id -> key mapping. Return the set of unique keys matching the query.

Following these steps for each cell that maps to a key in the index we preform `log(keys #) + log(cell ids) + n * roaring bitset inclusion search` operations. 


## Public C++ API 

TODO: 




