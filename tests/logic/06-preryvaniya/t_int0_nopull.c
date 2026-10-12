/* Мутант int0.c без подтяжки PD2 (tests/mutants/06-int0-nopull.json): PORTD=0x00.
   Отпущенная кнопка не подтянута, вход "висит" (в модели читается 0): спада при нажатии нет. */
#include "sim.h"
#include "06-preryvaniya/int0__nopull.c"
#define BTN PIN('D', 2)
static void hook(int k, double ms){
  (void)ms;
  if(k == 1) btn_down(BTN);
  if(k == 2) btn_up(BTN);
  if(k == 3) btn_down(BTN);
  if(k == 4) btn_up(BTN);
}
int main(void){
  sim_loop(6, hook);
  CHECK(!pin_pullup(BTN), "мутант: подтяжки на PD2 нет");
  CHECK(isr_calls("INT0") == 0 && pin_out(PIN('B', 4)) == 1, "мутант ловится: без подтяжки нажатия не видны");
  return sim_done("06/int0__nopull (мутант)");
}
