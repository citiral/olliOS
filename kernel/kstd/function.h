#ifndef OLLIOS_GIT_FUNCTION_H
#define OLLIOS_GIT_FUNCTION_H

namespace std {
    template <typename T>
    class function;

    template <typename R, typename... Args>
    class function<R(Args...)>
    {
        // function pointer types for the type-erasure behaviors
        // all these char* parameters are actually casted from some functor type
        typedef R (*invoke_fn_t)(char*, Args&&...);
        typedef void (*construct_fn_t)(char*, char*);
        typedef void (*destroy_fn_t)(char*);

        // type-aware generic functions for invoking
        // the specialization of these functions won't be capable with
        //   the above function pointer types, so we need some cast
        template <typename Functor>
        static R invoke_fn(Functor* fn, Args&&... args)
        {
            return (*fn)(std::forward<Args>(args)...);
        }

        template <typename Functor>
        static void construct_fn(Functor* construct_dst, Functor* construct_src)
        {
            // the functor type must be copy-constructible
            new (construct_dst) Functor(*construct_src);
        }

        template <typename Functor>
        static void destroy_fn(Functor* f)
        {
            f->~Functor();
        }

        // these pointers are storing behaviors
        invoke_fn_t invoke_f;
        construct_fn_t construct_f;
        destroy_fn_t destroy_f;

        // erase the type of any functor and store it into a char*
        // so the storage size should be obtained as well
        char* data_ptr;
        size_t data_size;
    public:
        function()
            : invoke_f(nullptr)
            , construct_f(nullptr)
            , destroy_f(nullptr)
            , data_ptr(nullptr)
            , data_size(0)
        {}

        // construct from any functor type
        template <typename Functor>
        function(Functor f)
            // specialize functions and erase their type info by casting
            : invoke_f(reinterpret_cast<invoke_fn_t>(invoke_fn<Functor>))
            , construct_f(reinterpret_cast<construct_fn_t>(construct_fn<Functor>))
            , destroy_f(reinterpret_cast<destroy_fn_t>(destroy_fn<Functor>))
            , data_ptr(new char[sizeof(Functor)])
            , data_size(sizeof(Functor))
        {
            // copy the functor to internal storage
            this->construct_f(this->data_ptr.get(), reinterpret_cast<char*>(&f));
        }

        // copy constructor
        function(function const& rhs)
            : invoke_f(rhs.invoke_f)
            , construct_f(rhs.construct_f)
            , destroy_f(rhs.destroy_f)
            , data_size(rhs.data_size)
        {
            if (this->invoke_f) {
                // when the source is not a null function, copy its internal functor
                this->data_ptr.reset(new char[this->data_size]);
                this->construct_f(this->data_ptr.get(), rhs.data_ptr.get());
            }
        }

        ~function()
        {
            if (data_ptr != nullptr) {
                this->destroy_f(this->data_ptr.get());
            }
        }

        // other constructors, from nullptr, from function pointers

        R operator()(Args&&... args)
        {
            return this->invoke_f(this->data_ptr.get(), std::forward<Args>(args)...);
        }
    };
}

#endif