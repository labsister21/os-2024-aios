#include "header/process/scheduler.h"
#include "header/cpu/portio.h"

#define PIT_MAX_FREQUENCY   1193182
#define PIT_TIMER_FREQUENCY 1000
#define PIT_TIMER_COUNTER   (PIT_MAX_FREQUENCY / PIT_TIMER_FREQUENCY)

#define PIT_COMMAND_REGISTER_PIO          0x43
#define PIT_COMMAND_VALUE_BINARY_MODE     0b0
#define PIT_COMMAND_VALUE_OPR_SQUARE_WAVE (0b011 << 1)
#define PIT_COMMAND_VALUE_ACC_LOHIBYTE    (0b11  << 4)
#define PIT_COMMAND_VALUE_CHANNEL         (0b00  << 6) 
#define PIT_COMMAND_VALUE (PIT_COMMAND_VALUE_BINARY_MODE | PIT_COMMAND_VALUE_OPR_SQUARE_WAVE | PIT_COMMAND_VALUE_ACC_LOHIBYTE | PIT_COMMAND_VALUE_CHANNEL)

#define PIT_CHANNEL_0_DATA_PIO 0x40

int running_process_idx;

void activate_timer_interrupt(void) {
    __asm__ volatile("cli");
    // Setup how often PIT fire
    uint32_t pit_timer_counter_to_fire = PIT_TIMER_COUNTER;
    out(PIT_COMMAND_REGISTER_PIO, PIT_COMMAND_VALUE);
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) (pit_timer_counter_to_fire & 0xFF));
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) ((pit_timer_counter_to_fire >> 8) & 0xFF));

    // Activate the interrupt
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_TIMER));
}


/**
 * Read all general purpose register values and set control register.
 * Resume the execution flow back to ctx.eip and ctx.eflags
 * 
 * @note          Implemented in assembly
 * @param context Target context to switch into
 */
// __attribute__((noreturn)) extern void process_context_switch(struct Context ctx);



/* --- Scheduler --- */
/**
 * Initialize scheduler before executing init process 
 */
void scheduler_init(void){
    activate_timer_interrupt();
    process_manager_state.active_process_count = 0;
    running_process_idx = 0;
}

/**
 * Save context to current running process
 * 
 * @param ctx Context to save to current running process control block
 */
void scheduler_save_context_to_current_running_pcb(struct Context ctx){
    struct ProcessControlBlock* currentProcess = process_get_current_running_pcb_pointer();
    currentProcess->context = ctx;
}

/**
 * Trigger the scheduler algorithm and context switch to new process
 */
__attribute__((noreturn)) void scheduler_switch_to_next_process(void){
    int pilihan = 0;
    // pilih process
    for (int i = 1; i < PROCESS_COUNT_MAX; i++) {
        if (_process_list[(i + running_process_idx) % PROCESS_COUNT_MAX].metadata.state == READY) {
            pilihan = (i + running_process_idx) % PROCESS_COUNT_MAX;
            break;
        }
    }
    _process_list[running_process_idx].metadata.state = READY;
    struct ProcessControlBlock pickedProcess = _process_list[pilihan];
    running_process_idx = pilihan;
    _process_list[running_process_idx].metadata.state = RUNNING;
    
    // pic ack --> 
    pic_ack(IRQ_TIMER + PIC1_OFFSET);

    // use page directory dari pcb --> ganti register cr3
    paging_use_page_directory(&(pickedProcess.context.page_directory_virtual_addr));

    // context switch
    process_context_switch(pickedProcess.context);
}
