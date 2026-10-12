/* Мутант freq.c с масками без "| 0x04" и PORTB=0xF0 (tests/mutants/11-freq-pullup.json): развёртка
   выключает подтяжку PB2 (та же ловушка, что в варианте Б клавиатуры из главы 10). Сигнал с кнопки или
   с выхода, который отпускает линию, не считается: без подтяжки вход висит и читается 0. */
#include "sim.h"
#include "11-signaly/freq__pullup.c"
#define SIG PIN('B', 2)
int main(void){
  sim_start();
  T0(8);
  CHECK(!pin_pullup(SIG), "мутант пойман: подтяжки на PB2 нет");
  T1(1);
  for(int i = 0; i < 5; i++){ pin_ext(SIG, 0); T0(10); pin_ext(SIG, -1); T0(10); }   /* 5 нажатий без дребезга */
  T1(1);
  CHECK(disp_num() != 5, "мутант пойман: 5 нажатий кнопки на PB2 не насчитаны");
  return sim_done("11/freq мутант без | 0x04");
}
