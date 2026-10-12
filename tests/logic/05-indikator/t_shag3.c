/* Глава 5, шаг 3 (shag3.c): коды 4..9 дописаны, на индикаторе 4567. */
#include "sim.h"
#include "05-indikator/shag3.c"
int main(void){
  sim_start();
  CHECK(disp_digit(0) == 4 && disp_digit(1) == 5 && disp_digit(2) == 6 && disp_digit(3) == 7, "на индикаторе 4567");
  int ok = 1;
  for(int d = 0; d < 10; d++) if(seg2dig((unsigned char)kod7[d]) != d) ok = 0;
  CHECK(ok, "kod7[0..9] - коды цифр 0..9 (a = PD0 ... g = PD6, 1 = горит)");
  return sim_done("05/shag3");
}
