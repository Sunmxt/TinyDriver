
#include "gpio.h"
#include "stm32f4xx.h"
#include "std/inc/stm32f4xx_rcc.h"
#include "std/inc/stm32f4xx_gpio.h"

TDRVStatus tdrv_gpio_ha_init(TDevice *_ha_instance);
TDRVStatus tdrv_gpio_ha_deinit(TDevice *_ha_instance);
TDRVStatus gpio_pin_configure(TDevice *_ha_instance, Word _index, Word _mode);

TDrvGPIOInterface GPIOInterface = {
        tdrv_gpio_ha_init,
        tdrv_gpio_ha_deinit,
        gpio_pin_configure
    };

TDRVStatus tdrv_gpio_ha_init(TDevice *_ha_instance)
{
    GPIOInfoSTM32F4 *info;
    
    info = (GPIOInfoSTM32F4*) _ha_instance -> private_data;

    switch( (uint32_t)info -> regs )
    {
    case GPIOA_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); break;
    case GPIOB_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); break;
    case GPIOC_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); break;
    case GPIOD_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); break;
    case GPIOE_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); break;
    case GPIOF_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE); break;
    case GPIOG_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE); break;
    case GPIOH_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE); break;
    case GPIOI_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE); break;
    case GPIOJ_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOJ, ENABLE); break;
    }

    GPIO_DeInit(info -> regs);

    return TDRV_OK;
}

TDRVStatus tdrv_gpio_ha_deinit(TDevice *_ha_instance)
{
    GPIOInfoSTM32F4 *info;
    info = (GPIOInfoSTM32F4*) _ha_instance -> private_data;

    switch((uint32_t)info -> regs)
    {
    case GPIOA_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, DISABLE); break;
    case GPIOB_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, DISABLE); break;
    case GPIOC_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, DISABLE); break;
    case GPIOD_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, DISABLE); break;
    case GPIOE_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, DISABLE); break;
    case GPIOF_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, DISABLE); break;
    case GPIOG_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, DISABLE); break;
    case GPIOH_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, DISABLE); break;
    case GPIOI_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, DISABLE); break;
    case GPIOJ_BASE:
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOJ, DISABLE); break;
    }

    return TDRV_OK;
}

TDRVStatus gpio_pin_configure(TDevice *_ha_instance, Word _index, Word _mode)
{
    GPIOInfoSTM32F4 *info;
    GPIO_InitTypeDef init;
    
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(_index >= 16 )
            return TDRV_INVAILD_PARAMETER;
    #endif

    info = (GPIOInfoSTM32F4*) _ha_instance -> private_data;
    init.GPIO_Pin = ((uint16_t)0x0001) << _index;

    //configure pull mode
    switch(_mode & TDRV_GPIO_PULL_MODE_MASK)
    {
    case TDRV_GPIO_FLOATING:
        init.GPIO_PuPd = GPIO_PuPd_NOPULL; break;
    case TDRV_GPIO_PULL_UP:
        init.GPIO_PuPd = GPIO_PuPd_UP; break;
    case TDRV_GPIO_PULL_DOWN:
        init.GPIO_PuPd = GPIO_PuPd_DOWN; break;
    }

    //configure io
    if(_mode & TDRV_GPIO_PERIPHERAL_MASK)
    {
        init.GPIO_Mode = GPIO_Mode_AF;
        if( _index > 7)
        {
            _index -= 8;
            info -> regs ->AFR[1] = ((info -> regs -> AFR[1] & ~(0xF << (_index << 2))) | ((((_mode & TDRV_GPIO_PERIPHERAL_MASK) >> TDRV_GPIO_PERIPHERAL_BIT) - 1 & 0xF) << (_index << 2)));
        }
        else
            info -> regs -> AFR[0] = ((info -> regs -> AFR[0] & ~(0xF << (_index << 2))) | ((((_mode & TDRV_GPIO_PERIPHERAL_MASK) >> TDRV_GPIO_PERIPHERAL_BIT) - 1 & 0xF) << (_index << 2)));
    }
    else
    {
        switch(_mode & TDRV_GPIO_MODE_MASK)
        {
        case TDRV_GPIO_INPUT:
            init.GPIO_Mode = GPIO_Mode_IN; break;
        case TDRV_GPIO_OUTPUT:
            init.GPIO_Mode = GPIO_Mode_OUT; break;
        case TDRV_GPIO_ANALOG:
            init.GPIO_Mode = GPIO_Mode_AN; break;
        }
    }

    //configure output type
    
    if(_mode & TDRV_GPIO_OPEN_DRAIN)
        init.GPIO_OType = GPIO_OType_OD;
    else
        init.GPIO_OType = GPIO_OType_PP;

    //configure speed
    switch(_mode & TDRV_GPIO_SPEED_MASK)
    {
    case TDRV_GPIO_LOW_SPEED:
        init.GPIO_Speed = GPIO_Low_Speed; break;
    case TDRV_GPIO_MEDIUM_SPEED:
        init.GPIO_Speed = GPIO_Medium_Speed; break;
    case TDRV_GPIO_HIGH_SPEED:
        init.GPIO_Speed = GPIO_Fast_Speed; break;
    case TDRV_GPIO_SUPER_SPEED:
        init.GPIO_Speed = GPIO_High_Speed; break;
    }


    GPIO_Init(info -> regs, &init);

    return TDRV_OK;
}
