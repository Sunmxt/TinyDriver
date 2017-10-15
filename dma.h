#ifndef TINY_DRIVER_DMA
#define TINY_DRIVER_DMA

#include "tiny_driver.h"

/* Get DMA Infomation */
typedef struct _Tiny_Driver_DMA_Capability
{
    Byte channels_count;
    Byte stream_count;
    
    Word stream_capability;
    /* Direction capability */
    #define TDRV_DMA_CAP_MEM2MEM            0x01
    #define TDRV_DMA_CAP_MEM2DEV            0x02
    #define TDRV_DMA_CAP_DEV2MEM            0x04
    #define TDRV_DMA_CAP_DIRECTION_MASK     0x07
    /* Data Width capability */
    #define TDRV_DMA_CAP_8BIT               0x08
    #define TDRV_DMA_CAP_16BIT              0x10
    #define TDRV_DMA_CAP_32BIT              0x20
    #define TDRV_DMA_CAP_64BIT              0x40
    #define TDRV_DMA_CAP_128BIT             0x80
    #define TDRV_DMA_CAP_DATA_WIDTH         0xF0
    /* Double buffer capability */
    #define TDRV_DMA_CAP_DOUBLE_BUFFER      0x0100

    size_t max_buffer_size;
}TDrvDMACapability;

typedef TDRVStatus (*TDrvDMAGetFeature)(TDevice *_ha_instance, TDrvDMACapability *_capability);

/* DMA Stream Configure */
typedef TDRVStatus (*TDrvDMAStreamConnect)(TDevice *_ha_instance, Byte _stream, Byte _channel);
typedef TDRVStatus (*TDrvDMAStreamAlloc)(TDevice *_device, Byte _stream);
typedef TDRVStatus (*TDrvDMAStreamFree)(TDevice *_device, Byte _stream);
typedef TDRVStatus (*TDrvDMASetMode)(TDevice *_ha_instance, Byte _stream, Word _mode);
#define TDRV_DMA_DEV2MEM                0x0000
#define TDRV_DMA_MEM2DEV                0x0001
#define TDRV_DMA_MEM2MEM                0x0002
#define TDRV_DMA_DIRECTION_MASK         0x0003
#define TDRV_DMA_DIRECTION_BIT          0

#define TDRV_DMA_DEV_8BIT               0x0000
#define TDRV_DMA_DEV_16BIT              0x0004
#define TDRV_DMA_DEV_32BIT              0x0008
#define TDRV_DMA_DEV_64BIT              0x0010
#define TDRV_DMA_DEV_128BIT             0x0014
#define TDRV_DMA_DEV_DATA_WIDTH_MASK    0x001F
#define TDRV_DMA_DEV_DATA_WIDTH_BIT     2

#define TDRV_DMA_MEM_8BIT               0x0000
#define TDRV_DMA_MEM_16BIT              0x0020
#define TDRV_DMA_MEM_32BIT              0x0040
#define TDRV_DMA_MEM_64BIT              0x0060
#define TDRV_DMA_MEM_128BIT             0x0080
#define TDRV_DMA_MEM_DATA_WIDTH_MASK    0x00E0
#define TDRV_DMA_MEM_DATA_WIDTH_BIT     5

#define TDRV_DMA_MEM_INC                0x0100
#define TDRV_DMA_MEM_INC_BIT            8
#define TDRV_DMA_DEV_INC                0x0200
#define TDRV_DMA_DEV_INC_BIT            9
#define TDRV_DMA_REPEAT                 0x0400
#define TDRV_DMA_REPEAT_BIT             10
#define TDRV_DMA_DEV_TERMINATE          0x0800
#define TDRV_DMA_DEV_TERMINATE_BIT      11

#define TDRV_DMA_DOUBLE_BUFFER          0x1000
#define TDRV_DMA_DOUBLE_BUFFER_BIT      12

#define TDRV_DMA_LOCK_CONFIG            0x2000
#define TDRV_DMA_LOCK_CONFIG_BIT        13

/* DMA Start / Stop */
typedef TDRVStatus (*TDrvDMAStop)(TDevice *_ha_instance, Byte _stream);

/* DMA Send / Receive */
typedef TDRVStatus (*TDrvDMAStartTransfer)(TDevice *_ha_instance, Byte _stream, void* _buffer , void* _device_target, size_t _buffer_size);
typedef TDRVStatus (*TDrvDMASetSwapBuffer)(TDevice *_ha_instance, Byte _stream, void* _buffer);

/* State */
typedef size_t (*TDrvDMAGetBufferRemainSize)(TDevice *_ha_instance, Byte _stream);

/* DMA Callback */
#define TDRV_DMA_EVT_COMPLETED          1
#define TDRV_DMA_EVT_BUFFER_SWAPED      2
#define TDRV_DMA_HARDWARE_ERROR         3
typedef void* (TDrvDMACallback)(Byte _dma_event, void* _event_param, void* _user_param);
typedef TDRVStatus (*TDrvDMASetCallback)(TDevice *_ha_instance, Byte _stream, TDrvDMACallback _callback, void* _user_param);


typedef struct _Tiny_Driver_DMA_Interface
{
    TDrvHAInit init;
    TDrvHADeinit deinit;

    TDrvDMAStreamAlloc StreamAlloc;
    TDrvDMAStreamFree StreamFree;
    TDrvDMAGetFeature GetCapability;
    TDrvDMAStreamConnect StreamConnect;
    TDrvDMASetMode SetMode;
    TDrvDMAStop Stop;
    TDrvDMAStartTransfer StartTransfer;
    TDrvDMASetSwapBuffer SetSwapBuffer;
    TDrvDMAGetBufferRemainSize GetBufferRemainSize;
    TDrvDMASetCallback SetCallback;
}TDrvDMAInterface;

#define TDRV_DMA_API(_device_index) TDRV_API(TDrvDMAInterface, _device_index)


#endif
