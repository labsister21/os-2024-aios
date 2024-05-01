#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"

static struct FAT32DriverState driver_state;

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

uint32_t cluster_to_lba(uint32_t cluster){
    return cluster * CLUSTER_SIZE;
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster){
    struct FAT32DirectoryEntry *table = dir_table->table;
    
    // Entry-0 contains self (attr name = name)
    memcpy(table[0].name, name, 8);
    table[0].user_attribute = UATTR_NOT_EMPTY;
    table[0].attribute = ATTR_SUBDIRECTORY;

    // Entry-1 contains parent dir
    table[1].user_attribute = UATTR_NOT_EMPTY;
    table[1].cluster_low = parent_dir_cluster;
    table[1].cluster_high = parent_dir_cluster >> 16;

    // Set all remaining entry to empty
    for (int i = 2; i < 64; i++)
    {
        table[i].user_attribute = !UATTR_NOT_EMPTY;
    }
}

bool is_empty_storage(void){
    struct BlockBuffer block;
    read_blocks(block.buf, BOOT_SECTOR, 1);
    return memcmp(block.buf, fs_signature, BLOCK_SIZE) != 0;
}

void create_fat32(void){
    write_blocks(&fs_signature, BOOT_SECTOR, 1);

    driver_state.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    driver_state.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    driver_state.fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE;

    for(uint16_t i = 3; i < CLUSTER_MAP_SIZE; i++){
        driver_state.fat_table.cluster_map[i] = FAT32_FAT_EMPTY_ENTRY;
    }

    write_clusters(&driver_state.fat_table, 1, 1);
    init_directory_table(&driver_state.dir_table_buf, "root", 2);
    write_clusters(&driver_state.dir_table_buf.table, 2, 1);
    // initialize root directory and write it to cluster 2
    // struct FAT32DirectoryTable rootDir = {
    //     .table = {
    //         {
    //         .name = {'r','o','o','t'},
    //         .attribute = ATTR_SUBDIRECTORY,
    //         .user_attribute = UATTR_NOT_EMPTY,
    //         .cluster_high = 0x00,
    //         .cluster_low = 0x02,
    //         .filesize = 0 
    //         }
    //     }
    // };
    // write_clusters(rootDir.table, 2, 1);
}

void initialize_filesystem_fat32(void){
    if(is_empty_storage())
        create_fat32();
    else
        read_clusters(&driver_state.fat_table, 1, 1);
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    write_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    read_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
}

int8_t read_directory(struct FAT32DriverRequest request){
    read_clusters(&driver_state.dir_table_buf, request.parent_cluster_number, 1);
    if(!(driver_state.dir_table_buf.table->user_attribute == UATTR_NOT_EMPTY)){
        return -1;
    }
    for (int i = 0; i<(int)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry));i++){
        if(memcmp(driver_state.dir_table_buf.table[i].name, request.name,8)){
            if (driver_state.dir_table_buf.table[i].attribute != ATTR_SUBDIRECTORY){
                return 1;
            } else{
                write_clusters(driver_state.dir_table_buf.table, request.parent_cluster_number, 1);
                // Read directory
                read_clusters(request.buf, ((driver_state.dir_table_buf.table[i].cluster_high << 16) + driver_state.dir_table_buf.table[i].cluster_low),1);
                return 0;
            }
        }
    }
    return 2;
}

int8_t read(struct FAT32DriverRequest request){
    // OPTIONAL
    read_clusters(&driver_state.dir_table_buf.table, request.parent_cluster_number, 1);
    if(!(driver_state.dir_table_buf.table->user_attribute == UATTR_NOT_EMPTY)){
        return -1;
    }
    read_clusters(&driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
    for (int i = 0; i < (int)(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry));i++){
        if(memcmp(driver_state.dir_table_buf.table[i].name, request.name,8) == 0 && memcmp(driver_state.dir_table_buf.table[i].ext,request.ext,3) == 0){
            if (driver_state.dir_table_buf.table[i].attribute == ATTR_SUBDIRECTORY){
                return 1;
            }else if(driver_state.dir_table_buf.table[i].filesize > request.buffer_size){
                return -1;
            }else{
                int counter = 0;
                int cluster_number = (driver_state.dir_table_buf.table[i].cluster_high << 16) + driver_state.dir_table_buf.table[i].cluster_low;
                while(cluster_number != FAT32_FAT_END_OF_FILE){
                    read_clusters(&request.buf + CLUSTER_SIZE*counter, cluster_number, 1);
                    cluster_number = driver_state.fat_table.cluster_map[cluster_number];
                    counter++;
                }
                return 0;
            }
        }else{
            return 2;
        }
    }
    return -1;
}

