/*
    
    Tiny Driver

    Simple and Scalable Hardward Abstract Framework for Embedded Devices.

*/

#ifndef TINY_DRIVER_HEADERS
#define TINY_DRIVER_HEADERS

/* Compile time assert */

#include "atomic.h"

typedef unsigned char Byte;
typedef unsigned int Uint;
typedef unsigned short Word;
typedef unsigned long Dword;
typedef unsigned int size_t;
typedef int Bool;


/* ------------------- Status -------------------- */
typedef int TDRVStatus;
#define TDRV_OK                         0
#define TDRV_BUSY                       -1
#define TDRV_NOT_SUPPORTED              -2
#define TDRV_INVAILD_PARAMETER          -3
#define TDRV_ALREADY_EXIST              -4
#define TDRV_ILLEGAL_STATE              -5

#define TDRV_WARN_DATA_CUT              -1

#define TDRV_STATUS_ERROR_USER          -1000
#define TDRV_STATUS_WARNING_USER        1000
/* ----------------------------------------------- */

typedef struct _Tiny_Driver_Device
{
    const char* name;
    void* private_data;

    struct _Driver_Interface
    {
        const char* vendor;
        #define TDRV_DIV_UNKNOWN        0
        #define TDRV_DIV_TINY_DRIVER    1
        #define TDRV_DIV_OTHERS         10

        uint16_t version;
        uint16_t type;
        #define TDRV_IT_TDRV_GPIO       1
        #define TDRV_IT_TDRV_TIMER      2
        #define TDRV_IT_TDRV_DMA        3
        #define TDRV_IT_TDRV_I2C        4
        #define TDRV_IT_SENSOR          5
        
        struct _Tiny_Driver_Hardware_Abstract_Interface *interfaces;
    }driver;
}TDevice, *TDeviceTable;

typedef TDRVStatus (*TDrvHAInit)(TDevice *_ha_instance);
typedef TDRVStatus (*TDrvHADeinit)(TDevice *_ha_instance);

typedef struct _Tiny_Driver_Hardware_Abstract_Interface
{
    TDrvHAInit init;
    TDrvHADeinit deinit;
}TDrvHAInterface;


// #define TDRV_API(_interface_type, _index) (*((_interface_type*)PeripheralTable[_index].driver.interfaces))
#define TDRV_BASIC_API(_device) (*((TDrvHAInterface*)((TDevice*)_device) -> driver.interfaces))
#define TDRV_API(_interface_type, _device_ptr) (*((_interface_type*)((TDevice*)_device_ptr) -> driver.interfaces))

TDRVStatus TinyDriverLoad(TDevice *_instance);
TDRVStatus TinyDriverUnload(TDevice *_instance);
TDRVStatus TinyDriverLoadAll(void);
TDRVStatus TinyDriverUnloadAll(void);

#endif
