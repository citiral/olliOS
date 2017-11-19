#include "types.h"
#include "stdio.h"
#include "string.h"
#include "kstd/string.h"
#include "kstd/new.h"
#include "stdlib.h"
#include "acpi.h"

#include "cpu.h"
#include "gdt.h"
#include "interrupt.h"
#include "sleep.h"

#include "eventbus/eventbus.h"
#include "eventbus/eventbustest.h"
#include "eventbus/nulllistener.h"

#include "pic.h"
#include "apic.h"
#include "io.h"
#include "alloc.h"
#include "linker.h"

#include "memory/virtual.h"
#include "memory/physical.h"

#include "devicemanager.h"
#include "devices/keyboard.h"
#include "devices/vga.h"
#include "devices/serial.h"
#include "devices/ata/ata.h"
#include "devices/pci/pci.h"
#include "devices/cmos.h"

#include "fs/iso9660.h"
#include "fs/virtualfilesystem.h"
#include "fs/filesystem.h"

#include "multiboot.h"
#include "kernelshell.h"
#include "cpuid.h"


void initCpu() {
    // setup a flat segmentation structure. We're going to be using paging anyway.
	GdtCreateFlat();
	GdtFlush();
    LOG_STARTUP("GDT initialized.");

    // Then we set up the interupt tables so we can actually use interrupts (these contain function pointers to interrupt handlers)
	IdtcreateEmpty();
	IdtFlush();
	IdtRegisterInterrupts();
    LOG_STARTUP("IDT initialized.");

    // I forgot what this does but it has to do with interrupts.. I believe
	initialize_tss(0x10, 0x28);
    LOG_STARTUP("TSS initialized.");

    // Program the PIC so interrupts make sense (0-32 is reserved for intel, ...)
	PicInit();
    LOG_STARTUP("PIC initialized.");
}

void initMemory(multiboot_info* multiboot) {
    // We initialise the state of the physical memory allocator
    physicalMemoryManager.init();

    // use the multiboot header to discover valid memory regions
    if ((multiboot->flags & (1 << 6)) == 0) {
        // if multiboot didn't pass us our usable memory areas (which it really always should) use a default.
        LOG_STARTUP("Err: No MMAP specified by multiboot.");
        kernelAllocator.init((void*)KERNEL_END_VIRTUAL, 0xFFFFFFFF - (size_t)KERNEL_END_VIRTUAL);

        // set up the physical memory allocator from the end of the kernel to the end of the address space
        physicalMemoryManager.registerAvailableMemory(KERNEL_END_PHYSICAL, 0xFFFFFFFF);
    } else {
        // loop over each mmap descriptor, these are of variable size so the loop is special
        for (multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)multiboot->mmap_addr;
             (u64)mmap != (u64)multiboot->mmap_addr + multiboot->mmap_length;
             mmap = (multiboot_memory_map_t*)((char*)mmap + mmap->size + sizeof(mmap->size))) {

            void* addr = (void*)mmap->addr;
            size_t length = (size_t)mmap->len;

            // only use it if type is 1 (usable RAM)
            if (mmap->type != 1)
                continue;

            // only use the first physical gb
            if ((size_t)addr >= 0x40000000)
                continue;

            // don't let this shit overlap with the kernel itself
            if ((size_t)addr < (size_t)KERNEL_END_PHYSICAL) {
                size_t diff = (size_t)KERNEL_END_PHYSICAL - (size_t) addr;
                addr = (void*)((size_t)addr + diff);

                if (length < diff)
                    continue;

                length -= diff;
            }
            
            // and let the physical memory manager know that memory is free.
            // we have to round it up the the nearest page though
            u32 offset = (0x1000 - (u32)addr % 0x1000);
            physicalMemoryManager.registerAvailableMemory((void*) ((size_t) addr + offset), length - offset);
        }
    }

    // Now that we can allocate physical memory, initialize the paging. This should set up a valid not 4MB pagetable, with only the KERNEL_BEGIN_PHYSICAL to KERNEL_END_PHYSICAL mapped to the higher half, and nothing more.
    PageInit();
    LOG_STARTUP("Paging initialized.");
}

void printrec(DirEntry* root) {
    while (root->valid()) {
        if (root->name() != "." && root->name() != "..") {
            printf("%s ", root->name().c_str());
            if (root->type() == DirEntryType::Folder) {
                DirEntry* next = root->openDir();
                printrec(next);
                delete next;
            }
        }
        root->advance();
    }
}

