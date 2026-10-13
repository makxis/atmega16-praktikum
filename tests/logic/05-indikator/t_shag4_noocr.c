/* Мутант shag4.c без OCR1A=31249 (tests/mutants/05-shag4-noocr.json): та же ошибка, что в Shasi.c (задание 1 главы 8). */
#include "sim.h"
#include "05-indikator/shag4__noocr.c"
int main(void){
  sim_start();
  isr_calls_reset(); run_ms(1000);
  long n = isr_calls("TIMER1_COMPA");
  CHECK(n > 30000, "мутант пойман: без OCR1A прерывание Timer1 идёт ~31250 раз в секунду, а не 1");
  return sim_done("05/shag4 мутант без OCR1A");
}
