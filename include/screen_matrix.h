#ifndef _ZEOS_SCREEN_MATRIX_H
#define _ZEOS_SCREEN_MATRIX_H

#include <types.h>
#include <colors.h>

/******************************************
 * 
 * Word = 16 bits
 * 
 * Information of each Word entry of the matrix.
 * 
 * | Blink | Background color | Foreground color  |  Ignored  |  Char |
 *    x            xxx                xxxx             0000      xxxx
 *  1 bit         3 bits              4 bits           4 bits    4 bits
 * 
 ******************************************/
typedef Word screen_matrix[80][25];

typedef char char_mat[80][25];
typedef color_t clr_mat[80][25];
typedef int blnk_mat[80][25];


Word screen_matrix_format(char c, color_t fg_clr, color_t bg_clr, int blnk);


/**
 * screen_matrix_compose - Compose a screen_matrix by three matrix components.
 * @sm: Screen matrix address to return the composed matrix.
 * @chm: Char matrix address.
 * @fgclm: Foreground color matrix.
 * @bgclm: Backgorung color matrix.
 * @blnk_mat: Blink "bit" matrix.
 */
void screen_matrix_compose(screen_matrix* sm, const char_mat* chm, const clr_mat* fgclm, const clr_mat* bgclm, const blnk_mat* bm);
color_t screen_matrix_getColor(const screen_matrix* matrix);
char screen_matrix_getChar(const screen_matrix* matrix, int x, int y);


#endif

