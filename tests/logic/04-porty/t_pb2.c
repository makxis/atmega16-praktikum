/* Глава 4, задание 5 (pb2.c): кнопка на PB2, светодиоды на PB4..PB7 того же порта. */
#include "sim.h"
#include "04-porty/pb2.c"
#define BTN PIN('B', 2)
static int seq[24], nseq = 0, nopull = 0;
static int lit(void){
  int k = -1, cnt = 0;
  for(int i = 0; i < 4; i++) if(pin_out(PIN('B', 4 + i)) == 0){ k = i; cnt++; }
  return cnt == 1 ? k : -1;
}
static void hook(int k, double ms){
  (void)ms;
  if(nseq < 24) seq[nseq++] = lit();
  if(!pin_pullup(BTN)) nopull++;
  if(k == 7) btn_down(BTN);
  if(k == 11) btn_up(BTN);
}
int main(void){
  static const int want[16] = {0,1,2,3,0,1,2,3, 2,1,0,3, 0,1,2,3};
  sim_loop(16, hook);
  int ok = (nseq == 16);
  for(int i = 0; i < 16 && ok; i++) if(seq[i] != want[i]) ok = 0;
  if(!ok){ printf("  получено:"); for(int i = 0; i < nseq; i++) printf(" %d", seq[i]); printf("\n"); }
  CHECK(nopull == 0, "подтяжка PB2 жива после каждой записи в PORTB");
  CHECK(ok, "отпущена: вперёд; нажата: назад; отпущена: снова вперёд");
  return sim_done("04/pb2");
}
