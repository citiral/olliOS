#ifndef __PROCESS_H
#define __PROCESS_H

#include "threading/thread.h"
#include "threading/scheduler.h"
#include "threading/spinlock.h"
#include "memory/virtual.h"
#include "kstd/unordered_map.h"
#include "util/unique.h"
#include "kstd/vector.h"
#include "kstd/string.h"

namespace threading {
    class Process {
    public:
        Process();
        ~Process();

        u32 pid;
        Process* parent;
        std::vector<Thread*> threads;
        memory::PageDirectory* pageDirectory;
    };

    extern Spinlock processesLock;
    extern std::unordered_map<u32, Process*>* processes;

    Process* currentProcess();
    void registerProcess(Process* process);
    void unregisterProcess(Process* process);
    void waitForProcess(u32 process);
    Process* getProcess(u32 process);
    template<class ... ARGS>
    Process* spawnProcess(void(*entry)(ARGS...), ARGS ... args);

    template<class ... ARGS>
    void ProcessFunctionEntrypoint(void(*entry)(ARGS...), ARGS ... args);    

    template<class ... ARGS>
    void ProcessFunctionEntrypoint(void(*entry)(ARGS...), ARGS ... args) {
        // call the entry function
        entry(args...);

        // Now we want to clean up the process, this contains multiple steps
        Process* p = currentProcess();
        Thread* t = currentThread();
        
        // first kill all threads running in the process
        for (int i = 0 ; i < p->threads.size() ; i++) {
            if (p->threads[i]->id() == t->id())
                continue;
            p->threads[i]->kill();
        }

        // finally we can unregister the process, and delete it        
        unregisterProcess(p);
        delete p;

        // destroy the pagetable. TODO make sure all physical memory backing this process is released
        memory::freePageDirectory(p->pageDirectory);
    }

    template<class ... ARGS>
    Process* spawnProcess(void(*entry)(ARGS...), ARGS ... args) {
        // allocate a new process
        Process* p = new Process();

        p->pid = pidGenerator.next();
        
        // make it inherit its parents environment
        Process* current = currentProcess();
        if (current != nullptr) {
            p->parent = current;
        }

        // initialize the process pagetable to a copy of the kernel
        p->pageDirectory = memory::kernelPageDirectory.clone();

        // And create a startup thread for it
        Thread* startup = new Thread(p, ProcessFunctionEntrypoint, entry, args...);

        // register the process
        registerProcess(p);

        // and schedule its main thread
        threading::scheduler->schedule(startup);

        return p;
    }
}


#endif // __PROCESS_H