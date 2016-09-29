//
// Created by citiral on 9/23/16.
//

#ifndef OLLIOS_GIT_BUCKETALLOC_H
#define OLLIOS_GIT_BUCKETALLOC_H

#include <stddef.h>

class BucketAlloc {
public:
    BucketAlloc();

    void init(void* start, size_t length);

    void* malloc(size_t size);
    void* realloc(void* ptr, size_t size);
    void free(void* ptr);
    void* calloc(size_t num, size_t size);

    void merge();
    void printStatistics();

private:
    void mergeOneArea(void* start, void* end);
    void* mallocOneTry(size_t size);
    void insertIntoBucket(size_t* region);

    size_t nextHighestPowerOfTwo(size_t value);
    size_t nextHighestPowerOfTwo(void* value);
    size_t nextLowestPowerOfTwo(size_t value);
    size_t nextLowestPowerOfTwo(void* value);

public:
    void* buckets[32];
    void* memoryLinkedList;
};

#endif //OLLIOS_GIT_BUCKETALLOC_H
