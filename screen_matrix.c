#include <screen_matrix.h>
#include <types.h>
#include <libc.h>

Word screen_matrix_format(char c, color_t fg_clr, color_t bg_clr, int blnk) {
    return (Word) (c & 0x00FF) | (((fg_clr << 8 | bg_clr << 12) & 0x7FFF) | (blnk <<  15));
}

color_t screen_matrix_get_fg_color(const screen_matrix matrix, int x, int y) {
    return (color_t) (matrix[x][y] & 0x0F00) >> 8;
}

color_t screen_matrix_get_bg_color(const screen_matrix matrix, int x, int y) {
    return (color_t) (matrix[x][y] & 0x7000) >> 12;
}

char screen_matrix_getChar(const screen_matrix matrix, int x, int y) {
    return (char) (matrix[x][y] & 0x00FF);
}

void screen_matrix_compose(screen_matrix sm, const char_mat chm, const clr_mat fgclm, const clr_mat bgclm, const blnk_mat bm) {
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            (sm)[i][j] = screen_matrix_format(chm[i][j], fgclm[i][j], bgclm[i][j], bm[i][j]);
        }
    }
}

void screen_matrix_compose_char(screen_matrix sm, const char_mat chm, color_t fg_clr, color_t bg_clr) {
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            if (chm[i][j] != 0) 
                sm[i][j] = (chm[i][j] & 0x00FF) | ((fg_clr << 8 | bg_clr << 12) & 0x7FFF);
            else
                sm[i][j] = 0;
        }
    }
}

void screen_matrix_init(screen_matrix sm) {
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 80; j++) {
            sm[i][j] = 0;
        }
    }
}

int screen_matrix_clrscr(Word b[25][80]) {
    return clrscr((char*) b);
}