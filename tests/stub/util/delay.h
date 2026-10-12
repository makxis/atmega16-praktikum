/* Заглушка <util/delay.h>: задержка = модельное время (таймеры продолжают считать), см. tests/sim.h */
#ifndef SIM_UTIL_DELAY_H
#define SIM_UTIL_DELAY_H
void _delay_ms(double ms);
void _delay_us(double us);
#endif
