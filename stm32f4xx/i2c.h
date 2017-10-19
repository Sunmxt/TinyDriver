#ifndef TINY_DRIVER_STM32F4XX_I2C
#define TINY_DRIVER_STM32F4XX_I2C

#include "..\i2c.h"
#include "..\dma.h"
#include "..\queue.h"
#include "hal_tab.h"

typedef struct _Tiny_Driver_I2C_Device_Meta
{
    I2C_TypeDef *regs;
    uint8_t flags;
    /*
        Low
            0b - 10-bit Address
            1b - 400kbps speed
            2b - i2c running
            3b - i2c new messages exist
            7b - hardware configured
        High
    */
    #define I2C_10BIT_ADDRESS       0x00
    #define I2C_400KBPS             0x01
    #define I2C_RUNNING             0x02
    #define I2C_NEW_MESSAGE         0x04
    #define I2C_DMA_SPLITED         0x08
    #define I2C_HARDWARE_CONFIGURED 0x80

    size_t split_size;
    size_t send_ptr;
    TDrvI2CMessage *running_msg;

    TDrvCriticalQueue queue;
    //bi_list_node *messages;
    //bi_list_node *add_msgs;
    int8_t dma_stream;
}TDrvI2CMeta;

/*
    I2C Message Private Field
*/


#define TDRV_I2C_FAILED_DMA_POS     (TDRV_I2C_MSG_PRIVATE_FLAGS_POS + 0)
#define TDRV_I2C_FINISH_POS         (TDRV_I2C_MSG_PRIVATE_FLAGS_POS + 1)

#define TDRV_I2C_FAILED_DMA         (1u << TDRV_I2C_FAILED_DMA_POS)
#define TDRV_I2C_FINISH             (1u << TDRV_I2C_FINISH_POS)

/*
    Struct extract macro
*/
#define EXTRACT_I2C_META(_device_ptr) ((TDrvI2CMeta*)_device_ptr -> private_data)
#endif


