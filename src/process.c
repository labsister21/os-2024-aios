#include "header/process/process.h"
#include "header/filesystem/fat32.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"

extern struct PageManagerState process_manager_state;
static struct ProcessControlBlock _process_list[PROCESS_COUNT_MAX] = {0};

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
    struct PageDirectory* pagedir = paging_create_new_page_directory();

    bool pagingStatus = paging_allocate_check(request.buffer_size);
    if(!pagingStatus) {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    

    pagingStatus = paging_allocate_user_page_frame(pagedir, request.buf);


    int8_t readfsStatus = read(request);
    if(readfsStatus != 1) {
        retcode = PROCESS_CREATE_FAIL_FS_READ_FAILURE;
        goto exit_cleanup;
    }

    int32_t p_index = process_list_get_inactive_index();
    struct ProcessControlBlock *new_pcb = &(_process_list[p_index]);

    new_pcb->metadata.pid = process_generate_new_pid();

    
    new_pcb->metadata.state = READY;
    new_pcb->context.eflags |= CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE;


exit_cleanup:
    return retcode;
}

bool process_destroy(uint32_t pid) {
    for(uint8_t i = 0; i < PROCESS_COUNT_MAX; i++) {
        if(_process_list[i].metadata.pid == pid) {
            memset(&_process_list[i], 0, sizeof(struct ProcessControlBlock));
            _process_list[i].metadata.state = KILLED;
            return true;
        }
    }
    return false;
}

uint32_t process_list_get_inactive_index() {
    for(uint8_t i = 0; i < PROCESS_COUNT_MAX; i++) {
        if(_process_list[i].metadata.state == KILLED) {
            return i;
        }
    }
    return -1;
}

int32_t process_generate_new_pid() {
    return process_manager_state.active_process_count + 1;
}
