/* Глава 10, вариант Б: kbd_calc.c. Столбцы клавиатуры на тех же PB4..PB7, что и разряды.
   Проверяем: подтяжка строк PB2, PB3 не затирается развёрткой (маски с | 0x0C); все 16 клавиш;
   ввод со сдвигом и переполнением 4 разрядов; * и #; одно нажатие с удержанием = одна цифра. */
#include "sim.h"
#include "10-klaviatura/kbd_calc.c"
static const int R[4] = {PIN('B', 2), PIN('B', 3), PIN('C', 0), PIN('C', 1)};
static const int C[4] = {PIN('B', 4), PIN('B', 5), PIN('B', 6), PIN('B', 7)};
static const int code[4][4] = {{1, 2, 3, 10}, {4, 5, 6, 11}, {7, 8, 9, 12}, {14, 0, 15, 13}};
static const int pos_r[10] = {3, 0, 0, 0, 1, 1, 1, 2, 2, 2}, pos_c[10] = {1, 0, 1, 2, 0, 1, 2, 0, 1, 2};
static void dig(int d){ kbd_tap(pos_r[d], pos_c[d], 50); }
static void typen(const char *s){ for(; *s; s++) dig(*s - '0'); }
int main(void){
  kbd_pins(R, C);
  sim_start();
  T0(40);
  CHECK(disp_num() == 0, "старт: 0000");
  int pu = 1;
  for(int k = 0; k < 8; k++){ T0(1); if(!pin_pullup(R[0]) || !pin_pullup(R[1])) pu = 0; }
  CHECK(pu, "при каждом разряде подтяжка на PB2, PB3 сохраняется (маски с | 0x0C)");
  typen("1234"); CHECK(disp_num() == 1234, "1, 2, 3, 4: 1234");
  dig(5); CHECK(disp_num() == 2345 && num == 2345, "5: старшая цифра ушла, 2345");
  kbd_tap(3, 2, 50); CHECK(disp_num() == 234, "#: стёрта последняя цифра, 0234");
  kbd_tap(3, 0, 50); CHECK(disp_num() == 0, "*: 0000");
  int bad = 0;
  for(int r = 0; r < 4; r++)
    for(int c = 0; c < 4; c++){
      int k = code[r][c];
      if(k < 10){
        kbd_tap(3, 0, 50); kbd_tap(r, c, 50);           /* * и цифра */
        if(disp_num() != k){ printf("  клавиша [%d][%d]: %d, ожидалось %d\n", r, c, disp_num(), k); bad++; }
      } else if(k >= 10 && k <= 13){
        kbd_tap(3, 0, 50); typen("77"); kbd_tap(r, c, 50); /* буква не меняет число */
        if(disp_num() != 77){ printf("  буква [%d][%d] изменила число: %d\n", r, c, disp_num()); bad++; }
      }
    }
  CHECK(bad == 0, "все 10 цифр вводятся, буквы A..D ничего не меняют");
  kbd_tap(3, 0, 50); typen("9999"); CHECK(disp_num() == 9999, "9999");
  dig(9); CHECK(num == 9999, "пятая 9: снова 9999 (без переполнения 16 бит: (num%1000)*10+k)");
  dig(0); CHECK(disp_num() == 9990, "0: 9990");
  kbd_tap(3, 0, 50); kbd_tap(0, 0, 1000); CHECK(num == 1, "удержание 1 в течение 2 с: одна цифра");
  return sim_done("10/kbd_calc");
}
