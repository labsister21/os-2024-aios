#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"
#include "header/process/process.h"


struct ProcessManagerState process_manager_state;
struct ProcessControlBlock _process_list[PROCESS_COUNT_MAX] = {0};

struct ProcessControlBlock* process_get_current_running_pcb_pointer(void) {
    for(uint8_t i = 0; i < PROCESS_COUNT_MAX; i++) {
        if(_process_list[i].metadata.state == RUNNING) {
            return &_process_list[i];
        }
    }
    return NULL;
}

int32_t process_create_user_process(struct FAT32DriverRequest request) {
    int32_t retcode = PROCESS_CREATE_SUCCESS;
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }

    // Ensure entrypoint is not located at kernel's section at higher half
    if ((uint32_t) request.buf >= KERNEL_VIRTUAL_ADDRESS_BASE) {
        retcode = PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT;
        goto exit_cleanup;
    }

    // Check whether memory is enough for the executable and additional frame for user stack
    uint32_t page_frame_count_needed = ceil_div(request.buffer_size + PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
    if (!paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    // Process PCB
    struct PageDirectory *current_pagedir = paging_get_current_page_directory_addr();
    struct PageDirectory* pagedir = paging_create_new_page_directory();
    void* pagingProcess = paging_allocate_user_page_frame(pagedir, request.buf);
    void* paging = paging_allocate_user_page_frame(pagedir, (void *)0xBFFFFFFC);

    if(!(paging && pagingProcess)) {
        retcode = PROCESS_CREATE_FAIL_PAGING_ALLOC_FAILURE;
        goto exit_cleanup;
    }

    paging_use_page_directory(pagedir);

    int8_t readfsStatus = read(request);
    if(readfsStatus != 0) {
        retcode = PROCESS_CREATE_FAIL_FS_READ_FAILURE;
        goto exit_cleanup;
    }


    int32_t p_index = process_list_get_inactive_index();
    struct ProcessControlBlock *new_pcb = &(_process_list[p_index]);
    process_manager_state.used_process[p_index] = true;

    new_pcb->memory.virtual_addr_used[0] = pagingProcess + KERNEL_VIRTUAL_ADDRESS_BASE;
    new_pcb->memory.virtual_addr_used[1] = paging + KERNEL_VIRTUAL_ADDRESS_BASE;
    new_pcb->memory.page_frame_used_count = 2;
    new_pcb->context.eip = (uint32_t)request.buf;
    new_pcb->context.cpu.stack.ebp = 0xBFFFFFFC;
    new_pcb->context.cpu.stack.esp = 0xBFFFFFFC;
    new_pcb->context.eflags |= CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE;
    new_pcb->context.cpu.segment.gs = 0x20 | 0x3;
    new_pcb->context.cpu.segment.fs = 0x20 | 0x3;
    new_pcb->context.cpu.segment.es = 0x20 | 0x3;
    new_pcb->context.cpu.segment.ds = 0x20 | 0x3;
    new_pcb->context.ss = 0x20 | 0x3;
    new_pcb->context.cs = 0x18 | 0x3;
    new_pcb->context.page_directory_virtual_addr = pagedir;
    new_pcb->metadata.pid = p_index;
    memcpy(new_pcb->metadata.name, "\0\0\0\0\0\0\0\0\0", 10);
    memcpy(new_pcb->metadata.name, request.name, 8);
    new_pcb->metadata.state = READY;
    paging_use_page_directory(current_pagedir);
    process_manager_state.active_process_count++;
exit_cleanup:
    return retcode;
}

bool process_destroy(uint32_t pid) {
    for(uint8_t i = 0; i < PROCESS_COUNT_MAX; i++) {
        if(_process_list[i].metadata.pid == pid) {
            memset(&_process_list[i], 0, sizeof(struct ProcessControlBlock));
            _process_list[i].metadata.state = KILLED;
            process_manager_state.used_process[pid] = false;
            process_manager_state.active_process_count--;
            return true;
        }
    }
    return false;
}

uint32_t process_list_get_inactive_index() {
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        if (!process_manager_state.used_process[i]) {
            return i;
        }
    }
    return -1;
}

int32_t process_generate_new_pid() {
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        if (!process_manager_state.used_process[i]) {
            return i;
        }
    };
    return -1;
}

uint32_t ceil_div(uint32_t x, uint32_t y) {
    return 1 + ((x - 1) / y); // if x != 0
}
