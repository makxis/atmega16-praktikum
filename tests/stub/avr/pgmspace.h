/* Заглушка <avr/pgmspace.h>: на ПК флеш и ОЗУ - одна память */
#ifndef SIM_AVR_PGMSPACE_H
#define SIM_AVR_PGMSPACE_H
#include <stdint.h>
#define PROGMEM
#define PSTR(s) (s)
#define pgm_read_byte(p) (*(const uint8_t *)(p))
#define pgm_read_word(p) (*(const uint16_t *)(p))
#endif
