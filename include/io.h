/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>
#include <screen_matrix.h>

/** Screen functions **/
/**********************/

Byte inb (unsigned short port);
void printc(char c);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string);
void clear_screen();
void clear_paint_screen(Word matrix[25][80]);
void setXY(int x, int y);
void change_screen_colors(int bg, int fg);

#endif  /* __IO_H__ */
