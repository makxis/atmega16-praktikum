/* Глава 10, вариант А, задание 3: kbd_lock.c (код 1234; прочерк после цифры; O - верно, E - ошибка). */
#include "sim.h"
#include "10-klaviatura/kbd_lock.c"
static const int R[4] = {PIN('B', 2), PIN('B', 3), PIN('C', 0), PIN('C', 1)};
static const int C[4] = {PIN('B', 5), PIN('B', 6), PIN('B', 7), PIN('D', 7)};
static const int pos_r[10] = {3, 0, 0, 0, 1, 1, 1, 2, 2, 2}, pos_c[10] = {1, 0, 1, 2, 0, 1, 2, 0, 1, 2};
static void dig(int d){ kbd_tap(pos_r[d], pos_c[d], 50); }
static void star(void){ kbd_tap(3, 0, 50); }
int main(void){
  kbd_pins(R, C);
  disp_digits(PIN('B', 4), -1, -1, -1);
  sim_start();
  T0(40);
  CHECK(disp_digit(0) == -1, "старт: индикатор пустой");
  dig(1); CHECK(disp_digit(0) == -2, "после первой цифры прочерк");
  dig(2); dig(3); CHECK(disp_digit(0) == -2, "после третьей цифры прочерк");
  dig(4); CHECK((disp_code(0) & 0x7F) == 0x3F, "1, 2, 3, 4: O (код верный)");
  dig(5); CHECK((disp_code(0) & 0x7F) == 0x3F, "лишняя цифра после четвёртой ничего не меняет");
  star(); CHECK(disp_digit(0) == -1, "*: заново, индикатор пустой");
  dig(1); dig(2); dig(3); dig(5); CHECK(disp_digit(0) == 14, "*, 1, 2, 3, 5: E (ошибка)");
  star(); dig(9); dig(2); dig(3); dig(4); CHECK(disp_digit(0) == 14, "ошибка в первой цифре тоже даёт E, но только после четвёртой");
  star(); kbd_tap(0, 3, 50); kbd_tap(1, 3, 50); kbd_tap(3, 2, 50);   /* A B # */
  CHECK(disp_digit(0) == -1, "буквы и # цифрами не считаются");
  dig(1); dig(2); dig(3); dig(4); CHECK((disp_code(0) & 0x7F) == 0x3F, "после букв код 1234 по-прежнему открывает");
  star(); kbd_tap(0, 0, 1000); CHECK(pos == 1, "удержание цифры 2 с = одна цифра");
  return sim_done("10/kbd_lock");
}
