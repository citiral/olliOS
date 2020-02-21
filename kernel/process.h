#ifndef __PROCESS_H_
#define __PROCESS_H_

#include "kstd/unordered_map.h"
#include "memory/virtual.h"
#include "util/unique.h"
#include "bindings.h"
#include "kstd/shared_ptr.h"
#include "kstd/vector.h"

struct BindingDescriptor {
    bindings::Binding* binding;
    size_t offset;
};

enum class ProcessState {
    Initing,
    Running,
    Forking,
    Stopped,
    Execve,
};

namespace threading {
    class Thread;
}

class Process {
public:
    Process();
    ~Process();

    void init(bindings::Binding* file, std::vector<std::string> args);
    void start();
    void wait();
    
    memory::PageDirectory* pagetable();

    void finish_fork(memory::PageDirectory* clone);
    void finish_execve();
    
    // syscall routines
    i32 open(const char* name, i32 flags, i32 mode);
    i32 close(i32 file);
    i32 write(i32 file, char* data, i32 len);
    i32 read(i32 file, char* data, i32 len);
    i32 exit(i32 status);
    i32 fork();
    i32 execve(const char* pathname, char *const *argv, char *const *envp);

    UniqueGenerator<i32> _binding_ids;
    i32 status_code;
    ProcessState state;
    threading::Thread* thread;
    std::vector<Process*> childs;
    u32 pid;

private:
    void free_pagetable();

    memory::PageDirectory* _pagetable;
    std::unordered_map<i32, BindingDescriptor> _bindings;
    std::vector<std::string> _args;
    bindings::Binding* _file;
    //std::shared_ptr<Process> _parent;
};

#endif
