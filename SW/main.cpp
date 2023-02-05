#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "pico/bootrom.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/sync.h"
#include "disp_int.pio.h"
#include "lib/dispDirver.h"

#define WR 0
#define RD 1
#define D0 2
#define D_NUM 8

// data write: D0-D7 - data; WR 0->1; RD 1
// status read: D0-D6: Indefinite; D7: PBUSY Flag, 1:BUSY, 0:READY; WR 1; RD 0  

const char *hello = "Hello, world!";
const char *hello2 = "VFD is <3   \r";

// function that allows us to use the BOOTSEL button as user input
bool __no_inline_not_in_flash_func(get_bootsel_button)();


int main() {
    static const uint led_pin = 25;


    static const uint wr_pin = 0;
    static const uint d0_pin = 2;
    initPIO(pio0, 0, wr_pin, d0_pin, &driver_program, disp_driver_program_init);

    // Initialize LED pin
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
    // intialize control pins

    // Initialize chosen serial port
    stdio_init_all();

    gpio_put(led_pin, 1);

    uint16_t bitmap[] = {
        0b0000111000011000,
        0b0001000111100110,
        0b0001001010001001,
        0b0001001110011001,
        0b0001001011011001,
        0b0001001111000001,
        0b0000101010101001,
        0b0000101110001001,
        0b0000101010011001,
        0b0000100101011001,
        0b0000010011000001,
        0b0000010001010001,
        0b0000001000110001,
        0b0000000101100110,
        0b0000000010001000,
        0b0000000001110000,

        0b0000111000011000,
        0b0001000111100110,
        0b0001001010001001,
        0b0001001110011001,
        0b0001001011011001,
        0b0001001111000001,
        0b0000101010101001,
        0b0000101110001001,
        0b0000101010011001,
        0b0000100101011001,
        0b0000010011000001,
        0b0000010001010001,
        0b0000001000110001,
        0b0000000101100110,
        0b0000000010001000,
        0b0000000001110000,

        0b0000111000011000,
        0b0001000111100110,
        0b0001001010001001,
        0b0001001110011001,
        0b0001001011011001,
        0b0001001111000001,
        0b0000101010101001,
        0b0000101110001001,
        0b0000101010011001,
        0b0000100101011001,
        0b0000010011000001,
        0b0000010001010001,
        0b0000001000110001,
        0b0000000101100110,
        0b0000000010001000,
        0b0000000001110000,

        0b0000111000011000,
        0b0001000111100110,
        0b0001001010001001,
        0b0001001110011001,
        0b0001001011011001,
        0b0001001111000001,
        0b0000101010101001,
        0b0000101110001001,
        0b0000101010011001,
        0b0000100101011001,
        0b0000010011000001,
        0b0000010001010001,
        0b0000001000110001,
        0b0000000101100110,
        0b0000000010001000,
        0b0000000001110000,

        0b0000111000011000,
        0b0001000111100110,
        0b0001001010001001,
        0b0001001110011001,
        0b0001001011011001,
        0b0001001111000001,
        0b0000101010101001,
        0b0000101110001001,
        0b0000101010011001,
        0b0000100101011001,
        0b0000010011000001,
        0b0000010001010001,
        0b0000001000110001,
        0b0000000101100110,
        0b0000000010001000,
        0b0000000001110000,

        0b0000111000011000,
        0b0001000111100110,
        0b0001001010001001,
        0b0001001110011001,
        0b0001001011011001,
        0b0001001111000001,
        0b0000101010101001,
        0b0000101110001001,
        0b0000101010011001,
        0b0000100101011001,
        0b0000010011000001,
        0b0000010001010001,
        0b0000001000110001,
        0b0000000101100110,
        0b0000000010001000,
        0b0000000001110000,
    };

    // Loop forever
    while (true) {
        selectWindow(0);
        uint fps = 1111;
        char buff[5];
        uint64_t lastTime = 0;
        while(true){
            drawBitmap(0, 0, 16*5, 2, bitmap);
            fps = 1000000 / (time_us_64() - lastTime);
            lastTime = time_us_64();
            setCursor(80, 0);
            sprintf(buff, "%4d", fps);
            sendString(buff);
        }
        sleep_ms(10000);
        setBrightness(2);
        sendString(hello);
        scrollText(16, 100);
        setCursor(25, 1);
        sendStringDelay(hello2, 100);
        sleep_ms(1000);
        displayReset();
        if (get_bootsel_button()) {
            reset_usb_boot(0,0);
        }
    }

}


bool __no_inline_not_in_flash_func(get_bootsel_button)() {
    const uint CS_PIN_INDEX = 1;
    uint32_t flags = save_and_disable_interrupts();
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);
    for (volatile int i = 0; i < 1000; ++i);
    bool button_state = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);
    restore_interrupts(flags);
    return button_state;
}
