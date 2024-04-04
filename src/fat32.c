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
    // asdfkladfj;a
}

bool is_empty_storage(void){
    struct BlockBuffer block;
    read_blocks(block.buf, BOOT_SECTOR, 1);
    return !memcmp(block.buf, fs_signature, BLOCK_SIZE);
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
}

int8_t read(struct FAT32DriverRequest request){
    // OPTIONAL
    read_clusters(&driver_state.dir_table_buf, request.parent_cluster_number, 1);
    if(!(driver_state.dir_table_buf.table->user_attribute == UATTR_NOT_EMPTY)){
        return -1;
    }
    read_clusters(&driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
    for (int i = 0; i<(CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry));i++){
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
            }
        }else{
            return 2;
        }
    }
}

int8_t write(struct FAT32DriverRequest request){
    // OPTIONAL
}

int8_t delete(struct FAT32DriverRequest request){
    // OPTIONAL
}