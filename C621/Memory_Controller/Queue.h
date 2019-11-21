#ifndef __QUEUE_HH__
#define __QUEUE_HH__

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include "Request.h"

// Each Node stores one request
struct Node;
typedef struct Node Node;
typedef struct Node
{
    uint64_t mem_addr;
    Request_Type req_type; // Request type

    int bank_id; // Which bank the request targets to

    Node *prev;
    Node *next;
}Node;

typedef struct Queue
{
    Node *first;
    Node *last;

    unsigned size; // Current size of the queue
}Queue;

Queue* initQueue()
{
    Queue *q = (Queue *)malloc(sizeof(Queue));

    q->first = NULL;
    q->last = NULL;
    q->size = 0;

    return q;
}	

// Push a request to the queue
void pushToQueue(Queue *q, Request *req)
{
    // Check if the queue is empty.
    if (q->first == NULL && q->last == NULL)
    {
        Node *node = (Node *)malloc(sizeof(Node));
        node->mem_addr = req->memory_address;
        node->req_type = req->req_type;
        node->bank_id = req->bank_id;

        node->prev = NULL;
        node->next = NULL;
    }
}

#endif
