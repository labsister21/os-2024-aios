#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    uint16_t pos = r * 80 + c;
	out(0x3D4, 0x0F);
	out(0x3D5, (uint8_t) (pos & 0xFF));
	out(0x3D4, 0x0E);
	out(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
     uint16_t pos = row * 80 + col;
     FRAMEBUFFER_MEMORY_OFFSET[pos * 2] = c;
     FRAMEBUFFER_MEMORY_OFFSET[pos * 2 + 1] = (bg << 4) | (fg & 0x0F);
}

void framebuffer_clear(void) {
    memset(FRAMEBUFFER_MEMORY_OFFSET, 0x7000, sizeof(uint8_t)*2000);
}
