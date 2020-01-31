#include "fs/bindings.h"
#include "util/unique.h"
#include "string.h"

namespace bindings
{
    Binding* root = NULL;
    threading::Mutex lock;

    static UniqueGenerator<u64> id_generator(1);

    void init()
    {
        id_generator = UniqueGenerator<u64>(1);
        root = new OwnedBinding("");
    }

    Binding::Binding(std::string name): id(id_generator.next()), name(name), _first_child(NULL), _next_sibling(NULL), _parent(NULL), _on_create_cbs(NULL), _on_data_cbs(NULL), _on_write_cbs(NULL), _on_read_cb(NULL)
    {
    }

    OwnedBinding::OwnedBinding(std::string name): Binding(name)
    {
    }

    Binding* Binding::get(const char* name)
    {
        const char* subname = name;

        // Skip leading slash
        if (subname[0] == '/') {
            subname++;

            // True if the path ends with a '/', in which case we are the targetted binding
            if (subname[0] == 0) {
                return this;
            }
        }

        // Handle '.' and '..'
        if (subname[0] == '.') {
            if (subname[1] == '/') {
                return this->get(subname+2);
            } else if (subname[1] == 0) {                
                return this;
            } else if (subname[1] == '.') {
                if (subname[2] == '/') {
                    if (_parent) {
                        return _parent->get(subname+3);
                    } else {
                        return NULL;
                    }
                } else if (subname[2] == 0) {                
                    return _parent;
                }
            }
        }

        lock.lock();
        Binding* child = _first_child;

        while (child != NULL) {

            int i = 0;
            do {
                // End of section in path has been reached
                if (child->name[i] == 0) {
                    if (subname[i] == 0) {
                        lock.release();
                        return child;
                    } else if (subname[i] == '/') {
                        lock.release();
                        return child->get(subname+i);
                    } else {
                        break;
                    }
                }
            } while (subname[i] == child->name[i++]);

            child = child->_next_sibling;
        }
        lock.release();

        return NULL;
    }

    bool Binding::has(const char* name)
    {
        return get(name) != NULL;
    }

    void Binding::write(const void* data, size_t size)
    {
        iterate<Binding_on_write>(&_on_write_cbs, (OwnedBinding*) this, size, data);
    }
    
    size_t Binding::read(void* buffer, size_t size, size_t offset)
    {
        if (_on_read_cb)
            return _on_read_cb((OwnedBinding*)this, buffer, size, offset);
        else
            return 0;
    }

    void OwnedBinding::provide(const void* data, size_t size)
    {
        iterate<Binding_on_data>(&_on_data_cbs, this, size, data);
    }
    Binding* Binding::enumerate(on_create_cb cb, bool persistent)
    {
        Binding* child = _first_child;
        if (persistent) {
            on_create(cb);
        }

        while (child != NULL) {
            if (!cb(this, child)) {
                break;
            }

            child = child->_next_sibling;
        }

        return this;
    }

    Binding* Binding::on_create(on_create_cb cb)
    {
        Binding_on_create* binding_on_create = new Binding_on_create();
        binding_on_create->cb = cb;
        lock.lock();
        binding_on_create->next = _on_create_cbs;
        _on_create_cbs = binding_on_create;
        lock.release();

        return this;
    }

    Binding* Binding::on_data(on_data_cb cb)
    {
        lock.lock();
        Binding_on_data* binding_on_data = new Binding_on_data();
        binding_on_data->cb = cb;
        binding_on_data->next = NULL;// _on_data_cbs;
        _on_data_cbs = binding_on_data;
        lock.release();

        return this;
    }

    OwnedBinding* OwnedBinding::on_write(on_write_cb cb)
    {
        Binding_on_write* binding_on_write = new Binding_on_write();
        binding_on_write->cb = cb;
        lock.lock();
        binding_on_write->next = _on_write_cbs;
        _on_write_cbs = binding_on_write;
        lock.release();

        return this;
    }
    
    OwnedBinding* OwnedBinding::on_read(on_read_cb cb)
    {
        _on_read_cb = cb;
        return this;
    }

    MemoryBinding::MemoryBinding(std::string name, const void* data, size_t size): OwnedBinding(name)
    {
        // Copy over the data
        buffer = new char[size];
        this->size = size;
        memcpy(buffer, data, size);

        // Register a callback to read the data
        on_read([](OwnedBinding* binding, void* buffer, size_t size, size_t offset) -> size_t {
            MemoryBinding* t = (MemoryBinding*) binding;

            if (t->size < size + offset) {
                size = t->size - offset;
            }

            memcpy(buffer, t->buffer, size);

            return size;
        });
    }

    RefMemoryBinding::RefMemoryBinding(std::string name, const void* data, size_t size): OwnedBinding(name)
    {
        // Copy over the data
        buffer = data;
        this->size = size;

        // Register a callback to read the data
        on_read([](OwnedBinding* binding, void* buffer, size_t size, size_t offset) -> size_t {
            RefMemoryBinding* t = (RefMemoryBinding*) binding;

            if (t->size < size + offset) {
                size = t->size - offset;
            }

            memcpy(buffer, t->buffer, size);

            return size;
        });
    }
}