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

    int col = 0;
    int row = 0;
    keyboard_state_activate();
    framebuffer_write(row, col, '\0', 0xF, 0);
    framebuffer_set_cursor(0,0);
    while(true){
        char c = '\0';
        get_keyboard_buffer(&c);
        if (c != '\0'){
            if (c == '\n'){
                row++, col=0;
                framebuffer_write(row, col, '\0', 0xF, 0);
                framebuffer_set_cursor(row, col);
            } else if (c == '\t'){
                col += 4;
                if(col>=80){
                    row++;
                    col = 0;
                }
                framebuffer_write(row, col, '\0', 0xF, 0);
                framebuffer_set_cursor(row, col);
            } else if (c == '\b'){
                if(!(col == 0 && row == 0)){
                    col--;
                    if (col < 0){
                        row--;
                        col = 79;
                        while(framebuffer_read(row,col) == '\0' && col != 0){
                            col--;
                        }
                        if (row <= 0){
                            row = 0;
                        }
                    } 
                    framebuffer_write(row, col, '\0', 0xF, 0);
                    framebuffer_set_cursor(row, col);
                }        
            }
            else{
                framebuffer_write(row, col, c, 0xF, 0);
                col++;
                if(col==80){
                    row++;
                    col = 0;
                }
                framebuffer_write(row, col, '\0', 0xF, 0);
                framebuffer_set_cursor(row, col);
            }
        }
    }
}