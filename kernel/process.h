#ifndef __PROCESS_H_
#define __PROCESS_H_

#include "kstd/unordered_map.h"
#include "threading/thread.h"
#include "memory/virtual.h"
#include "util/unique.h"
#include "bindings.h"

struct BindingDescriptor {
    bindings::Binding* binding;
    size_t offset;
};

enum class ProcessState {
    Initing,
    Running,
    Forking,
    Stopped,
};

class Process {
public:
    Process();
    ~Process();

    void init(bindings::Binding* file);
    void start();
    void wait();
    
    memory::PageDirectory* pagetable();

    void finish_fork(memory::PageDirectory* clone);
    
    // syscall routines
    i32 open(const char* name, i32 flags, i32 mode);
    i32 close(i32 file);
    i32 write(i32 file, char* data, i32 len);
    i32 read(i32 file, char* data, i32 len);
    i32 exit(i32 status);
    i32 fork();

    UniqueGenerator<i32> _binding_ids;
    i32 status_code;
    ProcessState state;
    threading::Thread* thread;
    std::vector<Process*> childs;

private:
    void free_pagetable();

    memory::PageDirectory* _pagetable;
    std::unordered_map<i32, BindingDescriptor> _bindings;
    u32 pid;
    Process* _parent;
};

#endif
