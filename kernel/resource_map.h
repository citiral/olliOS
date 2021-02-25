#ifndef __RESOURCE_MAP_H_
#define __RESOURCE_MAP_H_

#include <kstd/shared_ptr.h>
#include <kstd/vector.h>

template <class T>
class ResourceMap {
public:
    ResourceMap(size_t max_count): _max_count(max_count)
    {
        allocate_resources();
    }

    ~ResourceMap()
    {
        free_all_resources();
        free(_data);
    }

    ResourceMap(const ResourceMap& map): _max_count(map.max_count)
    {
        _data = (Entry*) malloc(sizeof(Entry) * _max_count);
        for (size_t i = 0 ; i < _max_count ; i++) {
            _data[i].used = map._data[i].used;
            if (_data[i].used) {
                new (&_data[i].entry) T(map._data[i].entry);
            }
        }
    }

    ResourceMap<T>& operator=(const ResourceMap<T>& map)
    {
        if (&map != this) {
            free_all_resources();
            free(_data);

            _max_count = map._max_count;
            _data = (Entry*) malloc(sizeof(Entry) * _max_count);
            for (size_t i = 0 ; i < _max_count ; i++) {
                _data[i].used = map._data[i].used;
                if (_data[i].used) {
                    new (&_data[i].entry) T(map._data[i].entry);
                }
            }     
        }

        return *this;
    }

    int used_count()
    {
        int count = 0;
        for (size_t i = 0 ; i < _max_count ; i++) {
            if (_data[i].used) {
                count++;
            }
        }

        return count;
    }

    int free_count()
    {
        return _max_count - used_count();
    }

    T* at(i32 index)
    {
        if (index < 0) {
            return nullptr;

        }
        if (_data[index].used) {
            return &_data[index].entry;
        } else {
            return nullptr;
        }
    }

    i32 new_resource()
    {
        for (size_t i = 0 ; i < _max_count ; i++) {
            if (!_data[i].used) {
                _data[i].used = true;
                new (&_data[i].entry) T();
                return i;
            }
        }
        return -1;
    }

    T* new_resource(i32 index)
    {
        if (!_data[index].used) {
            _data[index].used = true;
            new (&_data[index].entry) T();
            return at(index);
        }
        return nullptr;
    }

    void free_resource(i32 index)
    {
        if (index < _max_count) {
            if (_data[index].used) {
                _data[index].used = false;
                _data[index].entry.~T();
            }
        }
    }

private:
    void allocate_resources()
    {
        _data = (Entry*) malloc(sizeof(Entry) * _max_count);

        for (size_t i = 0 ; i < _max_count ; i++) {
            _data[i].used = false;
        }
    }

    void free_all_resources()
    {
        for (size_t i = 0 ; i < _max_count ; i++) {
            if (_data[i].used) {
                _data[i].used = false;
                _data[i].entry.~T();
            }
        }
    }

    struct Entry {
        T entry;
        bool used;
    };

    size_t _max_count;
    Entry* _data;
};

#endif