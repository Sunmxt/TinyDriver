#ifndef TINY_DRIVER_QUEUE
#define TINY_DRIVER_QUEUE

#include <stdint.h>
#include "klist.h"

typedef enum _Tiny_Driver_Critical_Queue_Flag_Type
{
    CQFT_U8 = 8,    //unsigned 8bit
    CQFT_U16 = 16,   //unsigned 16bit
    CQFT_U32 = 32,   //unsigned 32bit
}TDrvCriticalQueueFlagType;

typedef struct _Tiny_Driver_Critical_Queue
{
    TDrvCriticalQueueFlagType flag_type;
    void* flag_ptr;
    bi_list_node *queue;
    bi_list_node *add;
}TDrvCriticalQueue;

uint32_t TDrvCriticalQueuePush(TDrvCriticalQueue *_queue, bi_list_node *_node, uint32_t _mask);
bi_list_node* TDrvCriticalQueuePop(TDrvCriticalQueue *_queue, uint32_t _mask);
bi_list_node* TDrvCriticalQueuePeek(TDrvCriticalQueue *_queue, uint32_t _mask);


#endif
