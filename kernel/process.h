#ifndef __PROCESS_H_
#define __PROCESS_H_

#include "kstd/unordered_map.h"
#include "memory/virtual.h"
#include "util/unique.h"
#include "file.h"
#include "kstd/string.h"
#include "kstd/shared_ptr.h"
#include "kstd/vector.h"
#include "threading/waiting_list.h"
#include "resource_map.h"
#include <sys/dirent.h>

struct FileDescriptor {
    fs::FileHandle* handle;
    size_t offset;

    FileDescriptor(fs::FileHandle* h): handle(h), offset(0) { }

    ~FileDescriptor() {
        handle->close();
    }
};

enum class ProcessState {
    Initing,
    Running,
    Forking,
    Stopped,
    Waiting,
    Execve,
    PendingDestruction,
};

namespace threading {
    class Thread;
}

class Process {
public:
    Process();
    ~Process();

    void set_arguments(std::vector<std::string>& args);
    void init(fs::File* file, std::string& workingDirectory);
    void start();
    void wait();
    
    memory::PageDirectory* pagetable();
    void set_program_break(char* program_break);

    void set_state(ProcessState new_state);

    void finish_fork(memory::PageDirectory* clone);
    void finish_execve();
    
    i32 open(fs::File* file, i32 flags, i32 mode);

    // syscall routines
    i32 open(const char* name, i32 flags, i32 mode);
    i32 close(i32 file);
    i32 write(i32 file, const char* data, i32 len);
    i32 read(i32 file, char* data, i32 len);
    i32 exit(i32 status);
    i32 fork();
    i32 execve(const char* pathname, char *const *argv, char *const *envp);
    i32 wait(i32* status);
    i32 isatty(i32 file);
    i32 lseek(i32 file, i32 ptr, i32 dir);
    i32 fstat(i32 file, struct stat* st);
    void* sbrk(i32 inc);
    i32 pipe(i32 pipefd[2]);
    i32 dup(int filedes);
    i32 dup2(int filedes, int filedes2);
    i32 readdir(i32 filedes, struct dirent* dirent);
    char* getwd(char* buf, size_t size);

    i32 status_code;
    volatile ProcessState state;
    threading::Thread* thread;
    std::vector<Process*> childs;
    u32 pid;

private:
    void free_pagetable();

    memory::PageDirectory* _pagetable;
    ResourceMap<std::shared_ptr<FileDescriptor>> _bindings;
    std::vector<std::string> _args;
    fs::File* _file;
    char* _program_break;
    fs::File* _descriptor;
    threading::WaitingList _waitingForStopped;
    threading::WaitingList _waitingForChildStopped;
    threading::Spinlock _stateLock;
    Process* _parent = nullptr;
    std::string _workingDirectory;
};

#endif
