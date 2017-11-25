#ifndef TINY_DRIVER_MANAGE
#define TINY_DRIVER_MANAGE

#include <stdint.h>
#include "klist.h"
#include "tiny_driver.h"

/*
    Listener Hub Manager.
*/

typedef struct _Tiny_Driver_Listener_Hub
{
    uint8_t flags;
    #define TDRV_LSNR_HUB_BUSY      0x01
    #define TDRV_PEND_DISCONNECT    0x02
    #define TDRV_PEND_NOTIFICATION  0x04 

    for_list_node *lsnr;
    
}TDrvListenerHub;

typedef struct _Tiny_Driver_Listener_Base
{
    #define TDRV_HUB_LISTENER_COMMON_HEADER()   \
        uint8_t flags;                          \
        for_list_node node;                     \
        void* (*event_handler)(TDrvListenerHub *_hub, struct _Tiny_Driver_Listener_Base *listener, uint16_t _event, void* _params)
            #define TDRV_LSNR_ATTACHED          1
            #define TDRV_LSNR_DETACHING         2
            #define TDRV_LSNR_DETACHED          3
            #define TDRV_LSNR_USER_MSG          100
            
    #define TDRV_LSNR_DISCONNECTED  0x01

    TDRV_HUB_LISTENER_COMMON_HEADER();
}TDrvHubListenerBase;

struct _Tiny_Driver_Listener_Hub;


#define TDRV_TO_HUB_LISTENER(_pointer) S_LIST_TO_DATA(_pointer, TDrvHubListenerBase, node)

TDRVStatus lsnr_hub_listener_connect(TDrvListenerHub *_hub, TDrvHubListenerBase *_listener);
TDRVStatus lsnr_hub_listener_disconnect(TDrvListenerHub *_hub, TDrvHubListenerBase *_listener);
TDRVStatus lsnr_hub_notify(TDrvListenerHub *_hub, uint16_t _event, void* _param);
TDRVStatus lsnr_hub_create(TDrvListenerHub *_hub);
TDRVStatus lsnr_hub_destroy(TDrvListenerHub *_hub);

/* 
    Critical Queue 
*/

#endif
