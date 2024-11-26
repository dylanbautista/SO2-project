//
// Created by Dylan Bautista on 26/11/24.
//

#ifndef _ZEOS_CIRCULAR_BUFFER_H
#define _ZEOS_CIRCULAR_BUFFER_H

#include <asm.h>
#include <segment.h>
#include <types.h>

#define MIDA_BUFFER 50

struct buffer_circular {
    int head;
    int tail;
    int size;
    char buffer[MIDA_BUFFER];
};

char circular_buffer_pop(struct buffer_circular * buff);
void circular_buffer_push(struct buffer_circular * buff, char c);
int circular_buffer_full(struct buffer_circular * buff);
int circular_buffer_empty(struct buffer_circular * buff);

#endif //_ZEOS_CIRCULAR_BUFFER_H
