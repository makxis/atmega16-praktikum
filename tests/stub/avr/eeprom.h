/* Заглушка <avr/eeprom.h>: EEPROM - массив в памяти ПК (512 байт, после "стирания" 0xFF) */
#ifndef SIM_AVR_EEPROM_H
#define SIM_AVR_EEPROM_H
#include <stdint.h>
#define EEMEM
extern uint8_t sim_eeprom[512];
static inline uint8_t eeprom_read_byte(const uint8_t *p){ return *(const uint8_t *)p; }
static inline void eeprom_write_byte(uint8_t *p, uint8_t v){ *p = v; }
static inline void eeprom_update_byte(uint8_t *p, uint8_t v){ *p = v; }
static inline uint16_t eeprom_read_word(const uint16_t *p){ return *p; }
static inline void eeprom_write_word(uint16_t *p, uint16_t v){ *p = v; }
static inline void eeprom_busy_wait(void){}
#endif
