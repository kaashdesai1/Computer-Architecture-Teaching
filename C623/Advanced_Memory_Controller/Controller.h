#ifndef __CONTROLLER_HH__
#define __CONTROLLER_HH__

#include "Bank.h"
#include "Queue.h"

// Bank
extern void initBank(Bank *bank);

// Queue operations
extern Queue* initQueue();
extern void pushToQueue(Queue *q, Request *req);
extern void migrateToQueue(Queue *q, Node *_node);
extern void deleteNode(Queue *q, Node *node);

// CONSTANTS
static unsigned MAX_WAITING_QUEUE_SIZE = 64;
static unsigned BLOCK_SIZE = 64; // cache block size
static unsigned NUM_OF_CHANNELS = 4; // 4 channels/controllers in total
static unsigned NUM_OF_BANKS = 32; // number of banks per channel

// DRAM Timings
static unsigned nclks_channel = 15;
static unsigned nclks_read = 53;
static unsigned nclks_write = 53;
static unsigned choice_sheduler=1;

// PCM Timings
// static unsigned nclks_read = 57;
// static unsigned nclks_write = 162;

// Controller definition
typedef struct Controller
{
    // The memory controller needs to maintain records of all bank's status
    Bank *bank_status;

    // Current memory clock
    uint64_t cur_clk;

    // Channel status
    uint64_t channel_next_free;
    uint64_t channel_next_available;


    // A queue contains all the requests that are waiting to be issued.
    Queue *waiting_queue;

    // A queue contains all the requests that have already been issued 
    // but are waiting to complete.
    Queue *pending_queue;

    /* For decoding */
    unsigned bank_shift;
    uint64_t bank_mask;
    int sheduler; //1 is for FCFS and 2 is for FR FCFS;

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
    controller->channel_next_free = 0;
    controller->channel_next_available = 0;


    controller->waiting_queue = initQueue();
    controller->pending_queue = initQueue();

    controller->bank_shift = log2(BLOCK_SIZE) + log2(NUM_OF_CHANNELS);
    controller->bank_mask = (uint64_t)NUM_OF_BANKS - (uint64_t)1;
    
    controller->sheduler=choice_of_sheduler;
    
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
    
    // Push to queue
    pushToQueue(controller->waiting_queue, req);

    return true;
}

void tick(Controller *controller)
{
    cout<<"Enter the choice for sheduler, press 1 for FCFS and 2 fro FR FCFS";
    cin>>choice_sheduler;
    // Step one, update system stats
    ++(controller->cur_clk);
    // printf("Clk: ""%"PRIu64"\n", controller->cur_clk);
    for (int i = 0; i < NUM_OF_BANKS; i++)
    {
        ++(controller->bank_status)[i].cur_clk;
        // printf("%"PRIu64"\n", (controller->bank_status)[i].cur_clk);
    }
    // printf("\n");

    // Step two, serve pending requests
    if (controller->pending_queue->size)
    {
        Node *first = controller->pending_queue->first;
        if (first->end_exe <= controller->cur_clk)
        {
            /*
            printf("Clk: ""%"PRIu64"\n", controller->cur_clk);
            printf("Address: ""%"PRIu64"\n", first->mem_addr);
            printf("Channel ID: %d\n", first->channel_id);
            printf("Bank ID: %d\n", first->bank_id);
            printf("Begin execution: ""%"PRIu64"\n", first->begin_exe);
            printf("End execution: ""%"PRIu64"\n\n", first->end_exe);
            */

            deleteNode(controller->pending_queue, first);
        }
    }

    // Step three, find a request to schedule
    if (controller->waiting_queue->size)
    {
        // Implementation One - FCFS
        Node *first = controller->waiting_queue->first;
        int target_bank_id = first->bank_id;
        
        if(choice_sheduler==2)
        {
            
            if ((controller->bank_status)[target_bank_id].next_available <= controller->cur_clk && 
            controller->channel_next_available <= controller->cur_clk)
        }
        
        else if ((controller->bank_status)[target_bank_id].next_free <= controller->cur_clk && 
            controller->channel_next_free <= controller->cur_clk)
        
            first->begin_exe = controller->cur_clk;
            if (first->req_type == READ)
            {
                first->end_exe = first->begin_exe + (uint64_t)nclks_read;
            }
            else if (first->req_type == WRITE)
            {
                first->end_exe = first->begin_exe + (uint64_t)nclks_write;
            }
           
            if(choice_sheduler==2)
            {
                // The target bank should be given to available once there is a request
                (controller->bank_status)[target_bank_id].next_available = first->end_exe;
                controller->channel_next_available = controller->cur_clk + nclks_channel;
            }
            // The target bank is no longer free until this request completes.
            else
            {
                (controller->bank_status)[target_bank_id].next_free = first->end_exe;
                controller->channel_next_free = controller->cur_clk + nclks_channel;
            }
            migrateToQueue(controller->pending_queue, first);
            deleteNode(controller->waiting_queue, first);
        }
    }
}

//definition of latency function 
float access_latency(float avg_clock_cycles, int NUM_OF_BANKS)
{
    /*Access latency is the number of
(memory) clock cycles between the time a request is inserted in the 
transaction queue and the time it’sscheduled*/
float diff_in_clock_cycles=(controller->bank_status[0]).cur_clk

for(int i=0;i<NUM_OF_BANKS;i++)
{
    diff_in_clock_cycles-=(controller->bank_status[i]).cur_clk;
}

diff_in_clock_cycles=abs(diff_in_clock_cycles);

return diff_in_clock_cycles;

}

uint64_t number_of_conflict()
{
    return controller->bank_shift-controller->clk;
}
#endif
