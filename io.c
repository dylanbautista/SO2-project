/*
 * io.c - 
 */

#include <io.h>

#include <colors.h>

#include <screen_matrix.h>

#include <types.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

Byte x, y=19;

/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

void printc(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | 0x0200;
    Word *screen = (Word *)0xb8000;
    screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
    }
  }
}

void print_color(char c, color_t char_col, color_t back_col, int blinking) {
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | (((char_col << 8 | back_col << 12) & 0x7FFF) | (blinking <<  15));
    Word *screen = (Word *)0xb8000;
    screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
    }
  }
}

void printc_xy_color(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    print_color(string[i], Light_Gray,Green, 0);
}

void clear_screen() {
  Word *screen = (Word *)0xb8000;
  for (int i = 0; i < NUM_COLUMNS; ++i) {
    for (int j = 0; j < NUM_ROWS; ++j) {
      screen[j * NUM_COLUMNS + i] = 0x00;
    }
  } 
}

void clear_paint_screen(Word matrix[25][80]) {
  Word *screen = (Word *)0xb8000;
  for (int i = 0; i < NUM_COLUMNS; ++i) {
    for (int j = 0; j < NUM_ROWS; ++j) {
      screen[j * NUM_COLUMNS + i] = matrix[j][i];
    }
  } 
}

void setXY(int x_n, int y_n) {
  x = x_n;
  y = y_n;
}

void change_screen_colors(int bg, int fg) {
  Word *screen = (Word *)0xb8000;
  for (int i = 0; i < NUM_COLUMNS; ++i) {
    for (int j = 0; j < NUM_ROWS; ++j) {
      if (screen[j * NUM_COLUMNS + i] != NULL) {
        screen[j * NUM_COLUMNS + i] = (screen[j * NUM_COLUMNS + i] & 0x80FF) | ((bg << 8 | fg << 12) & 0x7FFF);
      } else {
        screen[j * NUM_COLUMNS + i] = (NULL & 0x80FF) | ((bg << 8 | fg << 12) & 0x7FFF);
      }
    }
  } 
}