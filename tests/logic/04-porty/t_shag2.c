/* Глава 4, шаг 2 (shag2.c): бегущий огонь 1 2 3 4 1 2 ..., шаг 0,2 с. */
#include "sim.h"
#include "04-porty/shag2.c"
static int seq[16], nseq = 0, bad_ms = 0;
static int lit(void){                   /* какой светодиод горит: 0..3; -1, если горит не ровно один */
  int k = -1, cnt = 0;
  for(int i = 0; i < 4; i++) if(pin_out(PIN('B', 4 + i)) == 0){ k = i; cnt++; }
  return cnt == 1 ? k : -1;
}
static void hook(int k, double ms){ (void)k; if(nseq < 16) seq[nseq++] = lit(); if(ms != 200) bad_ms++; }
int main(void){
  sim_loop(12, hook);
  int ok = (nseq == 12);
  for(int i = 0; i < nseq; i++) if(seq[i] != i % 4) ok = 0;
  CHECK(ok, "огонь бежит по кругу: PB4, PB5, PB6, PB7, PB4 ... (горит ровно один)");
  CHECK(bad_ms == 0, "шаг огня 0,2 с");
  CHECK(pin_pullup(PIN('C', 1)), "PC1 уже с подтяжкой (пригодится в шаге 3)");
  return sim_done("04/shag2");
}
