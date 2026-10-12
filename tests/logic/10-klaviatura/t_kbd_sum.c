/* Глава 10, ответ на задания 5 и 6: kbd_sum.c (A +1, B -1 по кругу; C "плюс", D "равно", больше 9999 - "----"). */
#include "sim.h"
#include "10-klaviatura/kbd_sum.c"
static const int R[4] = {PIN('B', 2), PIN('B', 3), PIN('C', 0), PIN('C', 1)};
static const int C[4] = {PIN('B', 4), PIN('B', 5), PIN('B', 6), PIN('B', 7)};
static const int pos_r[10] = {3, 0, 0, 0, 1, 1, 1, 2, 2, 2}, pos_c[10] = {1, 0, 1, 2, 0, 1, 2, 0, 1, 2};
static void dig(int d){ kbd_tap(pos_r[d], pos_c[d], 50); }
static void typen(const char *s){ for(; *s; s++) dig(*s - '0'); }
static void A(void){ kbd_tap(0, 3, 50); }
static void B(void){ kbd_tap(1, 3, 50); }
static void Cp(void){ kbd_tap(2, 3, 50); }
static void D(void){ kbd_tap(3, 3, 50); }
static void star(void){ kbd_tap(3, 0, 50); }
static int dashes(void){ for(int i = 0; i < 4; i++) if(disp_digit(i) != -2) return 0; return 1; }
int main(void){
  kbd_pins(R, C);
  sim_start();
  T0(40);
  CHECK(disp_num() == 0, "старт: 0000");
  typen("41"); A(); CHECK(disp_num() == 42, "41, A: 42");
  B(); B(); CHECK(disp_num() == 40, "B, B: 40");
  star(); B(); CHECK(disp_num() == 9999, "0000, B: 9999");
  A(); CHECK(disp_num() == 0, "9999, A: 0000");
  kbd_tap(0, 3, 1000); CHECK(num == 1, "A с удержанием 2 с: ровно +1");
  star(); typen("12"); Cp(); CHECK(disp_num() == 0, "12, C: число запомнено, ввод с 0000");
  typen("34"); D(); CHECK(disp_num() == 46, "12 C 34 D: 46");
  star(); typen("9999"); Cp(); dig(1); D(); CHECK(dashes(), "9999 C 1 D: ----");
  dig(5); CHECK(disp_num() == 5, "после ---- ввод начинается заново: 5");
  star(); typen("5000"); Cp(); typen("4999"); D(); CHECK(disp_num() == 9999, "5000 C 4999 D: 9999 (ещё влезает)");
  star(); typen("7"); Cp(); star(); typen("3"); D(); CHECK(disp_num() == 3, "*: очищает и запомненное число");
  return sim_done("10/kbd_sum");
}
