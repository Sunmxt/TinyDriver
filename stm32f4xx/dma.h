#ifndef TINY_DRIVER_DMA_STM32F4
#define TINY_DRIVER_DMA_STM32F4

#include "../dma.h"
#include "stm32f4xx_dma.h"

typedef struct _Tiny_Driver_DMA_Infomation_STM32F4
{
    DMA_TypeDef *regs;
    DMA_Stream_TypeDef *streams[8];
    Word init_flags[8];
    #define TDRV_DMA_COMMON_FLAGS_MASK  0x00002FFF
    #define TDRV_DMA_CHANNEL_MASK       0x001C000
    #define TDRV_DMA_CHANNEL_BIT        14

    TDrvDMACallback *callback[8];
    void* callback_user_param[8];
    uint8_t stream_lock;
}TDrvDMAInfo;

extern TDrvDMAInterface DMAInterface;

#endif
