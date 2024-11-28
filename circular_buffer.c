//
// Created by Dylan Bautista on 26/11/24.
//

#include <circular_buffer.h>

/**
 * INIT_CIRCULAR_BUFFER - Initializes a circular_buffer
 * @buff: buffer to initialize
 */
void INIT_CIRCULAR_BUFFER(struct circular_buffer * buff) {
    buff->head = 0;
    buff->tail = -1;
    buff->num_elem = 0;
}

char circular_buffer_pop(struct circular_buffer * buff) {
    if (!circular_buffer_empty(buff)) { //If buffer is not empty
        int last_head = buff->head; //Save last head position
        if (buff->head < BUFFER_SIZE - 1) { //If head is not at the end of the array
            buff->head = buff->head + 1;
        } else { //If it is, head must point to position zero
            buff->head = 0;
        }
        buff->num_elem -= 1; //Decrease number of elements
        return buff->buffer[last_head]; //Return the previous value
    } else {
        return -1;
    }
}

int circular_buffer_push(struct circular_buffer * buff, char c) {
    if (!circular_buffer_full(buff)) { //If buffer is not full
        if (buff->tail < BUFFER_SIZE - 1) { //If tail is not at the end of the array
            buff->buffer[buff->tail + 1] = c; //
            buff->tail += 1;
        } else {
            buff->tail = 0;
            buff->buffer[0] = c;
        }
        buff->num_elem += 1;
        return 0;
    } else {
        return -1;
    }
}

int circular_buffer_full(struct circular_buffer * buff) {
    return (buff->num_elem >= BUFFER_SIZE);
}

int circular_buffer_empty(struct circular_buffer * buff) {
    return (buff->num_elem == 0);
}