int8_t write(struct FAT32DriverRequest request){
    // Read parent directory table & Read FAT table to drive state
    read_clusters(&driver_state.dir_table_buf, request.parent_cluster_number, 1);
    read_clusters(&driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

    // Check if the parent directory is valid and is a directory
    if(!(driver_state.dir_table_buf.table[0].user_attribute == UATTR_NOT_EMPTY)){
        return 2;
    }
    
    struct FAT32DirectoryEntry *table = driver_state.dir_table_buf.table;
    int required_clusters = request.buffer_size/CLUSTER_SIZE + (request.buffer_size % CLUSTER_SIZE != 0); // ceiling division
    // New directory needs 1 cluster
    if(required_clusters == 0) {
        required_clusters = 1;
    }

    int entry_number = -1;
    uint32_t cluster_numbers[required_clusters];
    int available_clusters = 0;

    // Check if there is a file with the same name or if the directory table is full
    for (int i = 2; i < 64; i++) {
        if (table[i].user_attribute != UATTR_NOT_EMPTY && available_clusters != required_clusters){
            // Find available clusters in FAT Table
            for (int j = 3; j < 512; j++) {
                if (driver_state.fat_table.cluster_map[j] == FAT32_FAT_EMPTY_ENTRY){
                    cluster_numbers[available_clusters] = j;
                    available_clusters++;
                }
                if (available_clusters == required_clusters) {
                    entry_number = i;
                    break;
                }
            }
        }
        // Check files with the same name and extension
        if (memcmp(request.name, table[i].name, 8) == 0 && memcmp(request.ext, table[i].ext, 3) == 0) {
            return 1;
        }
    }
    // Check if Directory Table is full
    if (entry_number == -1){
        return -1;
    }

    // Create new entry;
    table[entry_number].user_attribute = UATTR_NOT_EMPTY;
    table[entry_number].cluster_high = cluster_numbers[0] >> 16;
    table[entry_number].cluster_low = cluster_numbers[0];
    memcpy(table[entry_number].name, request.name, 8);

    // Request is directory
    if (request.buffer_size == 0) {
        // Set attribute to be a subdirectory
        table[entry_number].attribute = ATTR_SUBDIRECTORY;
        
        // Create new Directory Table & Initialize
        struct FAT32DirectoryTable newDirectoryTable;
        init_directory_table(&newDirectoryTable, request.name, request.parent_cluster_number);

        // Update driver_state FAT Table
        driver_state.fat_table.cluster_map[cluster_numbers[0]] = FAT32_FAT_END_OF_FILE;
        
        // Write Directory table to cluster
        write_clusters(&newDirectoryTable, cluster_numbers[0], 1);

    } else { // Request is a file
        table[entry_number].attribute = !ATTR_SUBDIRECTORY;
        memcpy(table[entry_number].ext, request.ext, 3);
        table[entry_number].filesize = request.buffer_size;

        // Write file buffer
        for (int k = 0; k < required_clusters; k++) {
            write_clusters(request.buf + k * CLUSTER_SIZE, cluster_numbers[k], 1);
            // Update FAT (Linked List)
            if (k != required_clusters-1){
                driver_state.fat_table.cluster_map[cluster_numbers[k]] =  cluster_numbers[k+1];
            } else {
                driver_state.fat_table.cluster_map[cluster_numbers[k]] = FAT32_FAT_END_OF_FILE;
            }
        }

    }

    // Update FAT table
    write_clusters(driver_state.fat_table.cluster_map, 1, 1);
    // Update parent's Directory table
    write_clusters(table, request.parent_cluster_number, 1);
    
    return 0;
}

int8_t delete(struct FAT32DriverRequest request){
    // OPTIONAL
    read_clusters(&driver_state.dir_table_buf, request.parent_cluster_number, 1);
    struct FAT32DirectoryEntry *table = driver_state.dir_table_buf.table;

    for(uint8_t i = 1; i < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i ++){
        if (table[i].user_attribute == UATTR_NOT_EMPTY && memcmp(request.name, table[i].name,8) == 0){
            // entry adalah directory
            if(table[i].attribute){
                struct FAT32DirectoryTable dirTable;
                read_clusters(&dirTable, table[i].cluster_low, 1);
                for (unsigned int i = 1 ; i < (CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)); i++){
                    // cek kalo ga empty
                    if(dirTable.table[i].user_attribute == UATTR_NOT_EMPTY){
                        return 2;
                    }
                }
                driver_state.fat_table.cluster_map[table[i].cluster_low] = 0;
                table[i].user_attribute = !UATTR_NOT_EMPTY;
                write_clusters(&driver_state.dir_table_buf, request.parent_cluster_number, 1);
                write_clusters(&driver_state.fat_table,1 ,1);
                return 0;
            } 
            // entry adalah file
            else{
                if( memcmp(table[i].ext, request.ext,3) == 0){
                    table[i].user_attribute = !UATTR_NOT_EMPTY;
                    write_clusters(&driver_state.dir_table_buf, request.parent_cluster_number,1);
                    
                    uint16_t temp = 0;
                    if(table[i].filesize % CLUSTER_SIZE == 0){
                        temp = table[i].filesize / CLUSTER_SIZE;
                    } else{
                        temp = (table[i].filesize / CLUSTER_SIZE) +1;
                    }

                    uint16_t cluster_count = temp;
                    uint16_t to_zeros[cluster_count];
                    uint16_t current_cluster = table[i].cluster_low;


                    for(uint16_t i =0; i < cluster_count; i++){
                        to_zeros[i] = current_cluster;
                        current_cluster = driver_state.fat_table.cluster_map[current_cluster];
                    }
                    for(uint16_t i =0; i < cluster_count; i++){
                        driver_state.fat_table.cluster_map[to_zeros[i]] = 0;
                    }

                    write_clusters(&driver_state.fat_table,1,1);
                    return 0;
                } else{
                    //do nothing
                }
            }
        }
    }
    return 1;
}