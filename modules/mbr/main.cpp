#include "filesystem/file.h"
#include "filesystem/registry.h"
#include "filesystem/fileview.h"
#include "types.h"
#include <stdio.h>

struct ChsAddress {
    u8 head;
    u8 sector;
    u8 cylinder;
} __attribute__((packed));

struct PartitionTableEntry {
    u8 attributes;
    ChsAddress start;
    u8 type;
    ChsAddress end;
    u32 startLba;
    u32 sectorCount;
} __attribute__((packed));

void process_partition_table_entry(fs::File* file, PartitionTableEntry* entry) {
}

void check_mbr(fs::File* file)
{
    u8 header[512];

    // Read the mbr header
    fs::FileHandle* h = file->open();
    h->read(header, 512, 0);
    h->close();

    if (header[0x1BC] != 0 || header[0x1BD] != 0) {
        return;
    }

    if (header[0x1FE] != 0x55 || header[0x1FF] != 0xAA) {
        return;
    }

    PartitionTableEntry* partitions = (PartitionTableEntry*)(header+0x1BE);
    for (int i = 0 ; i < 4 ; i++) {
        PartitionTableEntry* entry = &partitions[i];
        
        int active = (entry->attributes & 0x80) > 0;

        if (active) {

            if (entry->type == 0xCD) {
                printf("Found iso\n");
                char partname[64];
                sprintf(partname, "%s_%d", file->get_name(), i+1);
                auto partition = new fs::FileView(partname, file, entry->startLba*512, entry->sectorCount*512);
                fs::root->get("dev")->bind(partition);
                fs::registry->create_filesystem("ISO9660", partition, entry->startLba*512, entry->sectorCount*512);
            }
        }
    }
}

extern "C" void module_load(fs::File* root, const char* argv)
{
    fs::File* dev = root->get("dev");
	fs::FileHandle* desc = dev->open();
	
	fs::File* child = nullptr;

	while ((child = desc->next_child()) != nullptr) {
        check_mbr(child);
	}
}
