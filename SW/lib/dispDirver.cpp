#include "dispDirver.h"
#include "pico/stdlib.h"

PIO drv_pio;
uint drv_sm;

// initialize the pio state machine rd pin is wr+1
void initPIO(PIO pio, uint sm, uint wr, uint d0, const pio_program_t *program, void (*init_fun)(PIO, uint, uint, uint, uint, float)){
    static const float pio_freq = 20000000;
    uint offset = pio_add_program(pio, program);
    float div = (float)clock_get_hz(clk_sys) / pio_freq;
    init_fun(pio, sm, offset, wr, d0, div);
    pio_sm_set_enabled(pio, sm, true);
    drv_pio = pio;
    drv_sm = sm;
    // send 0b01111111 and 0b11111111 into the fifo for the pindirs stuff
    pio_sm_put_blocking(pio, sm, 0b01111111);
    pio_sm_put_blocking(pio, sm, 0b11111111);
}

// Sends a char to the display by just dumping the bits
void sendByte(uint8_t data) {
    pio_sm_put_blocking(drv_pio, drv_sm, data);
}

// Sends a string to the display by just dumping the bytes
void sendString(const char *str) {
    while (*str) {
        sendByte(*str++);
    }
}

// Sends a string to the display but wait between each byte
void sendStringDelay(const char *str, uint8_t delay) {
    while (*str) {
        sendByte(*str++);
        sleep_ms(delay);
    }
}

// Resets the display
void displayReset(void) {
    sendByte(0x0C);
}

// Scrolls the text on the display by enabling scrolling and then sending spaces (I didn't figure out how to turn it off lmao)
void scrollText(uint8_t amount, uint8_t delay) {
    sendByte(0x1F);
    sendByte(0x03);
    for (int i = 0; i < amount; i++) {
        sleep_ms(delay);
        sendByte(' ');
    }
}

// Shows or hides the internal cursor
void showCursor(bool show) {
    sendByte(0x1F);
    sendByte(0x43);
    sendByte(show ? 0x01 : 0x00);
}

// Selects the window of the display
// 0 - base window
// 1 - 4: User-Windows
void selectWindow(uint8_t window) {
    sendByte(0x1F);
    sendByte(0x28);
    sendByte(0x77);
    sendByte(0x01);
    sendByte(window);
}

// sets the cursor position
// x - 1 dot pet unit
// y - 8 dots per unit
void setCursor(uint16_t x, uint16_t y){
    sendByte(0x1F);
    sendByte(0x24);
    // send lower 8 bits of x
    sendByte(x & 0xFF);
    // send upper 8 bits of x
    sendByte((x >> 8) & 0xFF);
    // send lower 8 bits of y
    sendByte(y & 0xFF);
    // send upper 8 bits of y
    sendByte((y >> 8) & 0xFF);
}

// Sets the brightness of the display
// 1 - 12.5%
// 2 - 25%
// 3 - 37.5%
// 4 - 50%  
// 5 - 62.5%
// 6 - 75%
// 7 - 87.5%
// 8 - 100%
void setBrightness(uint8_t brightness){
    sendByte(0x1F);
    sendByte(0x58);
    sendByte(brightness);
}



// Draws a bitmap to the display
// x - 1 dot per unit cursor position
// y - 8 dots per unit cursor position
// width - 1 dot per unit width of the bitmap
// height - 8 dots per unit height of the bitmap
void drawBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *bitmap){
    setCursor(x, y);
    sendByte(0x1F);
    sendByte(0x28);
    sendByte(0x66);
    sendByte(0x11);
    // send lower 8 bits of x
    sendByte(width & 0xFF);
    // send upper 8 bits of x
    sendByte((width >> 8) & 0xFF);
    // send lower 8 bits of y
    sendByte(height & 0xFF);
    // send upper 8 bits of y
    sendByte((height >> 8) & 0xFF);

    sendByte(0x01);

    // send the bitmap based on it's size
    for (int i = 0; i < width; i++) {
        // send the lower 8 bits of the bitmap
        sendByte(reverseByte(bitmap[i] & 0xFF));
        // send the upper 8 bits of the bitmap if we are sending a 16 bit bitmap
        if (height > 1)
            sendByte(reverseByte((bitmap[i] >> 8) & 0xFF));
    }
}

unsigned char reverseByte(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}