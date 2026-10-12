/* Глава 4, шаг 3 (ogon.c): огонь бежит вперёд, пока кнопка PC1 нажата, - назад. */
#include "sim.h"
#include "04-porty/ogon.c"
#define BTN PIN('C', 1)
static int seq[32], nseq = 0, bad_ms = 0;
static int lit(void){
  int k = -1, cnt = 0;
  for(int i = 0; i < 4; i++) if(pin_out(PIN('B', 4 + i)) == 0){ k = i; cnt++; }
  return cnt == 1 ? k : -1;
}
static void hook(int k, double ms){
  if(nseq < 32) seq[nseq++] = lit();
  if(ms != 200) bad_ms++;
  if(k == 7) btn_down(BTN);             /* нажали после 8-го шага */
  if(k == 15) btn_up(BTN);              /* отпустили после 16-го */
}
int main(void){
  static const int want[24] = {0,1,2,3,0,1,2,3, 2,1,0,3,2,1,0,3, 0,1,2,3,0,1,2,3};
  sim_loop(24, hook);
  int ok = (nseq == 24);
  for(int i = 0; i < 24 && ok; i++) if(seq[i] != want[i]) ok = 0;
  if(!ok){ printf("  получено:"); for(int i = 0; i < nseq; i++) printf(" %d", seq[i]); printf("\n"); }
  CHECK(pin_pullup(BTN), "PC1: вход с подтяжкой");
  CHECK(ok, "вперёд 0123 0123, кнопка нажата: назад 2103 2103, отпущена: снова вперёд");
  CHECK(bad_ms == 0, "шаг огня 0,2 с");
  return sim_done("04/ogon");
}
