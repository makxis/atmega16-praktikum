/* Глава 4, задание 2 (dva.c): горят два соседних светодиода, огонь шириной два бежит по кругу. */
#include "sim.h"
#include "04-porty/dva.c"
static int seq[16], nseq = 0;
static int lit_mask(void){ int m = 0; for(int i = 0; i < 4; i++) if(pin_out(PIN('B', 4 + i)) == 0) m |= 1 << i; return m; }
static void hook(int k, double ms){ (void)k; (void)ms; if(nseq < 16) seq[nseq++] = lit_mask(); }
int main(void){
  static const int want[4] = {0x3, 0x6, 0xC, 0x9};     /* 1+2, 2+3, 3+4, 4+1 */
  sim_loop(12, hook);
  int ok = (nseq == 12);
  for(int i = 0; i < nseq; i++) if(seq[i] != want[i % 4]) ok = 0;
  CHECK(ok, "горят 1+2, 2+3, 3+4, 4+1, 1+2 ...");
  return sim_done("04/dva");
}
