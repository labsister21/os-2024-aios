#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/memory/paging.h"
#include "header/process/process.h"

__attribute__((aligned(0x1000))) static struct PageDirectory page_directory_list[PAGING_DIRECTORY_TABLE_MAX_COUNT] = {0};

__attribute__((aligned(0x1000))) struct PageDirectory _paging_kernel_page_directory = {
    .table = {
        [0] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address          = 0,
        },
        [0x300] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address          = 0,
        },
    }
};

static struct PageManagerState page_manager_state = {
    .page_frame_map = {
        [0] = true, [1] = false, [2] = false, [3] = false, [4] = false, [5] = false, [6] = false,
        [7] = false, [8] = false, [9] = false, [10] = false, [11] = false, [12] = false,
        [13] = false, [14] = false, [15] = false, [16] = false, [17] = false, [18] = false,
        [19] = false, [20] = false, [21] = false, [22] = false, [23] = false, [24] = false,
        [25] = false, [26] = false, [27] = false, [28] = false, [29] = false, [30] = false, [31] = false,
    },
    .free_page_frame_count = PAGE_FRAME_MAX_COUNT - 1    
};

static struct {
    bool page_directory_used[PAGING_DIRECTORY_TABLE_MAX_COUNT];
} page_directory_manager = {
    .page_directory_used = {false},
};

struct PageDirectory* paging_create_new_page_directory(void) {
    /*
     * TODO: Get & initialize empty page directory from page_directory_list
     * - Iterate page_directory_list[] & get unused page directory
     * - Mark selected page directory as used
     * - Create new page directory entry for kernel higher half with flag:
     *     > present bit    true
     *     > write bit      true
     *     > pagesize 4 mb  true
     *     > lower address  0
     * - Set page_directory.table[0x300] with kernel page directory entry
     * - Return the page directory address
     */ 
    for(uint8_t i = 0; i < PAGING_DIRECTORY_TABLE_MAX_COUNT; i++) {
        if(!page_directory_manager.page_directory_used[i]) {
            page_directory_manager.page_directory_used[i] = true;
            struct PageDirectory insert = {
                .table = {
                    [0] = {
                        .flag.present_bit       = 1,
                        .flag.write_bit         = 1,
                        .flag.use_pagesize_4_mb = 1,
                        .lower_address          = 0,
                    },
                    [0x300] = {
                        .flag.present_bit       = 1,
                        .flag.write_bit         = 1,
                        .flag.use_pagesize_4_mb = 1,
                        .lower_address          = 0,
                    }
                }
            };
            flush_single_tlb(&page_directory_list[i]);
            page_directory_list[i] = insert;
            return &page_directory_list[i];
        }
    }
}

struct PageDirectory* paging_get_current_page_directory_addr(void) {
    uint32_t current_page_directory_phys_addr;
    __asm__ volatile("mov %%cr3, %0" : "=r"(current_page_directory_phys_addr): /* <Empty> */);
    uint32_t virtual_addr_page_dir = current_page_directory_phys_addr + KERNEL_VIRTUAL_ADDRESS_BASE;
    return (struct PageDirectory*) virtual_addr_page_dir;
}

void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr, 
    void *virtual_addr, 
    struct PageDirectoryEntryFlag flag
) {
    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flag          = flag;
    page_dir->table[page_index].lower_address = ((uint32_t) physical_addr >> 22) & 0x3FF;
    flush_single_tlb(virtual_addr);
}

void flush_single_tlb(void *virtual_addr) {
    asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
}



/* --- Memory Management --- */
bool paging_allocate_check(uint32_t amount) {
    uint32_t required = (PAGE_FRAME_SIZE+amount-1) /PAGE_FRAME_SIZE;
    return required <= page_manager_state.free_page_frame_count;
}


bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    // Find a free physical frame
    for (size_t i = 0; i < PAGE_FRAME_MAX_COUNT; ++i) {
        if (!page_manager_state.page_frame_map[i])
        {
            // change it as used
            page_manager_state.page_frame_map[i] = true;
            // decrease count
            page_manager_state.free_page_frame_count--;

            struct PageDirectoryEntryFlag flag =
                {
                    .present_bit = 1,
                    .write_bit = 1,
                    .privilege = 1,
                    .use_pagesize_4_mb = 1};

            // Update page directory entry
            void *physical_addr = (void*)(i*PAGE_FRAME_SIZE);
            update_page_directory_entry(page_dir, physical_addr, virtual_addr, flag);
            return true;
        }
    }
    return false;
}

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    // take 10 bit significant from virtual_addr
    uint32_t addr_table = ((uint32_t)virtual_addr>>22) & 0x3FF;

    // Check if the page table entry is valid and the page frame is allocated
    if(page_dir->table[addr_table].flag.present_bit){
        // change it as free
        uint16_t idx = page_dir->table[addr_table].lower_address;
        page_manager_state.page_frame_map[idx] = false;

        page_manager_state.free_page_frame_count++; // Increase count

        // Remove the entry by setting it into 0
        page_dir->table[addr_table].flag.present_bit = 0;

        flush_single_tlb(virtual_addr);
        return true;
    } else {
        return false;
    }
}

bool paging_free_page_directory(struct PageDirectory *page_dir) {
     for(uint8_t i = 0; i < PAGING_DIRECTORY_TABLE_MAX_COUNT; i++) {
        if(&page_directory_list[i] == page_dir) {
            page_directory_manager.page_directory_used[i] = false;
            flush_single_tlb(page_dir);
            return true;
        }
     }
    return false;
}

void paging_use_page_directory(struct PageDirectory *page_dir_virtual_addr) {
    uint32_t physical_addr_page_dir = (uint32_t) page_dir_virtual_addr;
    // Additional layer of check & mistake safety net
    if ((uint32_t) page_dir_virtual_addr > KERNEL_VIRTUAL_ADDRESS_BASE)
        physical_addr_page_dir -= KERNEL_VIRTUAL_ADDRESS_BASE;
    __asm__  volatile("mov %0, %%cr3" : /* <Empty> */ : "r"(physical_addr_page_dir): "memory");
}
