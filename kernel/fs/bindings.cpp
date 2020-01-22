#include "fs/bindings.h"
#include "util/unique.h"

namespace bindings
{
    Binding* root = NULL;
    static UniqueGenerator<u64> id_generator(2);

    void init()
    {
        root = new OwnedBinding(1, "");
    }

    Binding::Binding(u64 id, std::string name): id(id), name(name), _first_child(NULL), _next_sibling(NULL), _on_create_cbs(NULL), _on_data_cbs(NULL), _on_write_cbs(NULL)
    {
    }

    OwnedBinding::OwnedBinding(u64 id, std::string name): Binding(id, name)
    {
    }

    Binding* Binding::get(const char* name)
    {
        Binding* child = _first_child;

        while (child != NULL) {
            if (child->name == name) {
                return child;
            }
            child = child->_next_sibling;
        }

        return NULL;
    }

    bool Binding::has(const char* name)
    {
        return get(name) != NULL;
    }

    OwnedBinding* Binding::create(std::string child_name, on_write_cb cb)
    {
        // Create a new child, and put him as the first sibling with the previous first sibling as his next one
        OwnedBinding* child = new OwnedBinding(id_generator.next(), child_name);
        child->_next_sibling = _first_child;
        _first_child = child;

        if (cb != NULL) {
            child->on_write(cb);
        }

        iterate<Binding_on_create>(&_on_create_cbs, this, child);
        return child;
    }

    void Binding::write(u32 size, const void* data)
    {
        iterate<Binding_on_write>(&_on_write_cbs, (OwnedBinding*) this, size, data);
    }

    void OwnedBinding::provide(u32 size, const void* data)
    {
        iterate<Binding_on_data>(&_on_data_cbs, this, size, data);
    }

    Binding* Binding::on_create(on_create_cb cb)
    {
        Binding_on_create* binding_on_create = new Binding_on_create();
        binding_on_create->cb = cb;
        binding_on_create->next = _on_create_cbs;
        _on_create_cbs = binding_on_create;

        return this;
    }

    Binding* Binding::on_data(on_data_cb cb)
    {
        Binding_on_data* binding_on_data = new Binding_on_data();
        binding_on_data->cb = cb;
        binding_on_data->next = _on_data_cbs;
        _on_data_cbs = binding_on_data;

        return this;
    }

    OwnedBinding* OwnedBinding::on_write(on_write_cb cb)
    {
        Binding_on_write* binding_on_write = new Binding_on_write();
        binding_on_write->cb = cb;
        binding_on_write->next = _on_write_cbs;
        _on_write_cbs = binding_on_write;

        return this;
    }
}