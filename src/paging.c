#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/memory/paging.h"

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
// TODO: Implement
bool paging_allocate_check(uint32_t amount) {
    // TODO: Check whether requested amount is available
    uint32_t required = (PAGE_FRAME_SIZE+amount-1) /PAGE_FRAME_SIZE;
    return required <= page_manager_state.free_page_frame_count;
}


bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    /*
     * TODO: Find free physical frame and map virtual frame into it
     * - Find free physical frame in page_manager_state.page_frame_map[] using any strategies
     * - Mark page_manager_state.page_frame_map[]
     * - Update page directory with user flags:
     *     > present bit    true
     *     > write bit      true
     *     > user bit       true
     *     > pagesize 4 mb  true
     */
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
    /* 
     * TODO: Deallocate a physical frame from respective virtual address
     * - Use the page_dir.table values to check mapped physical frame
     * - Remove the entry by setting it into 0
     */

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

