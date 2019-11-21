#ifndef __CONTROLLER_HH__
#define __CONTROLLER_HH__

#include "Bank.h"
#include "Queue.h"

// Bank
extern void initBank(Bank *bank);

// Queue operations
extern Queue* initQueue();
extern void push(Queue *q, Request *req);

// CONSTANTS
static unsigned MAX_WAITING_QUEUE_SIZE = 64;
static unsigned BLOCK_SIZE = 128; // cache block size
static unsigned NUM_OF_BANKS = 8; // number of banks

// Controller definition
typedef struct Controller
{
    // The memory controller needs to maintain records of all bank's status
    Bank *bank_status;

    // Current memory clock
    uint64_t cur_clk;

    // A queue contains all the requests that are waiting to be issued.
    Queue *waiting_queue;

    // A queue contains all the requests that have already been issued but are waiting to complete.
    Queue *pending_queue;

    /* For decoding */
    unsigned bank_shift;
    uint64_t bank_mask;

}Controller;

Controller *initController()
{
    Controller *controller = (Controller *)malloc(sizeof(Controller));
    controller->bank_status = (Bank *)malloc(NUM_OF_BANKS * sizeof(Bank));
    for (int i = 0; i < NUM_OF_BANKS; i++)
    {
        initBank(&((controller->bank_status)[i]));
    }
    controller->cur_clk = 0;

    controller->waiting_queue = initQueue();
    controller->pending_queue = initQueue();

    controller->bank_shift = log2(BLOCK_SIZE);
    controller->bank_mask = (uint64_t)NUM_OF_BANKS - (uint64_t)1;

    return controller;
}

unsigned ongoingPendingRequests(Controller *controller)
{
    unsigned num_requests_left = controller->waiting_queue->size + 
                                 controller->pending_queue->size;

    return num_requests_left;
}

bool send(Controller *controller, Request *req)
{
    if (controller->waiting_queue->size == MAX_WAITING_QUEUE_SIZE)
    {
        return false;
    }

    // Decode the memory address
    req->bank_id = ((req->memory_address) >> controller->bank_shift) & controller->bank_mask;
    
    return true;
}

void tick(Controller *controller)
{

}

#endif
