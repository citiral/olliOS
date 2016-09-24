//
// Created by citiral on 9/23/16.
//

#include "BucketAlloc.h"
#include "cdefs.h"
#include <string.h>
#include <stdio.h>

// if the first bit of a region is 1 it means that region is currently in use
#define USED_FLAG 0x80000000

BucketAlloc::BucketAlloc() {

}

void BucketAlloc::init(void* start, size_t length) {
    // these two values are used for the merging step
    this->start = start;
    end = start + length;

    // clear the buckets
    for (int i = 0 ; i < 32 ; i++) {
        buckets[i] = 0;
    }
    // throw the memory in the corresponding bucket
    int bucket = nextLowestPowerOfTwo(length - sizeof(size_t) * 2);
    buckets[bucket] = start;

    // and mark the header of the memory to indicate the length, the next pointer (which is 0)
    *(size_t*)start = (length - sizeof(size_t) * 2);
    *((size_t*)start + 1) = 0;
}

void* BucketAlloc::malloc(size_t size) {
    // try to allocate the memory
    void* mem = mallocOneTry(size);

    // if it is null, lets do a merge step and try again
    if (mem == nullptr) {
        merge();
        mem = mallocOneTry(size);
    }

    // and return the allocated memory, this might be null if nothing was found
    return mem;
}

void* BucketAlloc::mallocOneTry(size_t size) {
    // find the nearest bucket for the amount of memory that needs to be allocated
    int bucket = nextHighestPowerOfTwo(size);

    // loop over buckets until we find one with contents
    for (int i = bucket ; i < 32 ; i++) {
        // success, this bucket can be used
        if (buckets[i] != 0) {
            // get some information about the memory region that we are going to use
            size_t* oldregpos = (size_t*)buckets[i];

            // only if we don't fully use the chosen region and if there is room for another header (2 ints)
            if (size + 2 * sizeof(size_t) < oldregpos[0]) {
                // shrink the region to make place for the new mem
                size_t* newregpos = (size_t*)(buckets[i] + size + sizeof(size_t) * 2);
                newregpos[0] = oldregpos[0] - size - sizeof(size_t) * 2;

                // remove the old region from the bucket by replacing it with his next ptr
                buckets[i] = (void*)oldregpos[1];

                // reinsert the new region since it might have gone down a bucket
                insertIntoBucket(newregpos);
            } else {
                // if the remaining is not big enough, lets also allocate the last remaining bytes
                size = oldregpos[0];

                // we shouldn't forget to throw this memory out of his bucket
                buckets[i] = (void*)oldregpos[1];
            }

            // and reuse the old region for ourselves
            oldregpos[0] = size | USED_FLAG; // set the used flag so merge can now this region is active
            oldregpos[1] = 0; // the next pointer is used to check if the region is used so we have to clear it

            return oldregpos + 2;
        }
    }
    // no bucket has been found so lets return 0
    return nullptr;
}

void BucketAlloc::insertIntoBucket(size_t* region) {
    // get the bucket of the region
    int bucket = nextLowestPowerOfTwo(region[0]);

    // insert it into the linked list
    region[1] = (size_t)buckets[bucket];
    buckets[bucket] = region;
}

void* BucketAlloc::realloc(void* ptr, size_t size) {
    // just allocate new memory
    void* newmem = malloc(size);

    // copy the old memory
    size_t oldsize = *(((size_t*)ptr) - 2);
    if (oldsize < size) {
        memcpy(newmem, ptr, oldsize);
    } else {
        memcpy(newmem, ptr, size);
    }

    // free the old memory
    free(ptr);

    // and we can safely return the new mem :)
    return newmem;
}

void BucketAlloc::free(void* ptr) {
    // get a pointer to the memory region (just memory + header)
    size_t* reg = (size_t*)(ptr - 2 * sizeof(size_t));

    // remove the used flag
    reg[0] = reg[0] ^ USED_FLAG;

    // and insert it into a bucket
    insertIntoBucket(reg);
}

void* BucketAlloc::calloc(size_t num, size_t size) {
    return malloc(num * size);
}

void BucketAlloc::merge() {
    // first we clear all buckets since they will be rebuild after the merge
    for (int i = 0 ; i < 32 ; i++)
        buckets[i] = 0;

    // keep going untill we passed all memory
    void* current = start;
    while (current < end) {
        // get the current region
        size_t* regCur = (size_t*)current;

        // if this region is in use, continue
        if (regCur[0] >= USED_FLAG) {
            current += (regCur[0] ^ USED_FLAG) + 2 * sizeof(size_t); // advance current to the next region
            continue;
        }

        // if this is the last region, abort
        if (current + regCur[0] + 2 * sizeof(size_t) >= end)
            break;

        // otherwise, get the next region
        size_t* regNext = (size_t*)(current + regCur[0] + 2 * sizeof(size_t));

        // if that region is in use, continue
        if (regNext[0] >= USED_FLAG) {
            current += (regCur[0] ^ USED_FLAG) + 2 * sizeof(size_t); // advance current to the next region
            continue;
        }

        // all checks passed so we can merge the memory
        // the new size is both our sizes + the size of the reclaimed header
        size_t newsize = regCur[0] + regNext[0] + 2 * sizeof(size_t);

        // and the merge happens simply by increasing our size
        regCur[0] = newsize;
    }

    // now that memory is merged we can reconstruct the buckets
    current = start;
    while (current < end) {
        // get the current region
        size_t* regCur = (size_t*)current;

        // if this region is not in use, insert it into his bucket
        if (regCur[0] < USED_FLAG) {
            insertIntoBucket(regCur);
        }

        printf("current = %X, size = %X\n", current, regCur[0]);
        //BOCHS_BREAKPOINT
        // and advance current to the next region, taking care of the used tag
        current += (regCur[0] & (~USED_FLAG)) + 2 * sizeof(size_t);
    }
}

size_t BucketAlloc::nextHighestPowerOfTwo(size_t v) {
    // gcc intrinsics bitch.
    // __builtin_clz returns the amount of leading zeros in a number
    // __builtin_popcount returns the amount of ones in a number
    return 31 - __builtin_clz(v) + (__builtin_popcount(v) > 1);
}

size_t BucketAlloc::nextHighestPowerOfTwo(void* value) {
    return nextHighestPowerOfTwo((size_t)value);
}

size_t BucketAlloc::nextLowestPowerOfTwo(size_t v) {
    // gcc intrinsics bitch. __builtin_clz returns the amount of leading zeros in a number
    return 31 - __builtin_clz(v);
}

size_t BucketAlloc::nextLowestPowerOfTwo(void* value) {
    return nextLowestPowerOfTwo((size_t)value);
}

void BucketAlloc::printStatistics() {
    printf("buckets:\n");
    for (int i = 0 ; i < 32/4 ; i++) {
        printf("%d: %X, %d: %X, %d: %X, %d: %X\n", i*4, buckets[i*4], i*4 + 1, buckets[i*4 + 1], i*4 + 2, buckets[i*4 + 2], i*4 + 3, buckets[i*4 + 3]);
    }
}