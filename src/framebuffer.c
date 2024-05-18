#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"

struct FramebufferState framebuffer_current_state = {
    .row = 0,
    .col = 0
};

void framebuffer_set_cursor(uint8_t r, uint8_t c){
    uint16_t pos = r * 80 + c;
	out(0x3D4, 0x0F);
	out(0x3D5, (uint8_t) (pos & 0xFF));
	out(0x3D4, 0x0E);
	out(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
    framebuffer_current_state.row = r;
    framebuffer_current_state.col = c;
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg){
    uint16_t pos = row * 80 + col;
    FRAMEBUFFER_MEMORY_OFFSET[pos * 2] = c;
    FRAMEBUFFER_MEMORY_OFFSET[pos * 2 + 1] = (bg << 4) | (fg & 0x0F);
}

void framebuffer_clear(void){
    memset(FRAMEBUFFER_MEMORY_OFFSET, 0, sizeof(uint8_t)*120*80);
}

char framebuffer_read(uint8_t row, uint8_t col){
    uint16_t pos = row * 80 + col;
    return FRAMEBUFFER_MEMORY_OFFSET[pos * 2];
}


void go_scroll()
{
    memcpy(FRAMEBUFFER_MEMORY, FRAMEBUFFER_MEMORY + 80 * 2, 80 * 2 * 25 - 80 * 2);
    framebuffer_current_state.row--;
    for (int i = 0; i < 80; i++)
    {
        framebuffer_write(framebuffer_current_state.row, i, '\0', 0xF, 0);
    }
}

void puts_char(char c, uint32_t color) {
    if (c != '\0'){
        if (c == '\n'){
            (framebuffer_current_state.row)++,  framebuffer_current_state.col=0;
            if(framebuffer_current_state.row == 25){
                go_scroll();
            }
            framebuffer_write(framebuffer_current_state.row, framebuffer_current_state.col, '\0', color, 0);
            framebuffer_set_cursor(framebuffer_current_state.row, framebuffer_current_state.col);
        } else if (c == '\t'){
            framebuffer_current_state.col += 4;
            if(framebuffer_current_state.col>=80){
                (framebuffer_current_state.row)++;
                if(framebuffer_current_state.row == 25){
                    go_scroll();
                }
                framebuffer_current_state.col = 0;
            }
            framebuffer_write(framebuffer_current_state.row, framebuffer_current_state.col, '\0', color, 0);
            framebuffer_set_cursor(framebuffer_current_state.row, framebuffer_current_state.col);
        } else if (c == '\b'){
            if(!(framebuffer_current_state.col == 0 && framebuffer_current_state.row == 0)){
                (framebuffer_current_state.col)--;
                if (framebuffer_current_state.col < 0){
                    (framebuffer_current_state.row)--;
                    (framebuffer_current_state.col) = 79;
                    while(framebuffer_read(framebuffer_current_state.row,framebuffer_current_state.col) == '\0' && framebuffer_current_state.col != 0){
                        (framebuffer_current_state.col)--;
                    }
                    if (framebuffer_current_state.row <= 0){
                        framebuffer_current_state.row = 0;
                    }
                } 
                framebuffer_write(framebuffer_current_state.row, framebuffer_current_state.col, '\0', color, 0);
                framebuffer_set_cursor(framebuffer_current_state.row, framebuffer_current_state.col);
            }        
        }
        else{
            framebuffer_write(framebuffer_current_state.row, framebuffer_current_state.col, c, color, 0);
            (framebuffer_current_state.col)++;
            if(framebuffer_current_state.col==80){
                (framebuffer_current_state.row)++;
                if(framebuffer_current_state.row == 25){
                    go_scroll();
                }
                framebuffer_current_state.col = 0;
            }
            framebuffer_write(framebuffer_current_state.row, framebuffer_current_state.col, '\0', color, 0);
            framebuffer_set_cursor(framebuffer_current_state.row, framebuffer_current_state.col);
        }
    }
}


void puts(const char *str, uint32_t count, uint32_t color){
    for (uint32_t i=0; i<count; i++) {
        if (str[i] == '\0')
            break;
        puts_char(str[i],color);
    }
    framebuffer_set_cursor(framebuffer_current_state.row,framebuffer_current_state.col);
}