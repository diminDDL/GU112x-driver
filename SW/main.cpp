#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "disp_int.pio.h"
#include "lib/util.h"

#define WR 0
#define RD 1
#define D0 2
#define D_NUM 8

// data write: D0-D7 - data; WR 0->1; RD 1
// status read: D0-D6: Indefinite; D7: PBUSY Flag, 1:BUSY, 0:READY; WR 1; RD 0  

const char *hello = "Hello, world!";
const char *hello2 = "VFD is <3   \r";

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

int main() {

    test();


    const uint led_pin = 25;
    static const float pio_freq = 2000;
    
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &driver_program);
    float div = (float)clock_get_hz(clk_sys) / pio_freq;

    disp_driver_program_init(pio, sm, offset, led_pin, div);

    pio_sm_set_enabled(pio, sm, true);

    // Initialize LED pin
    // gpio_init(led_pin);
    // gpio_set_dir(led_pin, GPIO_OUT);
    // intialize control pins
    gpio_init(WR);
    gpio_set_dir(WR, GPIO_OUT);
    gpio_init(RD);
    gpio_set_dir(RD, GPIO_OUT);
    // intialize data pins
    for (int i = 0; i < D_NUM; i++) {
        gpio_init(D0 + i);
        gpio_set_dir(D0 + i, GPIO_OUT);
    }

    // Initialize chosen serial port
    stdio_init_all();

    // Loop forever
    while (true) {
        selectWindow(0);
        sendString(hello);
        scrollText(16, 100);
        sendStringDelay(hello2, 100);
        sleep_ms(1000);
        displayReset();
    }
    
}