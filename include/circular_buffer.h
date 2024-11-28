//
// Created by Dylan Bautista on 26/11/24.
//

#ifndef _ZEOS_CIRCULAR_BUFFER_H
#define _ZEOS_CIRCULAR_BUFFER_H

#include <asm.h>
#include <segment.h>
#include <types.h>

#define BUFFER_SIZE 3

struct circular_buffer {
    int head;
    int tail;
    int num_elem;
    char buffer[BUFFER_SIZE];
};

void INIT_CIRCULAR_BUFFER(struct circular_buffer * buff);
char circular_buffer_pop(struct circular_buffer * buff);
int circular_buffer_push(struct circular_buffer * buff, char c);
int circular_buffer_full(struct circular_buffer * buff);
int circular_buffer_empty(struct circular_buffer * buff);

#endif //_ZEOS_CIRCULAR_BUFFER_H
