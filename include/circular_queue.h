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

#ifndef __CUTILS_CIRCULAR_QUEUE_H
#define __CUTILS_CIRCULAR_QUEUE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The default maximum capacity of elements.
 */
#define CIRCULAR_QUEUE_CAPACITY_DEFAULT                     20
/**
 * The default maximum length of single element.
 */
#define CIRCULAR_QUEUE_ELEMENTSIZE_DEFAULT                  (1024 * 40)

/**
 * Error number returned when the length of the offered
 * element less than the maximum length of single queue
 * element.
 */
#define CIRCULAR_QUEUE_ERRNO_LESS_THAN_ELEMENTSIZE          (-0x00000001)   // int -1
/**
 * Error number returned when no more memory to malloc.
 */
#define CIRCULAR_QUEUE_ERRNO_OUT_OF_MAMORY                  (-0x00000002)   // int -2


/**
 * A element in circular queue.
 */
typedef struct _circular_queue_element_t {
    /**
     * Pointer to actual data this element represents.
     */
    char* value;
    /**
     * Actual length of this element.
     */
    size_t length;
    /**
     * Pointer to next element.
     */
    struct _circular_queue_element_t* next;
} circular_queue_element_t;

/**
 *  A circular queue is a linear data structure in which
 *  the operations are performed based on FIFO (First In
 *  First Out) principle and the last position is connected
 *  back to the first position to make a circle. In this
 *  implementation, the storage for elements are not
 *  allocated when initializing but allocated on demand
 *  and gradually increased to its capacity. When reaches
 *  the capacity, the new offered element will overwrite
 *  the oldest one at front.
 */
typedef struct _circular_queue_t {
    /**
     * The maximum capacity of elements.
     */
    int32_t capacity;
    /**
     * The maximum length of single element.
     */
    size_t elementsize;
    /**
     * Header of underlying linked linear storage allocated
     * on demond.
     */
    circular_queue_element_t* storage;
    /**
     * The count of underlying linked linear storage. The
     * maximum is {#capacity}.
     */
    int32_t scount;
    /**
     * The count of actual elements. The maximum is {#capacity}.
     */
    int32_t ecount;
    /**
     * The front pointer.
     */
    circular_queue_element_t* front;
    /**
     * The rear pointer.
     */
    circular_queue_element_t* rear;
    /**
     * Private mutex for poll/offer concurrently.
     */
    pthread_mutex_t mutex;
} circular_queue_t;

/**
 * Initializes the specified circular queue with default
 * attributes. It must call {#circular_queue_destroy()}
 * to release resources once a circular queue has been
 * initialized.
 *
 * @param queue specified circular queue be operated
 * @return 0 if successful, or corresponding error number
 */
int circular_queue_init(circular_queue_t *queue);

/**
 * Sets the maximum of elements the circular queue can hold.
 *
 * @param queue specified circular queue be operated
 * @param capacity the maximum capacity of elements
 * @return 0 if successful, or corresponding error number
 */
int circular_queue_setcapacity(circular_queue_t *queue, int32_t capacity);

/**
 * Sets the maximum length of single element in the circular
 * queue.
 *
 * @param queue specified circular queue be operated
 * @param elementsize the maximum length of single element
 * @return 0 if successful, or corresponding error number
 */
int circular_queue_setelementsize(circular_queue_t *queue, size_t elementsize);

/**
 * Inserts the specified element into the specified circular
 * queue if the length of the element is less than the maximum
 * length of single element in the circular queue, otherwise
 * it fails.
 *
 * @param queue specified circular queue be operated
 * @param value specified element to be inserted
 * @param length length of the specified element
 * @return 0 if successful, or corresponding error number,
 *         includes
 *         {#CIRCULAR_QUEUE_ERRNO_LESS_THAN_ELEMENTSIZE}
 *         or {#CIRCULAR_QUEUE_ERRNO_OUT_OF_MAMORY}.
 */
int circular_queue_offer(circular_queue_t *queue, const char *value, size_t length);

/**
 * Retrieves and removes the head of the specified circular
 * queue, returns 0 if this circular queue is empty, or
 * corresponding error number
 *
 * @param queue specified circular queue be operated
 * @param buffer supplied butter to store the element retrieved
 *               from the circular queue
 * @param size length of the supplied buffer
 * @return the total bytes of the element retrieved, or 0 if the
 *         queue is empty, or corresponding error number
 */
ssize_t circular_queue_poll(circular_queue_t *queue, char *buffer, size_t size);

/**
 * Retrieves and removes specified number of elements from the
 * head of the specified circular queue. All elements will be
 * pulled out if the circular queue holds less count of elements
 * than retrieved.
 *
 * @param queue specified circular queue be operated
 * @param buffer two-dimensional array supplied to store elements
                 retrieved from the circular queue
 * @param count the number of elements to retrieve
 * @param size length of the receiving buffer for single element
 * @param acount actual number of elements retrieved
 * @param asize array to receive the actual length of every
 *              element retrieved
 * @return the total bytes of the elements retrieved, 0 if the
 *         queue is empty, or corresponding error number
 */
ssize_t circular_queue_bucket(circular_queue_t *queue, char **buffer, int32_t count,
                              size_t size, int32_t *acount, size_t *asize);
/**
 * Locks the specified circular queue.
 *
 * @param queue specified circular queue be operated
 * @return 0 if successful, or corresponding error number
 */
int circular_queue_lock(circular_queue_t *queue);

/**
 * Unlocks the specified circular queue.
 *
 * @param queue specified circular queue be operated
 * @return 0 if successful, or corresponding error number
 */
int circular_queue_unlock(circular_queue_t *queue);

/**
 * Destroy the specified circular queue and release resources.
 *
 * @param queue specified circular queue be operated
 * @return 0 if successful, or corresponding error number
 */
int circular_queue_destroy(circular_queue_t *queue);


#ifdef __cplusplus
}
#endif

#endif //__CUTILS_CIRCULAR_QUEUE_H
