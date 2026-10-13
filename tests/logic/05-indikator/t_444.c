/* Глава 5, шаг 1 (444.c): стартовая программа как есть. */
#include "sim.h"
#include "05-indikator/444.c"
int main(void){
  sim_start();
  CHECK(DDRB == 0xF0 && DDRD == 0xFF, "PB4..PB7 и весь порт D - выходы");
  CHECK(ACSR & (1 << ACD), "компаратор выключен (ACD = 1)");
  CHECK(disp_digit(0) == 3 && disp_digit(1) == 2 && disp_digit(2) == 1 && disp_digit(3) == 0,
        "на индикаторе 3 2 1 0 (разряды Ind[0]..Ind[3] на PB4..PB7)");
  CHECK(OCR1A == 31249, "OCR1A=31249");
  isr_calls_reset(); run_ms(1000);
  CHECK(isr_calls("TIMER1_COMPA") == 1, "1 с модельного времени: ровно одно прерывание Timer1");
  long n0 = isr_calls("TIMER0_OVF");
  CHECK(n0 >= 487 && n0 <= 489, "1 с: ~488 переполнений Timer0 (каждые 2,048 мс)");
  CHECK(disp_digit(0) == 3 && disp_digit(3) == 0, "обработчик Timer1 пустой: через секунду на индикаторе то же");
  int ok = 1;                           /* развёртка: за вызов Timer0 - один разряд, по кругу */
  for(int i = 0; i < 8; i++){
    unsigned char idx = i1;
    T0(1);
    if(PORTD != (unsigned char)Ind[idx] || PORTB != (unsigned char)Mas[idx]) ok = 0;
  }
  CHECK(ok, "развёртка: после каждого вызова Timer0 PORTD = Ind[i1], PORTB = Mas[i1], i1 по кругу 0..3");
  return sim_done("05/444");
}
