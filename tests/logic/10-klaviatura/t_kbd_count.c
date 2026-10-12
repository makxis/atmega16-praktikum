/* Глава 10, вариант А, задание 2: kbd_count.c (цифра задаёт значение, A +1, B -1 по кругу 0..9). */
#include "sim.h"
#include "10-klaviatura/kbd_count.c"
static const int R[4] = {PIN('B', 2), PIN('B', 3), PIN('C', 0), PIN('C', 1)};
static const int C[4] = {PIN('B', 5), PIN('B', 6), PIN('B', 7), PIN('D', 7)};
static void k(int r, int c){ kbd_tap(r, c, 50); }
int main(void){
  kbd_pins(R, C);
  disp_digits(PIN('B', 4), -1, -1, -1);
  sim_start();
  T0(40);
  CHECK(disp_digit(0) == 0, "старт: 0");
  k(2, 0);                                              /* 7 */
  CHECK(disp_digit(0) == 7, "клавиша 7: 7");
  k(0, 3); k(0, 3); k(0, 3);                            /* A A A */
  CHECK(disp_digit(0) == 0, "7, A, A, A: 0 (после 9 идёт 0)");
  k(1, 3);                                              /* B */
  CHECK(disp_digit(0) == 9, "затем B: 9 (перед 0 стоит 9)");
  kbd_tap(0, 3, 1000);                                  /* A, держим 2 с */
  CHECK(disp_digit(0) == 0 && n == 0, "A с удержанием 2 с: ровно один шаг, 9 -> 0");
  k(1, 1);                                              /* 5 */
  k(3, 0); k(3, 2); k(2, 3); k(3, 3);                   /* * # C D */
  CHECK(disp_digit(0) == 5, "клавиши *, #, C, D ничего не меняют");
  for(int i = 0; i < 25; i++) k(0, 3);                  /* 25 раз A */
  CHECK(disp_digit(0) == 0 && n == 0, "5 + 25 нажатий A = 0 (по кругу, каждое нажатие ровно +1)");
  return sim_done("10/kbd_count");
}
