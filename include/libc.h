/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>
#include <types.h>

extern int errno;

int write(int fd, char *buffer, int size);

void itoa(int a, char *b);

int strlen(char *a);

void perror();

int getpid();

int fork();

int threadCreateWithStack( void (*function)(void* arg), int N, void* parameter, void (*ext));

int getKey(char* b, int timeout);

int gettime();

void exit();

int yield();

int clrscr(char* b);

int gotoXY(int x, int y);

int changeColor(int fg, int bg);

char* memRegGet(int num_pages);

int memRegDel(char* m);

int get_stats(int pid, struct stats *st);

void SAVE_REGS(void);
void RESTORE_REGS(void);

#endif  /* __LIBC_H__ */
