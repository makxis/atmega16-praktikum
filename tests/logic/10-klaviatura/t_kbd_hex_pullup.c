/* Мутант kbd_hex.c с PORTB=0xE0 вместо 0xEC (tests/mutants/10-kbd_hex-pullup.json): нули в битах PB2, PB3
   выключают подтяжку строк СТР1, СТР2. Строки "висят" и читаются как 0: клавиша "нажимается" сама. */
#include "sim.h"
#include "10-klaviatura/kbd_hex__pullup.c"
static const int R[4] = {PIN('B', 2), PIN('B', 3), PIN('C', 0), PIN('C', 1)};
static const int C[4] = {PIN('B', 5), PIN('B', 6), PIN('B', 7), PIN('D', 7)};
int main(void){
  kbd_pins(R, C);
  disp_digits(PIN('B', 4), -1, -1, -1);
  sim_start();
  T0(100);
  CHECK(!pin_pullup(R[0]) && !pin_pullup(R[1]), "мутант пойман: подтяжки на PB2, PB3 нет");
  CHECK(disp_digit(0) != -2, "мутант пойман: никто ничего не нажимал, а на индикаторе не прочерк");
  CHECK(disp_digit(0) == 10, "висящая СТР1 читается в каждом столбце: последней видна A (СТР1, СТЛ4)");
  kbd_tap(2, 1, 50);                                    /* 8 */
  CHECK(disp_digit(0) != 8, "мутант пойман: клавиша 8 не читается");
  return sim_done("10/kbd_hex мутант PORTB=0xE0");
}
