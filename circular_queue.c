/*
 * Copyright (c) 2017, Feilong Wang
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *  Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 *  Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include "circular_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

int circular_queue_init(circular_queue_t *queue) {
    queue->capacity = CIRCULAR_QUEUE_CAPACITY_DEFAULT;
    queue->elementsize = CIRCULAR_QUEUE_ELEMENTSIZE_DEFAULT;
    queue->storage = NULL;
    queue->scount = 0;
    queue->ecount = 0;
    queue->front = NULL;
    queue->rear = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    return 0;
}

int circular_queue_setcapacity(circular_queue_t *queue, int32_t capacity) {
    queue->capacity = capacity;
    return 0;
}

int circular_queue_setelementsize(circular_queue_t *queue, size_t elementsize) {
    queue->elementsize = elementsize;
    return 0;
}

int circular_queue_offer(circular_queue_t *queue, const char *value, size_t length) {
    if (queue->elementsize < length) {
        return CIRCULAR_QUEUE_ERRNO_LESS_THAN_ELEMENTSIZE;
    }
    if (length <= 0) {
        // Do NOT allow to put 0-length element.
        return 0;
    }
    if (queue->ecount < queue->scount) {
        // This can never be queue->tail->next == queue->header
        // since queue->ecount < queue->scount.
        memcpy(queue->rear->next->value, value, length);
        queue->rear->next->length = length;
        queue->rear = queue->rear->next;
        if (queue->ecount == 0) {
            // In this case, elements have been polled out.
            queue->front = queue->rear;
        }
        queue->ecount++;
    } else {
        // This must be queue->ecount == queue->scount.
        if (queue->ecount < queue->capacity) {
            char* space = (char*) malloc(queue->elementsize);
            if (!space) {
                return CIRCULAR_QUEUE_ERRNO_OUT_OF_MAMORY;
            }
            memcpy(space, value, length);
            circular_queue_element_t *element = (circular_queue_element_t *) malloc(
                    sizeof(circular_queue_element_t));
            if (!element) {
                free(space);
                return CIRCULAR_QUEUE_ERRNO_OUT_OF_MAMORY;
            }
            element->value = space;
            element->length = length;
            if (queue->ecount == 0) {
                queue->storage = queue->front = queue->rear = element->next = element;
            } else {
                element->next = queue->rear->next;
                queue->rear->next = element;
                queue->rear = element;
            }
            queue->ecount++;
            queue->scount++;
        } else {
            memcpy(queue->rear->next->value, value, length);
            queue->rear->next->length = length;
            queue->rear = queue->rear->next;
            queue->front = queue->front->next;
        }
    }
    return 0;
}

ssize_t circular_queue_poll(circular_queue_t *queue, char *buffer, size_t size) {
    return circular_queue_bucket(queue, &buffer, 1, size, NULL, NULL);
}

ssize_t circular_queue_bucket(circular_queue_t *queue, char **buffer, int32_t count,
                              size_t size, int32_t *acount, size_t *asize) {
    // Should be less than elementsize or length of element?
    if (size < queue->elementsize) {
        return CIRCULAR_QUEUE_ERRNO_LESS_THAN_ELEMENTSIZE;
    }
    if (queue->ecount <= 0) {
        return 0;
    }
    int32_t mcount = count < queue->ecount ? count : queue->ecount;
    size_t tsize = 0;
    for (int i = 0; i < mcount; i++) {
        memset(buffer[i], 0, size);
        memcpy(buffer[i], queue->front->value, queue->front->length);
        if (asize) {
            asize[i] = queue->front->length;
        }
        tsize += queue->front->length;
        queue->front = queue->front->next;
        queue->ecount--;
    }
    if (acount) {
        *acount = mcount;
    }
    return tsize;
}

int circular_queue_lock(circular_queue_t *queue) {
    return pthread_mutex_lock(&queue->mutex);
}

int circular_queue_unlock(circular_queue_t *queue) {
    return pthread_mutex_unlock(&queue->mutex);
}

int circular_queue_destroy(circular_queue_t *queue) {
    circular_queue_element_t* element;
    while (queue->scount > 0) {
        element = queue->storage;
        queue->storage = element->next;
        free(element->value);
        element->value = NULL;
        element->next = NULL;
        free(element);
        queue->scount--;
    }
    queue->ecount = 0;
    queue->storage = NULL;
    queue->front = NULL;
    queue->rear = NULL;
    queue->elementsize = 0;
    queue->capacity = 0;
    pthread_mutex_destroy(&queue->mutex);
    return 0;
}


#ifdef __cplusplus
}
#endif

