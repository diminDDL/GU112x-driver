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

// function that allows us to use the BOOTSEL button as user input
bool __no_inline_not_in_flash_func(get_bootsel_button)();


int main() {
    static const uint wr_pin = 0;
    static const uint d0_pin = 2;
    initPIO(pio0, 0, wr_pin, d0_pin, &driver_program, disp_driver_program_init);
    // Initialize chosen serial port
    stdio_init_all();

    uint16_t bitmap[112];
    // fill bitmap with 0
    for (int i = 0; i < 112; i++) {
        bitmap[i] = 0;
    }
    
    selectWindow(0);
    setBrightness(8);
    while(true){
        drawBitmap(0, 0, 16*7, 2, bitmap);
        sleep_ms(1);
        printf("%c", 'A');
        for (int i = 0; i < 112; i++) {
            int16_t c1;
            int16_t c2;
            c1 = getchar_timeout_us(10000);
            c2 = getchar_timeout_us(10000);
            if (c1 == PICO_ERROR_TIMEOUT || c2 == PICO_ERROR_TIMEOUT) {
                break;
            }
            bitmap[i] = 0;
            bitmap[i] |= (char)c1;
            bitmap[i] |= (char)c2 << 8;
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
