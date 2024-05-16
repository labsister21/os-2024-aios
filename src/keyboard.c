#include "header/driver/keyboard.h"
#include "header/text/framebuffer.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"

static struct KeyboardDriverState keyboard_state = {
    false, false, true, false, false, (char) 0
};

const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

void keyboard_state_activate(void){
    keyboard_state.keyboard_input_on = true;
}

void keyboard_state_deactivate(void){
    keyboard_state.keyboard_input_on = false;
}

void get_keyboard_buffer(char *buf){
    if(keyboard_state.keyboard_input_on && !keyboard_state.isBufferEmpty){
        *buf = keyboard_state.keyboard_buffer;
        keyboard_state.isBufferEmpty = true;
        keyboard_state.keyboard_buffer = (char) 0;
    }
}

void keyboard_isr(void){
    pic_ack(IRQ_KEYBOARD + PIC1_OFFSET);
    io_wait();
    uint8_t scancode = in(KEYBOARD_DATA_PORT);
    if(keyboard_state.keyboard_input_on && keyboard_state.isBufferEmpty){
        keyboard_state.keyboard_buffer = keyboard_scancode_1_to_ascii_map[scancode];
        keyboard_state.isBufferEmpty = false;
        return;
    }
}
void keyboard_do_something(int *row, int* col, char c){
    if (c != '\0'){
        if (c == '\n'){
            (*row)++,  *col=0;
            framebuffer_write(*row, *col, '\0', 0xF, 0);
            framebuffer_set_cursor(*row, *col);
        } else if (c == '\t'){
            *col += 4;
            if(*col>=80){
                (*row)++;
                *col = 0;
            }
            framebuffer_write(*row, *col, '\0', 0xF, 0);
            framebuffer_set_cursor(*row, *col);
        } else if (c == '\b'){
            if(!(*col == 0 && *row == 0)){
                (*col)--;
                if (*col < 0){
                    (*row)--;
                    (*col) = 79;
                    while(framebuffer_read(*row,*col) == '\0' && *col != 0){
                        (*col)--;
                    }
                    if (*row <= 0){
                        *row = 0;
                    }
                } 
                framebuffer_write(*row, *col, '\0', 0xF, 0);
                framebuffer_set_cursor(*row, *col);
            }        
        }
        else{
            framebuffer_write(*row, *col, c, 0xF, 0);
            (*col)++;
            if(*col==80){
                (*row)++;
                *col = 0;
            }
            framebuffer_write(*row, *col, '\0', 0xF, 0);
            framebuffer_set_cursor(*row, *col);
        }
    }
}
