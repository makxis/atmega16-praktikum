/* Заглушка <avr/io.h> для проверки логики на ПК: регистры ATmega16 - обычные переменные
   (определены в tests/sim.h), PINx вычисляются моделью выводов (sim_pin).
   Номера битов совпадают с avr-libc iom16.h (сверено: tests/stub/check_bits.py). */
#ifndef SIM_AVR_IO_H
#define SIM_AVR_IO_H
#include <stdint.h>

extern volatile uint8_t PORTA, PORTB, PORTC, PORTD, DDRA, DDRB, DDRC, DDRD;
extern volatile uint8_t SREG, SPL, SPH, SFIOR, MCUCR, MCUCSR, GICR, GIFR, TIMSK, TIFR;
extern volatile uint8_t TCCR0, TCNT0, OCR0, TCCR1A, TCCR1B, TCCR2, TCNT2, OCR2, ASSR;
extern volatile uint16_t TCNT1, OCR1A, OCR1B, ICR1;
extern volatile uint8_t ACSR, ADMUX, ADCSRA, ADCL, ADCH, WDTCR, OSCCAL, OCDR;
extern volatile uint8_t UDR, UCSRA, UCSRB, UCSRC, UBRRL, UBRRH, SPCR, SPSR, SPDR;
extern volatile uint8_t TWBR, TWSR, TWAR, TWDR, TWCR, EEARL, EEARH, EEDR, EECR, SPMCR;
extern volatile uint16_t ADC, ADCW, EEAR;

uint8_t sim_pin(int port);              /* уровни на выводах порта (0=A ... 3=D) */
#define PINA (sim_pin(0))
#define PINB (sim_pin(1))
#define PINC (sim_pin(2))
#define PIND (sim_pin(3))

#define _BV(bit) (1 << (bit))
#define bit_is_set(sfr, bit)   ((sfr) & _BV(bit))
#define bit_is_clear(sfr, bit) (!((sfr) & _BV(bit)))

/* выводы портов */
#define PA0 0
#define PA1 1
#define PA2 2
#define PA3 3
#define PA4 4
#define PA5 5
#define PA6 6
#define PA7 7
#define PB0 0
#define PB1 1
#define PB2 2
#define PB3 3
#define PB4 4
#define PB5 5
#define PB6 6
#define PB7 7
#define PC0 0
#define PC1 1
#define PC2 2
#define PC3 3
#define PC4 4
#define PC5 5
#define PC6 6
#define PC7 7
#define PD0 0
#define PD1 1
#define PD2 2
#define PD3 3
#define PD4 4
#define PD5 5
#define PD6 6
#define PD7 7
#define PORTA0 0
#define PORTA1 1
#define PORTA2 2
#define PORTA3 3
#define PORTA4 4
#define PORTA5 5
#define PORTA6 6
#define PORTA7 7
#define PORTB0 0
#define PORTB1 1
#define PORTB2 2
#define PORTB3 3
#define PORTB4 4
#define PORTB5 5
#define PORTB6 6
#define PORTB7 7
#define PORTC0 0
#define PORTC1 1
#define PORTC2 2
#define PORTC3 3
#define PORTC4 4
#define PORTC5 5
#define PORTC6 6
#define PORTC7 7
#define PORTD0 0
#define PORTD1 1
#define PORTD2 2
#define PORTD3 3
#define PORTD4 4
#define PORTD5 5
#define PORTD6 6
#define PORTD7 7
#define DDA0 0
#define DDA1 1
#define DDA2 2
#define DDA3 3
#define DDA4 4
#define DDA5 5
#define DDA6 6
#define DDA7 7
#define DDB0 0
#define DDB1 1
#define DDB2 2
#define DDB3 3
#define DDB4 4
#define DDB5 5
#define DDB6 6
#define DDB7 7
#define DDC0 0
#define DDC1 1
#define DDC2 2
#define DDC3 3
#define DDC4 4
#define DDC5 5
#define DDC6 6
#define DDC7 7
#define DDD0 0
#define DDD1 1
#define DDD2 2
#define DDD3 3
#define DDD4 4
#define DDD5 5
#define DDD6 6
#define DDD7 7
#define PINA0 0
#define PINA1 1
#define PINA2 2
#define PINA3 3
#define PINA4 4
#define PINA5 5
#define PINA6 6
#define PINA7 7
#define PINB0 0
#define PINB1 1
#define PINB2 2
#define PINB3 3
#define PINB4 4
#define PINB5 5
#define PINB6 6
#define PINB7 7
#define PINC0 0
#define PINC1 1
#define PINC2 2
#define PINC3 3
#define PINC4 4
#define PINC5 5
#define PINC6 6
#define PINC7 7
#define PIND0 0
#define PIND1 1
#define PIND2 2
#define PIND3 3
#define PIND4 4
#define PIND5 5
#define PIND6 6
#define PIND7 7

