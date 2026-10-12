/* Глава 4, задание 4 (klik.c): каждое нажатие PC1 разворачивает огонь, удержание - одно нажатие. */
#include "sim.h"
#include "04-porty/klik.c"
#define BTN PIN('C', 1)
static int seq[40], nseq = 0;
static int lit(void){
  int k = -1, cnt = 0;
  for(int i = 0; i < 4; i++) if(pin_out(PIN('B', 4 + i)) == 0){ k = i; cnt++; }
  return cnt == 1 ? k : -1;
}
static void hook(int k, double ms){
  (void)ms;
  if(nseq < 40) seq[nseq++] = lit();
  if(k == 5) btn_down(BTN);             /* нажали и держим 8 шагов */
  if(k == 13) btn_up(BTN);
  if(k == 17) btn_down(BTN);            /* второе нажатие, короткое (1 шаг) */
  if(k == 18) btn_up(BTN);
}
int main(void){
  /* шаги 0..5 вперёд; нажатие замечено после k=5 -> назад и при удержании назад; после k=17 снова вперёд */
  static const int want[26] = {0,1,2,3,0,1, 0,3,2,1,0,3,2,1,0,3,2,1, 2,3,0,1,2,3,0,1};
  sim_loop(26, hook);
  int ok = (nseq == 26);
  for(int i = 0; i < 26 && ok; i++) if(seq[i] != want[i]) ok = 0;
  if(!ok){ printf("  получено:"); for(int i = 0; i < nseq; i++) printf(" %d", seq[i]); printf("\n"); }
  CHECK(pin_pullup(BTN), "PC1: вход с подтяжкой");
  CHECK(ok, "нажатие разворачивает огонь, удержание не разворачивает снова, второе нажатие - снова вперёд");
  return sim_done("04/klik");
}
