/* Глава 4, задание 3 (pingpong.c): огонь отражается от краёв: 0 1 2 3 2 1 0 1 ... */
#include "sim.h"
#include "04-porty/pingpong.c"
static int seq[32], nseq = 0;
static int lit(void){
  int k = -1, cnt = 0;
  for(int i = 0; i < 4; i++) if(pin_out(PIN('B', 4 + i)) == 0){ k = i; cnt++; }
  return cnt == 1 ? k : -1;
}
static void hook(int k, double ms){ (void)k; (void)ms; if(nseq < 32) seq[nseq++] = lit(); }
int main(void){
  static const int want[6] = {0, 1, 2, 3, 2, 1};
  sim_loop(24, hook);
  int ok = (nseq == 24);
  for(int i = 0; i < nseq; i++) if(seq[i] != want[i % 6]) ok = 0;
  if(!ok){ printf("  получено:"); for(int i = 0; i < nseq; i++) printf(" %d", seq[i]); printf("\n"); }
  CHECK(ok, "0 1 2 3 2 1 0 1 2 3 2 1 ...: крайние горят по одному шагу");
  return sim_done("04/pingpong");
}
