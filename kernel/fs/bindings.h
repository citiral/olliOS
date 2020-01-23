#ifndef __BINDINGS_H
#define __BINDINGS_H

#include "kstd/string.h"
#include "threading/mutex.h"

namespace bindings {

    class Binding;
    class OwnedBinding;

    typedef bool(*on_create_cb)(Binding* binding, Binding* child);
    typedef bool(*on_data_cb)(Binding* binding, size_t size, const void* data);
    typedef bool(*on_write_cb)(OwnedBinding* binding, size_t size, const void* data);

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
        Binding(u64 id, std::string name);

        Binding* get(const char* name);
        bool has(const char* name);

        OwnedBinding* create(std::string child_name, on_write_cb cb = NULL);
        Binding* on_create(on_create_cb cb);
        Binding* on_data(on_data_cb cb);
        void write(u32 size, const void* data);
        
        template<class T, class... PARAMS>
        void iterate(T** node, PARAMS... params)
        {
            // Loop over all entries
            _lock.lock();
            while (*node != NULL) {

                
                //Execute the callback. If it returns false, remove it from the list
                _lock.release();
                bool keep_cb = (*node)->cb(params...);
                _lock.lock();
                
                if (!keep_cb) {                    
                    T* next = ( *node)->next;
                    delete (*node);
                    *node = next;
                // Otherwise, just continue to the next entry
                } else {
                    node = &((*node)->next);
                }
            }
            _lock.release();
        }

    public:
        u64 id;
        std::string name;
        Binding* _first_child;
        Binding* _next_sibling;
    protected:
        Binding_on_create* _on_create_cbs;
        Binding_on_data* _on_data_cbs;
        Binding_on_write* _on_write_cbs;
        threading::Mutex _lock;
    };

    class OwnedBinding: public Binding {
    public:
        OwnedBinding(u64 id, std::string name);

        OwnedBinding* on_write(on_write_cb cb);
        void provide(u32 size, const void* data);
    };

    void init();

    extern Binding* root;
}

#endif