void loadEventHandlers()
{
	new CMOS();
}

extern "C" void main(multiboot_info* multiboot) {
    // init lowlevel CPU related stuff
    // None of these should be allowed to touch the memory allocator, etc
	initCpu();

    // init the memory management, so we have proper paging and can allocate memory
    initMemory(multiboot);

    acpi::init();
    
    cpuid_field features = cpuid(1);
    if ((features.edx & (int)cpuid_feature::EDX_APIC) != 0) {
        LOG_STARTUP("Initializing APIC.");
        apic::Init();
    } else {
        LOG_STARTUP("APIC not supported, skipping. (Threading will not be supported)");
    }

	// Initialize the serial driver so that we can output debug messages very early.
	initSerialDevices();
	LOG_STARTUP("Serial driver initialized.");

	// Initialize the PCI driver
	PCI::init();
	LOG_STARTUP("PCI driver initialized.");

	// initialize the ATA driver
    ataDriver.initialize();
    LOG_STARTUP("ATA driver initialized.");

    // and register the default vga and and keyboard driver
    deviceManager.addDevice(&vgaDriver);
	LOG_STARTUP("VGA driver initialized.");
    
    deviceManager.addDevice(new KeyboardDriver());
	LOG_STARTUP("Keyboard driver initialized.");

	vfs = new VirtualFileSystem();
    LOG_STARTUP("Virtual filesystem created.");

    /*auto storagedevices = deviceManager.getDevices(DeviceType::Storage);
    for (size_t i =  0; i < storagedevices.size() ; i++) {
		BlockDevice* device = (BlockDevice*) storagedevices[i];
        DeviceStorageInfo info;
        device->getDeviceInfo(&info);
        char* name = "hdda";
        name[3] += i;
        LOG_STARTUP("BINDING %s to %s", info.deviceInfo.name, "hdd");
        vfs->BindFilesystem(name, new Iso9660FileSystem(device));
    }
    LOG_STARTUP("Bound filesystems.");*/

	/*char* mainL = (char*) main;
	char* physL = (char*) kernelPageDirectory.getPhysicalAddress((void*)mainL);
	printf("Location of 0x%X = 0x%X\n", mainL, physL);

	physL = (char*) physicalMemoryManager.allocatePhysicalMemory(0xFFFFF);
	char* virtL = (char*) kernelPageDirectory.getVirtualAddress((void*)physL);
	printf("Physical 0x%X is located at 0x%X\n", physL, virtL);

	printf("Val 0x%X:0x%X\n", *mainL, *virtL);
	*mainL = 51;
	printf("Val 0x%X:0x%X\n", *mainL, *virtL);*/

    //KeyboardDriver driver;
    
    //while (true) {
        //VirtualKeyEvent input[10];
        //int read = driver.read(input, 10);

        // send them to the input formatter
        /*for (size_t i = 0 ; i < read ; i += sizeof(VirtualKeyEvent))
        {
            putchar((u8)input[i].vkey);
        }*/
    //}

    /*Iso9660FileSystem fs(deviceManager.getDevice(DeviceType::Storage, 0));
    DirEntry* dir = fs.getRoot();
    printrec(dir);
    delete dir;*/
    
    /*char* data = new char[2048];

    if (deviceManager.getDevice(DeviceType::Storage, 0)->seek(0x10 * SIZEOF_KB * 2, SEEK_SET) != 0)
        printf("err seeking");
    printf("found");
    
    deviceManager.getDevice(DeviceType::Storage, 0)->read(data, 2048);
    

    printf("id is: %d", data[0]);

    delete[] data;
	*/

    /*const char* string = "/test/help/bvb";
    int depth = 0;
    while (true) {
        int length = strcspn(string, "/");

        printf("Dir %d: ", depth);
        for (int i = 0 ; i < length ; i++) {
            printf("%c", string[i]);
        }
        printf("\n");

        string += length;
        if (*string == '\0')
            break;
        else
            string++;

        depth++;
	}*/

	NullListener bus;
	loadEventHandlers();

	GetTimeEvent event;
	event.prepare(TARGET_ANY);
	TimeResponse* response = (TimeResponse*) bus.fireEventAndWait(event);
	//TimeResponse response = ((TimeResponse) bus.getResponse());
	LOG_INFO("Boot finished at %d:%d:%d %d/%d/%d", response->hours, response->minutes, response->seconds, response->day, response->month, response->year);
	delete response;

    KernelShell shell;
    shell.enter();
}
