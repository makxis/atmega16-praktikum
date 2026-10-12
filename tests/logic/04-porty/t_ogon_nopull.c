/* Мутант ogon.c без подтяжки (tests/mutants/04-ogon-nopull.json): PORTC=0x00.
   Отпущенная кнопка не подтянута, вход "висит" (в модели читается 0) - огонь бежит назад сам. */
#include "sim.h"
#include "04-porty/ogon__nopull.c"
static int seq[8], nseq = 0;
static int lit(void){
  int k = -1, cnt = 0;
  for(int i = 0; i < 4; i++) if(pin_out(PIN('B', 4 + i)) == 0){ k = i; cnt++; }
  return cnt == 1 ? k : -1;
}
static void hook(int k, double ms){ (void)k; (void)ms; if(nseq < 8) seq[nseq++] = lit(); }
int main(void){
  sim_loop(4, hook);
  CHECK(!pin_pullup(PIN('C', 1)), "в мутанте подтяжки на PC1 нет");
  CHECK(seq[0] == 0 && seq[1] == 3 && seq[2] == 2, "мутант пойман: кнопку не трогали, а огонь бежит назад");
  return sim_done("04/ogon мутант без подтяжки");
}
