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

#include "circular_queue.h"
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

    
#define CAPACITY_TEST 5
#define ELEMENTSIZE_TEST 20
    
void* circular_queue_assist_thread(void *args) {
    circular_queue_t* pqueue = (circular_queue_t*) args;
    char element[10];
    for (int i = 1000000; i < 1000045; i++) {
        printf("Element=%d\n", i);
        sprintf(element, "%d", i);
        circular_queue_lock(pqueue);
        circular_queue_offer(pqueue, element, strlen(element) + 1);
        circular_queue_unlock(pqueue);
        sleep(1);
    }
    return NULL;
}
    
int circular_queue_test() {
    circular_queue_t queue;
    circular_queue_init(&queue);
    circular_queue_setcapacity(&queue, CAPACITY_TEST);
    circular_queue_setelementsize(&queue, ELEMENTSIZE_TEST);

    ssize_t tsize;                                  // Total length of elements
    int32_t acount;                                 // Total count of elements
    size_t asize[CAPACITY_TEST];                    // Actual length of every element
    char buffer[CAPACITY_TEST][ELEMENTSIZE_TEST];   // Receiving buffer
    char* bufaddr[CAPACITY_TEST];                   // Buffer address
    for (int i = 0; i < CAPACITY_TEST; i++) {
        bufaddr[i] = buffer[i];
    }
    const char* element;                            // Tmp element

    tsize = circular_queue_poll(&queue, bufaddr[0], ELEMENTSIZE_TEST);
    bufaddr[0][tsize] = '\0';
    printf("Poll queue: tsize=%zu, content=[%s]\n", tsize, bufaddr[0]);

    element = "Jumping";
    circular_queue_offer(&queue, element, strlen(element) + 1);
    tsize = circular_queue_poll(&queue, bufaddr[0], ELEMENTSIZE_TEST);
    printf("Poll queue: tsize=%zu, content=[%s]\n", tsize, bufaddr[0]);

    element = "Grooving";
    circular_queue_offer(&queue, element, strlen(element) + 1);
    element = "Dancing";
    circular_queue_offer(&queue, element, strlen(element) + 1);
    element = "Everybody";
    circular_queue_offer(&queue, element, strlen(element) + 1);
    tsize = circular_queue_bucket(&queue, bufaddr, 2, ELEMENTSIZE_TEST, &acount, asize);
    printf("Poll queue: tsize=%zu, content=[%zu, %s], [%zu, %s]\n", tsize,
           asize[0], bufaddr[0], asize[1], bufaddr[1]);

    element = "Rooling";
    circular_queue_offer(&queue, element, strlen(element) + 1);
    element = "Moving";
    circular_queue_offer(&queue, element, strlen(element) + 1);
    element = "Singing";
    circular_queue_offer(&queue, element, strlen(element) + 1);
    tsize = circular_queue_bucket(&queue, bufaddr, 5, ELEMENTSIZE_TEST, &acount, asize);
    printf("Poll queue: tsize=%zu, content=[]\n", tsize);
    
    element = "Night&Day";
    circular_queue_offer(&queue, element, strlen(element) + 1);
    element = "Let's Fun Fun Together";
    circular_queue_offer(&queue, element, strlen(element) + 1);
    tsize = circular_queue_poll(&queue, bufaddr[0], ELEMENTSIZE_TEST);
    printf("Poll queue: tsize=%zu, content=[%s]\n", tsize, bufaddr[0]);
    
    element = "Let's";
    circular_queue_offer(&queue, element, strlen(element) + 1);
    tsize = circular_queue_poll(&queue, bufaddr[0], ELEMENTSIZE_TEST);
    printf("Poll queue: tsize=%zu, content=[%s]\n", tsize, bufaddr[0]);

    circular_queue_lock(&queue);
    tsize = circular_queue_poll(&queue, bufaddr[0], ELEMENTSIZE_TEST);
    circular_queue_unlock(&queue);
    printf("Poll queue: tsize=%zu, content=[%s]\n", tsize, bufaddr[0]);
    
    // Clean data
    tsize = circular_queue_bucket(&queue, bufaddr, 3, ELEMENTSIZE_TEST, &acount, asize);
    pthread_t pid;
    pthread_create(&pid, NULL, circular_queue_assist_thread, &queue);
    for (int i = 0; i < 16; i++) {
        circular_queue_lock(&queue);
        tsize = circular_queue_bucket(&queue, bufaddr, 3, ELEMENTSIZE_TEST, &acount, asize);
        circular_queue_unlock(&queue);
        printf("Poll queue: tsize=%zu, content=", tsize);
        if (tsize) {
            printf("[%zu, %s], ", asize[0], bufaddr[0]);
            printf("[%zu, %s], ", asize[1], bufaddr[1]);
            printf("[%zu, %s]\n", asize[2], bufaddr[2]);
        } else {
            printf("[]\n");
        }
        sleep(4);
    }
    pthread_join(pid, NULL);

    circular_queue_destroy(&queue);
    return 0;
}

#ifdef __cplusplus
}
#endif
