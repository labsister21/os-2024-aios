#include "header/cpu/interrupt.h"
#include "header/cpu/gdt.h"
#include "header/cpu/portio.h"
#include "header/driver/keyboard.h"
#include "header/filesystem/fat32.h"
#include "header/text/framebuffer.h"
#include "header/process/scheduler.h"
#include "header/stdlib/string.h"
#include "header/process/process.h"

void io_wait(void){
    out(0x80, 0);
}

void pic_ack(uint8_t irq){
    if (irq >= 8) out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void){
    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100); // ICW3: tell Master PIC, slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Disable all interrupts
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

void main_interrupt_handler(struct InterruptFrame frame){
    struct Context currentContext = {0};
    switch(frame.int_number){
        case PIC1_OFFSET + IRQ_KEYBOARD:
            keyboard_isr();
            break;
        case 0x30:
            syscall(frame);
            break;
        case PIC1_OFFSET + IRQ_TIMER:
            //copy frame ke context variable
            memcpy(&currentContext, &frame, sizeof(struct Context));
            //save current context ke running pcb
            scheduler_save_context_to_current_running_pcb(currentContext);
            //switch to next process (process context switch)
            scheduler_switch_to_next_process();
            break;
    }
}

void activate_keyboard_interrupt(void){
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
}

struct TSSEntry _interrupt_tss_entry = {
    .ss0  = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};

void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8; 
}

void syscall(struct InterruptFrame frame) {
    switch (frame.cpu.general.eax) {
        case 0:
            *((int8_t*) frame.cpu.general.ecx) = read(*(struct FAT32DriverRequest*) frame.cpu.general.ebx);
            break;
        case 1:
            *((int8_t *)frame.cpu.general.ecx) = read_directory(*(struct FAT32DriverRequest*)frame.cpu.general.ebx);
            break;
        case 2:
            *((int8_t *)frame.cpu.general.ecx) = write(*(struct FAT32DriverRequest*)frame.cpu.general.ebx);
            break;
        case 3:
            *((int8_t *)frame.cpu.general.ecx) = delete(*(struct FAT32DriverRequest*)frame.cpu.general.ebx);
            break;
        case 4:
            get_keyboard_buffer((char*) frame.cpu.general.ebx);
            break;
        case 5:
            puts_char(
                *((char*)frame.cpu.general.ebx), 
                frame.cpu.general.ecx
            );
            *((char*)frame.cpu.general.ebx) = 0;
            break;
        case 6:
            puts(
                (char*) frame.cpu.general.ebx, 
                frame.cpu.general.ecx, 
                frame.cpu.general.edx
            ); // Assuming puts() exist in kernel
            break;
        case 7: 
            keyboard_state_activate();
            break;
        case 8:
            read_clusters((struct FAT32DirectoryTable*) frame.cpu.general.ebx, (int) frame.cpu.general.ecx, 1);
            break;
        case 9:
            framebuffer_clear();
            framebuffer_set_cursor(0, 0);
            break;
        case 10:
            // Set process to terminates based on PID;
            *(bool*)frame.cpu.general.ecx = process_destroy(frame.cpu.general.ebx);

            break;
        case 11:
            // create user process
            *(uint32_t*)frame.cpu.general.ecx = process_create_user_process(*(struct FAT32DriverRequest*)frame.cpu.general.ebx);
            break;
        case 12:
            // get Process information
            for (int i = 0; i < 16; i++) {
                if (_process_list[i].metadata.state != KILLED) {
                    ((int*) frame.cpu.general.ebx)[i] = _process_list[i].metadata.pid;
                }
            }
            break;
    }
}
