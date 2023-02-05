#include "dispDirver.h"
#include "pico/stdlib.h"

#define WR 0
#define RD 1
#define D0 2
#define D_NUM 8


// initialize the pio state machine rd pin is wr+1
void initPIO(PIO pio, uint sm, uint wr, uint d0, const pio_program_t *program, void (*init_fun)(PIO, uint, uint, uint, uint, float)){
    static const float pio_freq = 100000; //100000000;
    uint offset = pio_add_program(pio, program);
    float div = (float)clock_get_hz(clk_sys) / pio_freq;
    init_fun(pio, sm, offset, wr, d0, div);
    pio_sm_set_enabled(pio, sm, true);
    // send 0b01111111 and 0b11111111 into the fifo for the pindirs stuff
    pio_sm_put_blocking(pio, sm, 0b01111111);
    pio_sm_put_blocking(pio, sm, 0b11111111);

    pio_sm_put_blocking(pio, sm, 'a');
    pio_sm_put_blocking(pio, sm, 'b');
    pio_sm_put_blocking(pio, sm, 'c');


}

// Sends a char to the display by just dumping the bits
void sendByte(uint8_t data) {
    gpio_put(WR, 0);
    sleep_us(1);
    for (int i = 0; i < D_NUM; i++) {
        gpio_put(D0 + i, data & 1);
        data >>= 1;
    }
    gpio_put(WR, 1);
    // read the busy flag and wait until it is ready
    gpio_put(RD, 0);
    sleep_us(1);
    gpio_set_dir(D0 + D_NUM - 1, GPIO_IN);
    while (gpio_get(D0 + D_NUM - 1)) {
        sleep_us(1);
    }
    gpio_set_dir(D0 + D_NUM - 1, GPIO_OUT);
    gpio_put(RD, 1);
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