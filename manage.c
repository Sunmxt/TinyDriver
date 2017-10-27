#include "manage.h"
#include "atomic.h"

TDRVStatus lsnr_hub_create(TDrvListenerHub *_hub)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_hub)
            return TDRV_INVAILD_PARAMETER;
    #endif

    _hub -> flags = 0;
    _hub -> lsnr = 0;
    return TDRV_OK;
}

TDRVStatus lsnr_hub_listener_connect(TDrvListenerHub *_hub, TDrvHubListenerBase *_listener)
{
    for_list_node *chg;
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_hub || !_listener)
            return TDRV_INVAILD_PARAMETER;
    #endif

    _listener -> flags = 0;
    do
    {
        _listener -> node.next = _hub -> lsnr;
        chg = (for_list_node*)TDrvCASPointer(&_hub -> lsnr, _listener -> node.next, &_listener -> node);
    }while(chg != _listener -> node.next);

    _listener -> event_handler(_hub, _listener, TDRV_LSNR_ATTACHED, 0);

    return TDRV_OK;
}

void lsnr_hub_remove_disconnected(TDrvListenerHub *_hub)
{
    for_list_node *lsnr_node;
    TDrvHubListenerBase *lsnr;
    
    lsnr_node = _hub -> lsnr;

    // Find new head.
    lsnr_node = _hub -> lsnr; 
    while(lsnr_node && !(TDRV_TO_HUB_LISTENER(lsnr_node) -> flags & TDRV_LSNR_DISCONNECTED))
    {
        lsnr = TDRV_TO_HUB_LISTENER(lsnr_node -> next);
        lsnr_node = lsnr_node -> next;
        lsnr -> event_handler(_hub, lsnr, TDRV_LSNR_DETACHED, 0);
    }
    _hub -> lsnr = lsnr_node;

    // Remove others
    while(lsnr_node && lsnr_node -> next)
    {
        lsnr = TDRV_TO_HUB_LISTENER(lsnr_node -> next);
        if(lsnr -> flags & TDRV_LSNR_DISCONNECTED)
        {
            lsnr_node -> next = lsnr_node -> next -> next;
            lsnr -> event_handler(_hub, TDRV_TO_HUB_LISTENER(lsnr_node), TDRV_LSNR_DISCONNECTED, 0);
        }
        else
            lsnr_node = lsnr_node -> next;
    }

    return;
}

void lsnr_hub_unlock_and_recover_pend(TDrvListenerHub *_hub)
{
    uint8_t chg8, old8, new8;

    for(;;)
    {
        chg8 = _hub -> flags;
        do
        {
            old8 = chg8;
            if(!(old8 & TDRV_LSNR_HUB_BUSY))
                break;

            if(old8 & TDRV_PEND_DISCONNECT)
                new8 = old8 & (~TDRV_PEND_DISCONNECT);

            chg8 = TDrvCAS8(&_hub -> flags, old8, new8);

        }while(old8 != chg8);

        if(chg8 & TDRV_PEND_DISCONNECT)
            lsnr_hub_remove_disconnected(_hub);
        else
            break;
    }
}

TDRVStatus lsnr_hub_notify(TDrvListenerHub *_hub, uint16_t _event, void *_param)
{
    uint8_t chg8, old8;
    for_list_node *lsnr_node;
    TDrvHubListenerBase *lsnr;

    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_hub)
            return TDRV_INVAILD_PARAMETER;
    #endif

    lsnr_node = _hub -> lsnr;
    if(!lsnr_node)
        return TDRV_OK;

    // Lock list.
    chg8 = _hub -> flags;
    do
    {
        old8 = chg8;
        if(old8 & TDRV_LSNR_HUB_BUSY)
            return TDRV_BUSY;
        chg8 = TDrvCAS8(&_hub -> flags, old8, old8 | TDRV_LSNR_HUB_BUSY);
    }while(chg8 != old8);

    do
    {
        lsnr = TDRV_TO_HUB_LISTENER(lsnr_node);
        lsnr -> event_handler(_hub, lsnr, _event, _param); 
        lsnr_node = lsnr_node -> next;
    }while(lsnr_node);

    // Unlock list
    lsnr_hub_unlock_and_recover_pend(_hub);

    return TDRV_OK;
}

TDRVStatus lsnr_hub_listener_disconnect(TDrvListenerHub *_hub, TDrvHubListenerBase *_listener)
{
    uint8_t chg8, old8, new8;

    #ifdef TDRV_NO_PARAMETER_CHECK
        if(!_hub || !_listener)
            return TDRV_INVAILD_PARAMETER;
    #endif

    _listener -> event_handler(_hub, _listener, TDRV_LSNR_DETACHING, 0);

    _listener -> flags |= TDRV_LSNR_DISCONNECTED;
    chg8 = _hub -> flags;
    do
    {
        old8 = chg8;
        if(old8 & TDRV_LSNR_HUB_BUSY)
            new8 = old8 | TDRV_PEND_DISCONNECT;
        else
            new8 |= TDRV_PEND_DISCONNECT;
        chg8 = TDrvCAS8(&_hub -> flags, old8, new8);
    }while(chg8 != old8);

    lsnr_hub_unlock_and_recover_pend(_hub);

    return TDRV_OK;
}
