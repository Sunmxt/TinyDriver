#ifndef TINY_DRIVER_GPIO
#define TINY_DRIVER_GPIO

#include "tiny_driver.h"


/* -------------------- Status -------------------- */
#define TDRV_STATUS_GPIO        TDRV_STATUS_USER
/* ------------------------------------------------ */

typedef TDRVStatus (*TDrvGPIOPinConfigure)(TDevice *_ha_instance, Word _index, Word _mode);
#define TDRV_GPIO_INPUT     0x0001
#define TDRV_GPIO_OUTPUT    0x0000
#define TDRV_GPIO_ANALOG    0x0002
#define TDRV_GPIO_MODE_MASK 0x0003

#define TDRV_GPIO_PULL_MODE_MASK    0x000C
#define TDRV_GPIO_FLOATING          0x0000
#define TDRV_GPIO_PULL_UP           0x0004
#define TDRV_GPIO_PULL_DOWN         0x0008

#define TDRV_GPIO_OUTPUT_MODE_MASK  0x0010
#define TDRV_GPIO_OPEN_DRAIN        0x0010
#define TDRV_GPIO_PUSH_PULL         0x0000


#define TDRV_GPIO_PERIPHERAL_MASK       0x01E0
#define TDRV_GPIO_PERIPHERAL_BIT        5
#define TDRV_GPIO_PERIPHERAL_BASE       0x0020
#define TDRV_GPIO_PERIPHERAL_SYS        0x0000
#define TDRV_GPIO_PERIPHERAL_1          TDRV_GPIO_PERIPHERAL_SYS + TDRV_GPIO_PERIPHERAL_BASE
#define TDRV_GPIO_PERIPHERAL_2          TDRV_GPIO_PERIPHERAL_1 + TDRV_GPIO_PERIPHERAL_BASE
#define TDRV_GPIO_PERIPHERAL_3          TDRV_GPIO_PERIPHERAL_2 + TDRV_GPIO_PERIPHERAL_BASE
#define TDRV_GPIO_PERIPHERAL_4          TDRV_GPIO_PERIPHERAL_3 + TDRV_GPIO_PERIPHERAL_BASE
#define TDRV_GPIO_PERIPHERAL_5          TDRV_GPIO_PERIPHERAL_4 + TDRV_GPIO_PERIPHERAL_BASE 
#define TDRV_GPIO_PERIPHERAL_6          TDRV_GPIO_PERIPHERAL_5 + TDRV_GPIO_PERIPHERAL_BASE 
#define TDRV_GPIO_PERIPHERAL_7          TDRV_GPIO_PERIPHERAL_6 + TDRV_GPIO_PERIPHERAL_BASE 
#define TDRV_GPIO_PERIPHERAL_8          TDRV_GPIO_PERIPHERAL_7 + TDRV_GPIO_PERIPHERAL_BASE 
#define TDRV_GPIO_PERIPHERAL_9          TDRV_GPIO_PERIPHERAL_8 + TDRV_GPIO_PERIPHERAL_BASE 
#define TDRV_GPIO_PERIPHERAL_10         TDRV_GPIO_PERIPHERAL_9 + TDRV_GPIO_PERIPHERAL_BASE 
#define TDRV_GPIO_PERIPHERAL_11         TDRV_GPIO_PERIPHERAL_10 + TDRV_GPIO_PERIPHERAL_BASE 
#define TDRV_GPIO_PERIPHERAL_12         TDRV_GPIO_PERIPHERAL_11 + TDRV_GPIO_PERIPHERAL_BASE 
#define TDRV_GPIO_PERIPHERAL_13         TDRV_GPIO_PERIPHERAL_12 + TDRV_GPIO_PERIPHERAL_BASE 
#define TDRV_GPIO_PERIPHERAL_14         TDRV_GPIO_PERIPHERAL_13 + TDRV_GPIO_PERIPHERAL_BASE 
#define TDRV_GPIO_PERIPHERAL_15         TDRV_GPIO_PERIPHERAL_14 + TDRV_GPIO_PERIPHERAL_BASE 
#define TDRV_GPIO_PERIPHERAL_16         TDRV_GPIO_PERIPHERAL_15 + TDRV_GPIO_PERIPHERAL_BASE 

#define TDRV_GPIO_SPEED_MASK            0x0600
#define TDRV_GPIO_LOW_SPEED             0x0000
#define TDRV_GPIO_MEDIUM_SPEED          0x0200
#define TDRV_GPIO_HIGH_SPEED            0x0400
#define TDRV_GPIO_SUPER_SPEED           0x0600
typedef TDRVStatus (*TDrvGPIOPinLock)(TDevice *_ha_instance, Word _index);
typedef TDRVStatus (*TDrvGPIOPinUnlock)(TDevice *_ha_instance, Word _index);
//typedef Uint (*TDrvGPIOPinSet)(TDevice *_ha_instance, Uint _index);
//typedef Uint (*TDrvGPIOPinReset)(TDevice *_ha_instance, Uint _index);
//typedef Uint (*TDrvGPIOPinRead)(TDevice *_ha_instance, Uint _index);


typedef struct Tiny_Driver_GPIO_Interface
{
    TDrvHAInit init;
    TDrvHADeinit deinit;
    TDrvGPIOPinConfigure config;

//    TDrvGPIOPinLock lock;
//    TDrvGPIOPinUnlock unlock;
//    TDrvGPIOPinSet set;
//    TDrvGPIOPinReset reset;
}TDrvGPIOInterface;

#define TDRV_GPIO_API(_gpio_periph_index) TDRV_API(TDrvGPIOInterface, _gpio_periph_index)

#endif
