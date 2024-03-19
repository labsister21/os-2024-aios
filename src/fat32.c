#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>>
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"

static struct FAT32DriverState fat32driver_state;

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
    return(cluster * CLUSTER_SIZE);
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster){
    // OPTIONAL
}

bool is_empty_storage(void){
    // Waiting...
}

void create_fat32(void){
    // Waiting...
}

void initialize_filesystem_fat32(void){
    // Waiting...
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    // OPTIONAL
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    // OPTIONAL
}

int8_t read_directory(struct FAT32DriverRequest request){
    // OPTIONAL
}

int8_t read(struct FAT32DriverRequest request){
    // OPTIONAL
}

int8_t write(struct FAT32DriverRequest request){
    // OPTIONAL
}

int8_t delete(struct FAT32DriverRequest request){
    // OPTIONAL
}