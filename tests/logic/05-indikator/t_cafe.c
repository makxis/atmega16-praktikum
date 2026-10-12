/* Глава 5, задание 3 (cafe.c): слово CAFE, а с 0xBEEF - bEEF. */
#include "sim.h"
#include "05-indikator/cafe.c"
int main(void){
  sim_start();
  int ok = 1;
  for(int d = 0; d < 16; d++) if(seg2dig((unsigned char)kod7[d]) != d) ok = 0;
  CHECK(ok, "kod7[0..15]: цифры 0..9 и буквы A b C d E F");
  CHECK(disp_digit(0) == 0xC && disp_digit(1) == 0xA && disp_digit(2) == 0xF && disp_digit(3) == 0xE, "на индикаторе C A F E");
  slovo = 0xBEEF; prog_main(); T0(4);   /* как если бы в программе написали 0xBEEF */
  CHECK(disp_digit(0) == 0xB && disp_digit(1) == 0xE && disp_digit(2) == 0xE && disp_digit(3) == 0xF, "0xBEEF: b E E F");
  return sim_done("05/cafe");
}
