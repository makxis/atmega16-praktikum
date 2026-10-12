/* Заглушка <avr/interrupt.h>: обработчик - обычная функция, её вызывает tests/sim.h.
   sei()/cli() меняют бит I в SREG (модель смотрит на него перед вызовом обработчика). */
#ifndef SIM_AVR_INTERRUPT_H
#define SIM_AVR_INTERRUPT_H
#include <avr/io.h>
#define ISR(vector, ...) void vector(void)
#define sei() (SREG |= 0x80)
#define cli() (SREG &= (uint8_t)~0x80)
#define reti() return
#define ISR_BLOCK
#define ISR_NOBLOCK
#define ISR_NAKED
#define EMPTY_INTERRUPT(vector) void vector(void) {}
#endif
