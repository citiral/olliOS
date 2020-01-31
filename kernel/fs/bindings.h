#ifndef __BINDINGS_H
#define __BINDINGS_H

#include "kstd/string.h"
#include "threading/mutex.h"
#include "threading/spinlock.h"

namespace bindings {

    // TODO: this is massively inefficient. Bindings should be made threadsafe in a non-braindead way so this can be removed
    // Besides I have not yet verified if, even with this lock, insertions into threading linked lists are thread-safe after all
    extern threading::Mutex lock;

    class Binding;
    class OwnedBinding;

    typedef bool(*on_create_cb)(Binding* binding, Binding* child);
    typedef bool(*on_data_cb)(Binding* binding, size_t size, const void* data);
    typedef bool(*on_write_cb)(OwnedBinding* binding, size_t size, const void* data);
    
    typedef bool(*read_cb)(Binding* binding, size_t size, const void* data, bool done);
    typedef size_t(*on_read_cb)(OwnedBinding* binding, void* buffer, size_t size, size_t offset);

    class Binding_on_create {
    public:
        Binding_on_create* next;
        on_create_cb cb;
    };

    class Binding_on_data {
    public:
        Binding_on_data* next;
        on_data_cb cb;
    };

    class Binding_on_write {
    public:
        Binding_on_write* next;
        on_write_cb cb;
    };

    class Binding {
    public:
        Binding(std::string name);

        Binding* get(const char* name);
        bool has(const char* name);

        
        Binding* enumerate(on_create_cb cb, bool persistent = false);
        Binding* on_create(on_create_cb cb);
        Binding* on_data(on_data_cb cb);
        void write(const void* data, size_t size);
        size_t read(void* buffer, size_t size, size_t offset);

        template<class T>
        T read() {            
            T t;

            size_t total = 0;
            size_t ret;

            do {
                ret = read(((u8*)&t) + total, sizeof(T) - total, total);
                total += ret;
            } while (ret > 0);

            return t;
        }
        
        template<class T>
        T* add(T* child)
        {
            lock.lock();
            child->_parent = this;
            child->_next_sibling = _first_child;
            _first_child = child;
            lock.release();

            iterate<Binding_on_create>(&_on_create_cbs, this, child);
            return child;
        }

        template<class T, class... PARAMS>
        void iterate(T** node, PARAMS... params)
        {
            // Loop over all entries
            lock.lock();
            while (*node != NULL) {

                //Execute the callback. If it returns false, remove it from the list
                lock.release();
                bool keep_cb = (*node)->cb(params...);
                lock.lock();
                
                if (!keep_cb) {
                    T* next = ( *node)->next;
                    delete (*node);
                    *node = next;
                // Otherwise, just continue to the next entry
                } else {
                    node = &((*node)->next);
                }
            }
            lock.release();
        }

    public:
        u64 id;
        std::string name;
        Binding* _first_child;
        Binding* _next_sibling;
        Binding* _parent;
    protected:
        Binding_on_create* _on_create_cbs;
        Binding_on_data* _on_data_cbs;
        Binding_on_write* _on_write_cbs;
        on_read_cb _on_read_cb;
    };

    class OwnedBinding: public Binding {
    public:
        OwnedBinding(std::string name);

        OwnedBinding* on_write(on_write_cb cb);
        OwnedBinding* on_read(on_read_cb cb);
        void provide(const void* data, size_t size);
    };

    class MemoryBinding: public OwnedBinding {
    public:
        MemoryBinding(std::string name, const void* data, size_t size);        

    private:
        void* buffer;
        size_t size;
    };

    class RefMemoryBinding: public OwnedBinding {
    public:
        RefMemoryBinding(std::string name, const void* data, size_t size);        

    private:
        const void* buffer;
        size_t size;
    };

    void init();

    extern Binding* root;
    // Global bindings lock
}

#endif