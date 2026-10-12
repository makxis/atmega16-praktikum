/* Глава 5, шаг 2 (shag2.c): "1" только в Ind[0], остальные разряды пустые. */
#include "sim.h"
#include "05-indikator/shag2.c"
int main(void){
  sim_start();
  CHECK(disp_digit(0) == 1, "в разряде Ind[0] (PB4) горит 1");
  CHECK(disp_digit(1) == -1 && disp_digit(2) == -1 && disp_digit(3) == -1, "разряды Ind[1]..Ind[3] пустые (код 0x00)");
  isr_calls_reset(); run_ms(1000);
  CHECK(isr_calls("TIMER1_COMPA") == 1, "Timer1: одно прерывание в секунду (обработчик пока пустой)");
  CHECK(disp_digit(0) == 1, "через секунду то же");
  return sim_done("05/shag2");
}
