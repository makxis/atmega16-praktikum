/* Мутант kbd_calc.c с масками без "| 0x0C" и PORTB=0xF0 (tests/mutants/10-kbd_calc-pullup.json):
   развёртка индикатора пишет 0 в биты PB2, PB3 и выключает подтяжку строк. Строки висят = 0,
   программа видит "нажатую" клавишу первой строки, настоящие цифры не вводятся. */
#include "sim.h"
#include "10-klaviatura/kbd_calc__pullup.c"
static const int R[4] = {PIN('B', 2), PIN('B', 3), PIN('C', 0), PIN('C', 1)};
static const int C[4] = {PIN('B', 4), PIN('B', 5), PIN('B', 6), PIN('B', 7)};
int main(void){
  kbd_pins(R, C);
  sim_start();
  T0(40);
  CHECK(!pin_pullup(R[0]) && !pin_pullup(R[1]), "мутант пойман: подтяжки на PB2, PB3 нет");
  kbd_tap(1, 1, 50);                                    /* 5 */
  kbd_tap(2, 2, 50);                                    /* 9 */
  CHECK(disp_num() != 59, "мутант пойман: 5, 9 не вводятся");
  CHECK(last == 10, "программа всё время видит нажатой A (СТР1 висит = 0 в каждом столбце)");
  return sim_done("10/kbd_calc мутант без | 0x0C");
}
