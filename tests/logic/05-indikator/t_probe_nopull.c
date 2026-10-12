/* Мутант probe.c без подтяжки (tests/mutants/05-probe-nopull.json): вход PC1 "висит",
   в модели читается 0 - щуп сам перебирает биты, будто кнопку держат. */
#include "sim.h"
#include "05-indikator/probe__nopull.c"
int main(void){
  sim_start();
  T0(1000);
  CHECK(nbit != 0, "мутант пойман: кнопку не трогали, а щуп ушёл с PD0");
  return sim_done("05/probe мутант без подтяжки");
}
