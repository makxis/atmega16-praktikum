/* Глава 4, задание 1 (skorost.c): шаг огня 0,2 с, пока нажата PC0 - 0,05 с; PC1 по-прежнему разворачивает. */
#include "sim.h"
#include "04-porty/skorost.c"
static int prev = -2, nsteps = 0, seq[64];
static double t_last = 0, dur[64];
static int lit(void){
  int k = -1, cnt = 0;
  for(int i = 0; i < 4; i++) if(pin_out(PIN('B', 4 + i)) == 0){ k = i; cnt++; }
  return cnt == 1 ? k : -1;
}
static void hook(int k, double ms){
  (void)k; (void)ms;
  int l = lit(); double t = sim_time_ms();
  if(l != prev){                        /* огонь сделал шаг: запоминаем, сколько горел прошлый */
    if(prev != -2 && nsteps < 64){ dur[nsteps] = t - t_last; seq[nsteps] = prev; nsteps++; }
    prev = l; t_last = t;
  }
  if(t >= 1000 && t < 1010) btn_down(PIN('C', 0));    /* с 1 с держим кнопку скорости */
  if(t >= 1500 && t < 1510) btn_down(PIN('C', 1));    /* с 1,5 с ещё и кнопку направления */
}
int main(void){
  sim_loop(250, hook);                  /* 250 пауз по 10 мс = 2,5 с */
  CHECK(pin_pullup(PIN('C', 0)) && pin_pullup(PIN('C', 1)), "PC0 и PC1: входы с подтяжкой");
  int slow = 1, fast = 0, fwd = 1, back = 0;
  for(int i = 0; i < nsteps; i++){
    double t_end = 0; for(int j = 0; j <= i; j++) t_end += dur[j];
    if(t_end <= 1000 && (dur[i] < 199 || dur[i] > 201)) slow = 0;
    if(t_end > 1100 && dur[i] > 49 && dur[i] < 51) fast++;
  }
  for(int i = 1; i < nsteps; i++){
    double t_end = 0; for(int j = 0; j <= i; j++) t_end += dur[j];
    int d = (seq[i] - seq[i - 1] + 4) % 4;
    if(t_end < 1450 && d != 1) fwd = 0;
    if(t_end > 1600 && d == 3) back++;
  }
  CHECK(slow, "кнопки отпущены: шаг 0,2 с");
  CHECK(fast >= 8, "нажата PC0: шаг 0,05 с");
  CHECK(fwd, "PC1 отпущена: огонь бежит вперёд");
  CHECK(back >= 5, "нажата и PC1: огонь бежит назад (на быстрой скорости)");
  return sim_done("04/skorost");
}
