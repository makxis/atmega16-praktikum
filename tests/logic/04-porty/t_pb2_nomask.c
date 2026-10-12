/* Мутант pb2.c без "| 0x04" (tests/mutants/04-pb2-nomask.json): PORTB=Mas[n] пишет 0 в бит 2
   и выключает подтяжку кнопки PB2. Отпущенная кнопка читается как нажатая, огонь бежит назад сам. */
#include "sim.h"
#include "04-porty/pb2__nomask.c"
static int seq[8], nseq = 0;
static int lit(void){
  int k = -1, cnt = 0;
  for(int i = 0; i < 4; i++) if(pin_out(PIN('B', 4 + i)) == 0){ k = i; cnt++; }
  return cnt == 1 ? k : -1;
}
static void hook(int k, double ms){ (void)k; (void)ms; if(nseq < 8) seq[nseq++] = lit(); }
int main(void){
  sim_loop(4, hook);
  CHECK(!pin_pullup(PIN('B', 2)), "мутант пойман: после PORTB=Mas[n] подтяжки на PB2 нет");
  CHECK(seq[0] == 0 && seq[1] == 3, "мутант пойман: кнопку не трогали, а огонь бежит назад");
  return sim_done("04/pb2 мутант без | 0x04");
}
