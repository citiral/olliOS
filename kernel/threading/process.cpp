#include "threading/process.h"
#include "memory/virtual.h"

namespace threading {
    Spinlock processesLock;
    std::unordered_map<u32, Process*>* processes;

    Process::Process(): pageDirectory() {

    }

    Process::~Process() {
    }

    Process* currentProcess() {
        Thread* t = threading::currentThread();

        if (t == nullptr)
            return nullptr;

        return t->parent;
    }

    void registerProcess(Process* process) {
        if (processes == nullptr)
            processes = new std::unordered_map<u32, Process*>();
        
        CLI();
        processesLock.lock();
        (*processes)[process->pid] = process;
        processesLock.release();
        STI();
    }

    void unregisterProcess(Process* process) {
        if (processes == nullptr)
            processes = new std::unordered_map<u32, Process*>();

        CLI();
        processesLock.lock();
        processes->erase(process->pid);
        processesLock.release();
        STI();
    }

    Process* getProcess(u32 process) {        
        CLI();
        processesLock.lock();
        Process* p = (*processes)[process];
        processesLock.release();
        STI();

        return p;
    }

    void waitForProcess(u32 process) {
        while (getProcess(process) != nullptr)
            threading::exit();
    }
}