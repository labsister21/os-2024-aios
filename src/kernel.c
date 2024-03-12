#include "header/cpu/gdt.h"
#include "header/cpu/idt.h"
#include "header/cpu/interrupt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"
#include "header/driver/keyboard.h"

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    /*
    framebuffer_clear();
    framebuffer_write(3, 8,  'H', 0, 0xF);
    framebuffer_write(3, 9,  'a', 0, 0xF);
    framebuffer_write(3, 10, 'i', 0, 0xF);
    framebuffer_write(3, 11, '!', 0, 0xF);
    framebuffer_set_cursor(3, 10);
    */
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    
    int col = 0;
    keyboard_state_activate();
    while(true){
        char c = '\0';
        get_keyboard_buffer(&c);
        if (c != '\0'){
            framebuffer_write(0, col++, c, 0xF, 0);
        }
    }
}
