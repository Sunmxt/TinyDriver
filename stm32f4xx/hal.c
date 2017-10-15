/*

    STM32F4xx Platform Hardware Abstract Driver & Device Description

*/
#include "../tiny_driver.h"
#include "hal_tab.h"
#include "gpio.h"
//#include "timer.h"
#include "dma.h"
#include "i2c.h"
#include "std/inc/stm32f4xx_gpio.h"


/* GPIO */
GPIOInfoSTM32F4 GPIOAInfo = {16, GPIOA};
GPIOInfoSTM32F4 GPIOBInfo = {16, GPIOB};
GPIOInfoSTM32F4 GPIOCInfo = {16, GPIOC};
GPIOInfoSTM32F4 GPIODInfo = {16, GPIOD};
GPIOInfoSTM32F4 GPIOEInfo = {16, GPIOE};

/*
//Timer 
TDrvTimerInfo TIM1Info = {TIM1, 0};
TDrvTimerInfo TIM2Info = {TIM2, 0};
TDrvTimerInfo TIM3Info = {TIM3, 0};
TDrvTimerInfo TIM4Info = {TIM4, 0};
//TDrvTimerInfo TIM5Info = {&RCC -> APB1ENR, 3, TIM5, 0};
//TDrvTimerInfo TIM6Info = {&RCC -> APB1ENR, 4, TIM6, 0};
//TDrvTimerInfo TIM7Info = {&RCC -> APB1ENR, 5, TIM7, 0};
//TDrvTimerInfo TIM8Info = {&RCC -> APB2ENR, 1, TIM8, 0};
*/

// DMA 
TDrvDMAInfo DMA1Info = 
	{
		DMA1
		, {DMA1_Stream0, DMA1_Stream1, DMA1_Stream2, DMA1_Stream3
        , DMA1_Stream4, DMA1_Stream5, DMA1_Stream6, DMA1_Stream7}
        , {0, 0, 0, 0, 0, 0, 0, 0}
        , {0, 0, 0, 0, 0, 0, 0, 0}
        , {0, 0, 0, 0, 0, 0, 0, 0}
	};
TDrvDMAInfo DMA2Info = 
    {
        DMA2
        , {DMA2_Stream0, DMA2_Stream1, DMA2_Stream2, DMA2_Stream3
        , DMA2_Stream4, DMA2_Stream5, DMA2_Stream6, DMA2_Stream7}
        , {0, 0, 0, 0, 0, 0, 0, 0}
        , {0, 0, 0, 0, 0, 0, 0, 0}
        , {0, 0, 0, 0, 0, 0, 0, 0}
    };

//I2C

TDrvI2CMeta I2C1Info = {I2C1, 0, 0, 0, 0, {CQFT_U8, 0, 0, 0}, 0};
TDrvI2CMeta I2C3Info = {I2C3, 0, 0, 0, 0, {CQFT_U8, 0, 0, 0}, 0};

/* Peripheral Infomation */
TDevice Peripherals[] = {
    {"GPIOA", &GPIOAInfo, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_GPIO, (TDrvHAInterface*)&GPIOInterface}}
    , {"GPIOB", &GPIOBInfo, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_GPIO, (TDrvHAInterface*)&GPIOInterface}}
    , {"GPIOC", &GPIOCInfo, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_GPIO, (TDrvHAInterface*)&GPIOInterface}}
    , {"GPIOD", &GPIODInfo, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_GPIO, (TDrvHAInterface*)&GPIOInterface}}
    , {"GPIOE", &GPIOEInfo, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_GPIO, (TDrvHAInterface*)&GPIOInterface}}
    , {"DMA1", &DMA1Info, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_DMA_TYPE1, (TDrvHAInterface*)&DMAInterface}}
    , {"DMA2", &DMA2Info, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_DMA_TYPE1, (TDrvHAInterface*)&DMAInterface}}
    , {"I2C1", &I2C1Info, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_I2C, (TDrvHAInterface*)&I2CInterface}}
    , {"I2C3", &I2C3Info, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_I2C, (TDrvHAInterface*)&I2CInterface}}
