#include "queue.h"
#include "atomic.h"

uint32_t critical_queue_set_exist_flag8(uint8_t* _target, uint8_t _mask)
{
    uint8_t old, chg;

    for(old = *_target ; ; old = chg)
    {
        chg = TDrvCAS8(_target, old, old | _mask);
        if(chg == old)
            break;
    }

    return chg;
}

uint32_t critical_queue_set_exist_flag16(uint16_t* _target, uint16_t _mask)
{
    uint16_t old, chg;

    for(old = *_target ; ; old = chg)
    {
        chg = TDrvCAS16(_target, old, old | _mask);
        if(chg == old)
            break;
    }

    return chg;
}

uint32_t critical_queue_set_exist_flag32(uint32_t* _target, uint32_t _mask)
{
    uint32_t old, chg;

    for(old = *_target ; ; old = chg)
    {
        chg = TDrvCAS32(_target, old, old | _mask);
        if(chg == old)
            break;
    }

    return chg;
}

uint32_t critical_queue_set_exist_flag(TDrvCriticalQueue *_queue, uint32_t _mask)
{
    switch(_queue -> flag_type)
    {
    case CQFT_U8: return critical_queue_set_exist_flag8((uint8_t*)_queue -> flag_ptr, (uint8_t)_mask);
    case CQFT_U16: return critical_queue_set_exist_flag16((uint16_t*)_queue -> flag_ptr, (uint16_t)_mask);
    case CQFT_U32: return critical_queue_set_exist_flag32((uint32_t*)_queue -> flag_ptr, _mask);
    }
    return 0;
}

uint32_t critical_queue_clear_exist_flag8(uint8_t *_target, uint8_t _mask)
{
    uint8_t old, chg;

    for(old = *_target; old & _mask ; old = chg)
    {
        chg = TDrvCAS8((uint8_t*)_target, old, old & (~_mask));
        if(chg == old)
        {
            old = chg;
            break;
        }
    }
    return old;
}

uint32_t critical_queue_clear_exist_flag16(uint16_t *_target, uint16_t _mask)
{
    uint16_t old, chg;

    for(old = *_target; old & _mask ; old = chg)
    {
        chg = TDrvCAS16(_target, old, old & (~_mask));
        if(chg == old)
        {
            old = chg;
            break;
        }
    }
    return old;
}

uint32_t critical_queue_clear_exist_flag32(uint32_t *_target, uint32_t _mask)
{
    uint32_t old, chg;

    for(old = *_target; old & _mask ; old = chg)
    {
        chg = TDrvCAS32(_target, old, old & (~_mask));
        if(chg == old)
        {
            old = chg;
            break;
        }
    }
    return old;
}

uint32_t critical_queue_clear_exist_flag(TDrvCriticalQueue *_queue, uint32_t _mask)
{
    switch(_queue -> flag_type)
    {
    case CQFT_U8: return critical_queue_clear_exist_flag8((uint8_t*)_queue -> flag_ptr, (uint8_t)_mask);
    case CQFT_U16: return critical_queue_clear_exist_flag16((uint16_t*)_queue -> flag_ptr, (uint16_t)_mask);
    case CQFT_U32: return critical_queue_clear_exist_flag32((uint32_t*)_queue -> flag_ptr, (uint32_t)_mask);
    }
    
    return 0;
}

uint32_t critical_queue_update_queue(TDrvCriticalQueue *_queue, uint32_t _mask)
{
    uint32_t count;
    bi_list_node *news, *prev, *it;

    //clear new message bit
    critical_queue_clear_exist_flag(_queue, _mask);

    // load queued node
    news = (bi_list_node*)TDrvExclusiveSwap32((uint32_t*)&_queue -> add, 0);
    if(!news)
        return 0;
    
    for(count = 0, prev = 0 ; ; )
    {
        count++;
        if(news -> prev)
        {
            count++;
            for(it = news -> prev; it -> next ; it = it -> next, count++);
            
            it -> next = prev;

            if(news -> next)
            {
                prev = news;
                it = news -> next;
                news -> next = news -> prev;
                news = it;
            }
            else
            {
                news -> next = news -> prev;
                prev = 0;
                break;
            }
        }
        else if(news -> next)
        {
            it = news -> next;
            news -> next = prev;
            news = it;
        }
        else
        {
            news -> next = prev;
            break;
        }
    }

    if(_queue -> queue)
    {
        for(it = _queue -> queue; it -> next; it = it -> next);
        it -> next = news;
    }
    else
        _queue -> queue = news;

    return count;
}

uint32_t TDrvCriticalQueuePush(TDrvCriticalQueue *_queue, bi_list_node *_node, uint32_t _mask)
{
    bi_list_node *old, *chg;

    // append to list
    for(old = _queue -> add ; ; _node -> next = chg)
    {
        _node -> next = old;
        chg = (bi_list_node*)TDrvCAS32((uint32_t*)&_queue -> add, (uint32_t)old, (uint32_t)_node);
        if(chg == old)
            break;
    }

    if(!old) // my duty to set message exist flag.
        return critical_queue_set_exist_flag(_queue, _mask);
    
    //return flags with specified bit length.
    return *((uint32_t*)_queue -> flag_ptr) & (~(((uint32_t)-1) << _queue -> flag_type));
}

bi_list_node* TDrvCriticalQueuePeek(TDrvCriticalQueue *_queue, uint32_t _mask)
{
    critical_queue_update_queue(_queue, _mask);

    return _queue -> queue;
}

bi_list_node* TDrvCriticalQueuePop(TDrvCriticalQueue *_queue, uint32_t _mask)
{
    bi_list_node *poped;

    critical_queue_update_queue(_queue, _mask);

    poped = _queue -> queue;
    if(poped)
        _queue -> queue = poped -> next;

    return poped;
}

