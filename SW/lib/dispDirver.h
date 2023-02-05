#ifndef TTT_H_
#define TTT_H_
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

void initPIO(PIO pio, uint sm, uint wr, uint d0, const pio_program_t *program, void (*init_fun)(PIO, uint, uint, uint, uint, float));
void sendByte(uint8_t data);
void sendString(const char *str);
void sendStringDelay(const char *str, uint8_t delay);
void displayReset(void);
void scrollText(uint8_t amount, uint8_t delay);
void showCursor(bool show);
void selectWindow(uint8_t window);

#endif