/* SREG */
#define SREG_C 0
#define SREG_Z 1
#define SREG_N 2
#define SREG_V 3
#define SREG_S 4
#define SREG_H 5
#define SREG_T 6
#define SREG_I 7

/* SFIOR */
#define PSR10 0
#define PSR2  1
#define PUD   2
#define ACME  3
#define ADTS0 5
#define ADTS1 6
#define ADTS2 7

/* MCUCR */
#define ISC00 0
#define ISC01 1
#define ISC10 2
#define ISC11 3
#define SM0   4
#define SM1   5
#define SE    6
#define SM2   7

/* MCUCSR */
#define PORF  0
#define EXTRF 1
#define BORF  2
#define WDRF  3
#define JTRF  4
#define ISC2  6
#define JTD   7

/* GICR и GIFR */
#define IVCE  0
#define IVSEL 1
#define INT2  5
#define INT0  6
#define INT1  7
#define INTF2 5
#define INTF0 6
#define INTF1 7

/* TIMSK и TIFR */
#define TOIE0  0
#define OCIE0  1
#define TOIE1  2
#define OCIE1B 3
#define OCIE1A 4
#define TICIE1 5
#define TOIE2  6
#define OCIE2  7
#define TOV0   0
#define OCF0   1
#define TOV1   2
#define OCF1B  3
#define OCF1A  4
#define ICF1   5
#define TOV2   6
#define OCF2   7

/* TCCR0 */
#define CS00  0
#define CS01  1
#define CS02  2
#define WGM01 3
#define COM00 4
#define COM01 5
#define WGM00 6
#define FOC0  7

/* TCCR1A */
#define WGM10  0
#define WGM11  1
#define FOC1B  2
#define FOC1A  3
#define COM1B0 4
#define COM1B1 5
#define COM1A0 6
#define COM1A1 7

/* TCCR1B */
#define CS10  0
#define CS11  1
#define CS12  2
#define WGM12 3
#define WGM13 4
#define ICES1 6
#define ICNC1 7

/* TCCR2 */
#define CS20  0
#define CS21  1
#define CS22  2
#define WGM21 3
#define COM20 4
#define COM21 5
#define WGM20 6
#define FOC2  7

/* ASSR */
#define TCR2UB 0
#define OCR2UB 1
#define TCN2UB 2
#define AS2    3

/* ACSR */
#define ACIS0 0
#define ACIS1 1
#define ACIC  2
#define ACIE  3
#define ACI   4
#define ACO   5
#define ACBG  6
#define ACD   7

/* ADMUX и ADCSRA */
#define MUX0  0
#define MUX1  1
#define MUX2  2
#define MUX3  3
#define MUX4  4
#define ADLAR 5
#define REFS0 6
#define REFS1 7
#define ADPS0 0
#define ADPS1 1
#define ADPS2 2
#define ADIE  3
#define ADIF  4
#define ADATE 5
#define ADSC  6
#define ADEN  7

/* WDTCR */
#define WDP0 0
#define WDP1 1
#define WDP2 2
#define WDE  3
#define WDTOE 4

/* UCSRA, UCSRB, UCSRC */
#define MPCM 0
#define U2X  1
#define PE   2
#define DOR  3
#define FE   4
#define UDRE 5
#define TXC  6
#define RXC  7
#define TXB8 0
#define RXB8 1
#define UCSZ2 2
#define TXEN 3
#define RXEN 4
#define UDRIE 5
#define TXCIE 6
#define RXCIE 7
#define UCPOL 0
#define UCSZ0 1
#define UCSZ1 2
#define USBS 3
#define UPM0 4
#define UPM1 5
#define UMSEL 6
#define URSEL 7

/* SPCR и SPSR */
#define SPR0 0
#define SPR1 1
#define CPHA 2
#define CPOL 3
#define MSTR 4
#define DORD 5
#define SPE  6
#define SPIE 7
#define SPI2X 0
#define WCOL 6
#define SPIF 7

/* EECR */
#define EERE  0
#define EEWE  1
#define EEMWE 2
#define EERIE 3

#endif
