/* Мутант shag3.c без паузы после изменения кнопки (tests/mutants/09-shag3-nopause.json):
   тест ловит его, если дребезг даёт лишние нажатия. */
#include "sim.h"
#include "09-knopka/shag3__nopause.c"
#define BTN PIN('C', 1)
int main(void){
  sim_start();
  btn_tap(BTN);
  CHECK(t1 > 1, "мутант пойман: без паузы одно нажатие с дребезгом даёт больше +1");
  return sim_done("09/shag3 мутант без паузы");
}