//    , {"TIM1", &TIM1Info, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_TIMER, (TDrvHAInterface*)&TimerInterface}}
//    , {"TIM2", 7, &TIM2Info, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_TIMER, (TDrvHAInterface*)&TimerInterface}}
//    , {"TIM3", 8, &TIM3Info, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_TIMER, (TDrvHAInterface*)&TimerInterface}}
//    , {"TIM4", 9, &TIM4Info, {TDRV_DIV_TINY_DRIVER, 0x0100, TDRV_IT_TDRV_TIMER, (TDrvHAInterface*)&TimerInterface}}
    , {0, 0, {0, 0, 0, 0}}
};

TDevice *tdrv_stm32f4_dma_irq_assoc[2] = { Peripherals + DMA_1, Peripherals + DMA_2};
TDevice *PeripheralTable = Peripherals;

void rbsky_hal_pin_init(void)
{
    // GPIO A Configure
    // USART2 TX
    TDRV_GPIO_API(GPIO_A).config(PeripheralTable + GPIO_A, 2, TDRV_GPIO_PERIPHERAL_8|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PUSH_PULL);
    // USART2 RX
    TDRV_GPIO_API(GPIO_A).config(PeripheralTable + GPIO_A, 3, TDRV_GPIO_PERIPHERAL_8|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_OPEN_DRAIN|TDRV_GPIO_PULL_UP);
    // USART2 CK
    TDRV_GPIO_API(GPIO_A).config(PeripheralTable + GPIO_A, 4, TDRV_GPIO_PERIPHERAL_8|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PUSH_PULL);
    // TIM3_CH1 (Motor Output 1)
    TDRV_GPIO_API(GPIO_A).config(PeripheralTable + GPIO_A, 6, TDRV_GPIO_PERIPHERAL_3|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PULL_UP|TDRV_GPIO_OPEN_DRAIN);
    // TIM3_CH2 (Motor Output 2)
    TDRV_GPIO_API(GPIO_A).config(PeripheralTable + GPIO_A, 7, TDRV_GPIO_PERIPHERAL_3|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PULL_UP|TDRV_GPIO_OPEN_DRAIN);
    // I2C3 SCL (User I2C Port)
    TDRV_GPIO_API(GPIO_A).config(PeripheralTable + GPIO_A, 8, TDRV_GPIO_PERIPHERAL_5|TDRV_GPIO_MEDIUM_SPEED|TDRV_GPIO_OPEN_DRAIN);
    // USB DM
    TDRV_GPIO_API(GPIO_A).config(PeripheralTable + GPIO_A, 11, TDRV_GPIO_PERIPHERAL_10|TDRV_GPIO_SUPER_SPEED);
    // USB DP
    TDRV_GPIO_API(GPIO_A).config(PeripheralTable + GPIO_A, 12, TDRV_GPIO_PERIPHERAL_10|TDRV_GPIO_SUPER_SPEED);

    // TIM3_CH3 (Motor Output 3)
    TDRV_GPIO_API(GPIO_B).config(PeripheralTable + GPIO_B, 0, TDRV_GPIO_PERIPHERAL_2|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PULL_UP|TDRV_GPIO_OPEN_DRAIN);
    // TIM3_CH4 (Motor Output 4)
    TDRV_GPIO_API(GPIO_B).config(PeripheralTable + GPIO_B, 1, TDRV_GPIO_PERIPHERAL_2|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PULL_UP|TDRV_GPIO_OPEN_DRAIN);
    // I2C1 SCL (Onbroad I2C Bus)
    TDRV_GPIO_API(GPIO_B).config(PeripheralTable + GPIO_B, 6, TDRV_GPIO_PERIPHERAL_5|TDRV_GPIO_MEDIUM_SPEED|TDRV_GPIO_OPEN_DRAIN);
    // I2C1 SDA (Onbroad I2C Bus)
    TDRV_GPIO_API(GPIO_B).config(PeripheralTable + GPIO_B, 7, TDRV_GPIO_PERIPHERAL_5|TDRV_GPIO_MEDIUM_SPEED|TDRV_GPIO_OPEN_DRAIN);
    //TDRV_GPIO_API(GPIO_E).config(PeripheralTable + GPIO_B, 7, TDRV_GPIO_OUTPUT|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PUSH_PULL);
    //GPIO_SetBits(GPIOB, 7);
    
    // TIM2_CH3 (Motor Output 5)
    TDRV_GPIO_API(GPIO_B).config(PeripheralTable + GPIO_B, 10, TDRV_GPIO_PERIPHERAL_2|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PULL_UP|TDRV_GPIO_OPEN_DRAIN);
    // TIM2_CH4 (Motor Output 6)
    TDRV_GPIO_API(GPIO_B).config(PeripheralTable + GPIO_B, 11, TDRV_GPIO_PERIPHERAL_2|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PULL_UP|TDRV_GPIO_OPEN_DRAIN);

    // I2C3 SDA (User I2C Port)
    TDRV_GPIO_API(GPIO_C).config(PeripheralTable + GPIO_C, 9, TDRV_GPIO_PERIPHERAL_5|TDRV_GPIO_MEDIUM_SPEED|TDRV_GPIO_OPEN_DRAIN);
    // SPI3 SCK
    TDRV_GPIO_API(GPIO_C).config(PeripheralTable + GPIO_C, 10, TDRV_GPIO_PERIPHERAL_7|TDRV_GPIO_HIGH_SPEED|TDRV_GPIO_PUSH_PULL);
    // SPI3 SDI
    TDRV_GPIO_API(GPIO_C).config(PeripheralTable + GPIO_C, 11, TDRV_GPIO_PERIPHERAL_7|TDRV_GPIO_HIGH_SPEED|TDRV_GPIO_OPEN_DRAIN|TDRV_GPIO_PULL_UP);
    // SPI3 SDO
    TDRV_GPIO_API(GPIO_C).config(PeripheralTable + GPIO_C, 12, TDRV_GPIO_PERIPHERAL_7|TDRV_GPIO_HIGH_SPEED|TDRV_GPIO_PUSH_PULL);

    // USART3 TX
    TDRV_GPIO_API(GPIO_D).config(PeripheralTable + GPIO_D, 8, TDRV_GPIO_PERIPHERAL_8|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PUSH_PULL);
    // USART3 RX
    TDRV_GPIO_API(GPIO_D).config(PeripheralTable + GPIO_D, 9, TDRV_GPIO_PERIPHERAL_8|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_OPEN_DRAIN|TDRV_GPIO_PULL_UP);
    // USART3_CK
    TDRV_GPIO_API(GPIO_D).config(PeripheralTable + GPIO_D, 10, TDRV_GPIO_PERIPHERAL_8|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PUSH_PULL);
    // TIM4_CH1 (Motor Output 7)
    TDRV_GPIO_API(GPIO_D).config(PeripheralTable + GPIO_D, 12, TDRV_GPIO_PERIPHERAL_3|TDRV_GPIO_OPEN_DRAIN|TDRV_GPIO_PULL_UP);
    // TIM4_CH2 (Motor Output 8)
    TDRV_GPIO_API(GPIO_D).config(PeripheralTable + GPIO_D, 13, TDRV_GPIO_PERIPHERAL_3|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_OPEN_DRAIN|TDRV_GPIO_PULL_UP);

    // TIM1_CH1 (PWM Input 1)
    TDRV_GPIO_API(GPIO_E).config(PeripheralTable + GPIO_E, 9, TDRV_GPIO_PERIPHERAL_2|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_OPEN_DRAIN|TDRV_GPIO_PULL_UP);
    // TIM1_CH2 (PWM Input 2)
    TDRV_GPIO_API(GPIO_E).config(PeripheralTable + GPIO_E, 11, TDRV_GPIO_PERIPHERAL_2|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_OPEN_DRAIN|TDRV_GPIO_PULL_UP);
    // TIM1_CH3 (PWM Input 3)
    TDRV_GPIO_API(GPIO_E).config(PeripheralTable + GPIO_E, 13, TDRV_GPIO_PERIPHERAL_3|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_OPEN_DRAIN|TDRV_GPIO_PULL_UP);
    // TIM1_CH4 (PWM Input 4)
    TDRV_GPIO_API(GPIO_E).config(PeripheralTable + GPIO_E, 14, TDRV_GPIO_PERIPHERAL_4|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_OPEN_DRAIN|TDRV_GPIO_PULL_UP);
    // Status LED
    TDRV_GPIO_API(GPIO_E).config(PeripheralTable + GPIO_E, 15, TDRV_GPIO_OUTPUT|TDRV_GPIO_LOW_SPEED|TDRV_GPIO_PUSH_PULL);
}

TDRVStatus PlatformHALInit(void)
{
    TDRVStatus status;

    status = TinyDriverLoadAll();

    if( TDRV_OK == status )
        rbsky_hal_pin_init();

    return status;
}

TDRVStatus PlatformHALClean(void)
{
    TDRVStatus status;

    status = TinyDriverUnloadAll();

    return status;
}

