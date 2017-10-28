/*
    
    I2C Bus driver for STM32F4XX MCUs.

*/


#include "std/inc/stm32f4xx_rcc.h"
#include "std/inc/stm32f4xx_i2c.h"
#include "std/inc/misc.h"
#include "i2c.h"
#include "../dma.h"

TDevice *i2c_devices_map[] = {0, 0, 0};

void* i2c_dma_callback(Byte _dma_event, void* _event_param, void* _user_param);

TDRVStatus i2c_dma_realloc_stream(TDrvI2CMeta *_meta, uint8_t _is_receive)
{
    TDRVStatus status;
    TDrvDMACapability dma_cap;
    Word dma_mode;
    
    // free stream
    if(_meta -> dma_stream >= 0)
        TDRV_DMA_API(_meta -> dma1_device).StreamFree(_meta -> dma1_device, _meta -> dma_stream);

    dma_mode = TDRV_DMA_MEM_8BIT|TDRV_DMA_DEV_8BIT|TDRV_DMA_MEM_INC;
    if(_is_receive)
    {
        switch((unsigned int)_meta -> regs)
        {
        case I2C1_BASE:
            if( TDRV_OK == TDRV_DMA_API(_meta -> dma1_device).StreamAlloc(_meta -> dma1_device, 0))
                _meta -> dma_stream = 0;
            else if( TDRV_OK != TDRV_DMA_API(_meta -> dma1_device).StreamAlloc(_meta -> dma1_device, 0))
                _meta -> dma_stream = 5;
            else
                return TDRV_BUSY;
            status = TDRV_DMA_API(_meta -> dma1_device).StreamConnect(_meta -> dma1_device, _meta -> dma_stream, 1);
            break;
        case I2C2_BASE:
            if( TDRV_OK == TDRV_DMA_API(_meta -> dma1_device).StreamAlloc(_meta -> dma1_device, 2))
                _meta -> dma_stream = 2;
            else if( TDRV_OK == TDRV_DMA_API(_meta -> dma1_device).StreamAlloc(_meta -> dma1_device, 3))
                _meta -> dma_stream = 3;
            else
                return TDRV_BUSY;
            status = TDRV_DMA_API(_meta -> dma1_device).StreamConnect(_meta -> dma1_device, _meta -> dma_stream, 7);
        case I2C3_BASE:
            if( TDRV_OK == TDRV_DMA_API(_meta -> dma1_device).StreamAlloc(_meta -> dma1_device, 2))
                _meta -> dma_stream = 2;
            else
                return TDRV_BUSY;
            status = TDRV_DMA_API(_meta -> dma1_device).StreamConnect(_meta -> dma1_device, _meta -> dma_stream, 3);
        }
        if(TDRV_OK != status)
            return status;
        dma_mode |= TDRV_DMA_DEV2MEM;
    }
    else
    {

        switch((unsigned int)_meta -> regs)
        {
        case I2C1_BASE:
            if( TDRV_OK == TDRV_DMA_API(_meta -> dma1_device).StreamAlloc(_meta -> dma1_device, 6))
                _meta -> dma_stream = 6;
            else if(TDRV_OK == TDRV_DMA_API(_meta -> dma1_device).StreamAlloc(_meta -> dma1_device, 7))
                _meta -> dma_stream = 7;
            else
                return TDRV_BUSY;
            status = TDRV_DMA_API(_meta -> dma1_device).StreamConnect(_meta -> dma1_device, _meta -> dma_stream, 1);
            break;
        case I2C2_BASE:
            if( TDRV_OK == TDRV_DMA_API(_meta -> dma1_device).StreamAlloc(_meta -> dma1_device, 7))
                _meta -> dma_stream = 7;
            else
                return TDRV_BUSY;
            status = TDRV_DMA_API(_meta -> dma1_device).StreamConnect(_meta -> dma1_device, _meta -> dma_stream, 7);
            break;
        case I2C3_BASE:
            if( TDRV_OK == TDRV_DMA_API(_meta -> dma1_device).StreamAlloc(_meta -> dma1_device, 4))
                _meta -> dma_stream = 4;
            else
                return TDRV_BUSY;
            status = TDRV_DMA_API(_meta -> dma1_device).StreamConnect(_meta -> dma1_device, _meta -> dma_stream, 3);
            break;
        }
        dma_mode |= TDRV_DMA_MEM2DEV;
    }

    status = TDRV_DMA_API(_meta -> dma1_device).SetMode(_meta -> dma1_device, _meta -> dma_stream, dma_mode);
    TDRV_DMA_API(_meta -> dma1_device).SetCallback(_meta -> dma1_device, _meta -> dma_stream, i2c_dma_callback, _meta);
    TDRV_DMA_API(_meta -> dma1_device).GetCapability(_meta -> dma1_device, &dma_cap);

    _meta -> split_size = dma_cap.max_buffer_size;

    return status;
}

TDRVStatus i2c_dma_load_message(TDrvI2CMeta *_meta, size_t *_loaded_size)
{
    TDRVStatus status;
    size_t new_size;

    status = TDRV_OK;
    
    if(_meta -> split_size < _meta -> running_msg -> size - _meta -> send_ptr)
        new_size = _meta -> split_size;
    else
    {
        new_size = _meta -> running_msg -> size - _meta -> send_ptr;
        if(_meta -> running_msg -> address & 0x0001)
        {
            if(new_size <= 2)
            {
                //if(new_size == 1)
                    new_size = 0;
                I2C_ITConfig(_meta -> regs, I2C_IT_BUF, ENABLE);
                
            }
            else
                new_size -= 2;
        }
        else
            // Last byte is handled by I2C Event Handler.
            new_size--;
    }
    
    if(new_size)
    {
        status = TDRV_DMA_API(_meta -> dma1_device).StartTransfer(_meta -> dma1_device, _meta -> dma_stream
                    , (uint8_t*)_meta -> running_msg -> data + _meta ->send_ptr
                    , (void*)&_meta -> regs -> DR, new_size);
        _meta -> send_ptr += new_size;
    }
    if(_loaded_size)
        *_loaded_size = new_size;

    return status;
}

uint32_t i2c_try_stop(TDrvI2CMeta *_meta)
{
    uint8_t old, chg;

    TDRV_DMA_API(_meta -> dma1_device).Stop(_meta -> dma1_device, _meta -> dma_stream);
    TDRV_DMA_API(_meta -> dma1_device).StreamFree(_meta -> dma1_device, _meta -> dma_stream);
    _meta -> dma_stream = -1;
    I2C_GenerateSTOP(_meta -> regs, ENABLE);
    I2C_DMACmd(_meta -> regs, DISABLE);
    I2C_Cmd(_meta -> regs, DISABLE);

    for(old = _meta -> flags; !(old & I2C_NEW_MESSAGE) ; old = chg)
    {
        chg = TDrvCAS8((uint8_t*)&_meta -> flags, old, old & (~(I2C_RUNNING|I2C_DMA_SPLITED)));
        if(chg == old)
            return 1;
    }

    return 0;
}

void i2c_notify_message_user(TDrvI2CMessage *_msg)
{
    _msg -> attribute = (_msg -> attribute & ~(TDRV_I2C_MSG_RECEIVE|TDRV_I2C_MSG_BUSY))
                        | ((_msg -> address & 0x0001) << TDRV_I2C_MSG_RECEIVE_POS);
    if(_msg -> callback)
        _msg -> callback(_msg);
}

TDRVStatus i2c_try_start_transfer(TDrvI2CMeta *_meta)
{
    TDRVStatus status;
    bi_list_node *node;
    TDrvI2CMessage *msg;
    size_t loaded;

    if(!_meta -> dma1_device)
        return TDRV_ILLEGAL_STATE;

    //lock-free i2c launch precedure.
    for(;;)
    {
        node = TDrvCriticalQueuePop(&_meta -> queue, I2C_NEW_MESSAGE);
        msg = _meta -> running_msg;
        if(msg)
        {
            if(!node) // it seems no message.
            {
                _meta -> running_msg = 0;
                //try to stop I2C Peripheral
                if( i2c_try_stop(_meta))
                {
                    i2c_notify_message_user(msg);
                    return TDRV_OK;
                }
                
                // new messages have come just now
                continue;
            }

            //has new message.
            msg = _meta -> running_msg;
            _meta -> running_msg = TDRV_TO_I2C_MSG(node);


            //If no restart condition is needed, generate stop condition.
            if( !(msg -> attribute & TDRV_I2C_MSG_RESTART
                && msg -> attribute & TDRV_I2C_MSG_PACKED) )
                I2C_GenerateSTOP(_meta -> regs, ENABLE);
            
            //Check whether DMA Stream reallocation is needed.
            if((msg -> address ^ (_meta -> running_msg -> attribute >> TDRV_I2C_MSG_RECEIVE_POS)) & 0x0001)
                i2c_dma_realloc_stream(_meta, _meta -> running_msg -> attribute & TDRV_I2C_MSG_RECEIVE);

            i2c_notify_message_user(msg);
        }
        else
        {
            if(!node)
                return TDRV_OK;

            _meta -> running_msg = TDRV_TO_I2C_MSG(node);
            i2c_dma_realloc_stream(_meta, _meta -> running_msg -> attribute & TDRV_I2C_MSG_RECEIVE);
            //I2C_AnalogFilterCmd(_meta -> regs, ENABLE);
            //I2C_DigitalFilterConfig(_meta -> regs, 0x0F);
            I2C_Cmd(_meta -> regs, ENABLE);
            I2C_AcknowledgeConfig(_meta -> regs, ENABLE);
        }

        I2C_ITConfig(_meta -> regs, I2C_IT_EVT, ENABLE);
        // Start next transfer
        _meta -> send_ptr = 0;
        // Initialize message address
        _meta -> running_msg -> address = 
            (_meta -> running_msg -> address & 0xFFFE) | ((_meta -> running_msg -> attribute & TDRV_I2C_MSG_RECEIVE) >> TDRV_I2C_MSG_RECEIVE_POS);

        status = i2c_dma_load_message(_meta, &loaded);
        if(!loaded)
        {
            I2C_DMACmd(_meta -> regs, DISABLE);
            I2C_ITConfig(_meta -> regs, I2C_IT_BUF, ENABLE);
        }
        else
        {
            I2C_DMACmd(_meta -> regs, ENABLE);
            I2C_ITConfig(_meta -> regs, I2C_IT_BUF, DISABLE);
        }

        if(TDRV_OK != status)
        {
            //failed to start dma
            _meta -> running_msg -> attribute |= TDRV_I2C_FAILED_DMA;
            _meta -> running_msg -> attribute &= ~TDRV_I2C_MSG_BUSY;
            continue;
        }

        I2C_GenerateSTART(_meta -> regs, ENABLE);
        break;
    }

    return TDRV_OK;
}


void* i2c_dma_callback(Byte _dma_event, void* _event_param, void* _user_param)
{
    TDrvI2CMeta *meta;
    size_t new_size;

    meta = (TDrvI2CMeta*) _user_param;
    switch(_dma_event)
    {
    case TDRV_DMA_EVT_COMPLETED:
        if(TDRV_OK == i2c_dma_load_message(meta, &new_size))
            if(!new_size)
            {
                I2C_DMACmd(meta -> regs, DISABLE);
                TDRV_DMA_API(meta -> dma1_device).Stop(meta -> dma1_device, meta -> dma_stream);
            }
        break;
    case TDRV_DMA_HARDWARE_ERROR:
        I2C_Cmd(meta -> regs, DISABLE);
        I2C_DMACmd(meta -> regs, DISABLE);
        meta -> running_msg -> attribute |= TDRV_I2C_FAILED_DMA;
        meta -> running_msg -> failed_info = _event_param;
        meta -> running_msg -> callback(meta -> running_msg);
        break;
    }
    
    return 0;
}


void i2c_nvic_config(uint32_t _irqn_ev, uint32_t _irqn_er, FunctionalState state)
{
    NVIC_InitTypeDef ninit;

    ninit.NVIC_IRQChannelPreemptionPriority = 0;
    ninit.NVIC_IRQChannelSubPriority = 0;
    ninit.NVIC_IRQChannelCmd = state;
    ninit.NVIC_IRQChannel = _irqn_ev;
    NVIC_Init(&ninit);

    ninit.NVIC_IRQChannel = _irqn_er;
    NVIC_Init(&ninit);
}

TDRVStatus i2c_init(TDevice *_device)
{
    TDrvI2CMeta *meta;
    meta = EXTRACT_I2C_META(_device);


    switch((unsigned int)meta -> regs)
    {
    case I2C1_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
        i2c_nvic_config(I2C1_EV_IRQn, I2C1_ER_IRQn, ENABLE);
        if(i2c_devices_map[0])
            return TDRV_ALREADY_EXIST;
        i2c_devices_map[0] = _device;
        break;
    case I2C2_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE); 
        i2c_nvic_config(I2C2_EV_IRQn, I2C2_ER_IRQn, ENABLE); 
        if(i2c_devices_map[1])
            return TDRV_ALREADY_EXIST;
        i2c_devices_map[1] = _device;
        break;
    case I2C3_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, ENABLE);
        i2c_nvic_config(I2C3_EV_IRQn, I2C3_ER_IRQn, ENABLE);
        if(i2c_devices_map[2])
            return TDRV_ALREADY_EXIST;
        i2c_devices_map[2] = _device;
        break;
    default:
        return TDRV_INVAILD_PARAMETER;
    }

    I2C_DeInit(meta -> regs);
    
    meta -> queue.flag_type = CQFT_U8;
    meta -> queue.queue = 0;
    meta -> queue.add = 0;
    meta -> queue.flag_ptr = &meta -> flags;
    meta -> running_msg = 0;

    meta -> dma_stream = -1;
    meta -> dma1_device = 0;

    return TDRV_OK;
}

TDRVStatus i2c_load_dma_info(TDevice *_device, TDevice *_dma1_device)
{
    TDrvI2CMeta *meta;
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device || !_dma1_device)
            return TDRV_INVAILD_PARAMETER;
    #endif

    meta = EXTRACT_I2C_META(_device);
    meta -> dma1_device = _dma1_device;

    return TDRV_OK;
}

TDRVStatus i2c_deinit(TDevice *_device)
{
    TDrvI2CMeta *meta;
    meta = EXTRACT_I2C_META(_device);

    switch((unsigned int)meta -> regs)
    {
    case I2C1_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE); 
        i2c_nvic_config(I2C1_EV_IRQn, I2C1_ER_IRQn, DISABLE);
        if(i2c_devices_map[0] != _device)
            return TDRV_ILLEGAL_STATE;
        i2c_devices_map[0] = 0;
        break;
    case I2C2_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, DISABLE);
        i2c_nvic_config(I2C2_EV_IRQn, I2C2_ER_IRQn, DISABLE);
        if(i2c_devices_map[1] != _device)
            return TDRV_ILLEGAL_STATE;
        i2c_devices_map[1] = 0;
        break;
    case I2C3_BASE:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, DISABLE);
        i2c_nvic_config(I2C3_EV_IRQn, I2C3_ER_IRQn, DISABLE);
        if(i2c_devices_map[2] != _device)
            return TDRV_ILLEGAL_STATE;
        i2c_devices_map[2] = 0;
        break;
    default:
        return TDRV_INVAILD_PARAMETER;
    }

    I2C_ITConfig(meta -> regs, I2C_IT_EVT|I2C_IT_ERR, DISABLE);

    return TDRV_OK;
}

TDRVStatus i2c_reconfigure(TDevice *_device, I2CAddressMode _addr_mode, I2CSpeed _speed)
{
    TDrvI2CMeta *meta;
    uint8_t old, chg, target;
    meta = EXTRACT_I2C_META(_device);
    
    target = 0;
    if(_addr_mode == I2C_10BIT_ADDR)
        return TDRV_NOT_SUPPORTED;
    if(_speed == I2C_400KBPS)
        target |= I2C_400KBPS;

    for(;;)
    {
        old = meta -> flags;
        if(!((old ^ target) & (~I2C_HARDWARE_CONFIGURED)))
            break;
        chg = TDrvCAS8(&meta -> flags, old, target);
        if(chg == old)
            break;
    }

    return TDRV_OK;
}

TDRVStatus i2c_update_hardware(TDrvI2CMeta *meta)
{
    uint8_t old, chg;
    I2C_InitTypeDef init;
    
    old = meta -> flags;
    if( !(old & I2C_HARDWARE_CONFIGURED) )
    {
        do
        {
            chg = TDrvCAS8(&meta -> flags, old, old | I2C_HARDWARE_CONFIGURED);
            if(chg == old)
                break;
            old = chg;
        }while(!(old & I2C_HARDWARE_CONFIGURED));

        init.I2C_Ack = I2C_Ack_Enable;
        if(old & I2C_10BIT_ADDRESS)
            //init.I2C_acknowledged_address = I2C_AcknowledgedAddress_10bit;
            return TDRV_NOT_SUPPORTED;
        else
            init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
        if(old & I2C_400KBPS)
            init.I2C_ClockSpeed = 400000;
        else
            init.I2C_ClockSpeed = 100000;
        init.I2C_DutyCycle = I2C_DutyCycle_16_9;
        init.I2C_Mode = I2C_Mode_I2C;
        init.I2C_OwnAddress1 = 0;

        I2C_Init(meta -> regs, &init);
        I2C_ITConfig(meta -> regs, I2C_IT_EVT|I2C_IT_ERR, ENABLE);
    }
    return TDRV_OK;
}

TDRVStatus i2c_queue_message(TDevice *_device, TDrvI2CMessage *_message)
{
    TDrvI2CMeta *meta;
    uint8_t old_flags;
    
    meta = EXTRACT_I2C_META(_device);
    i2c_update_hardware(meta);

    _message -> attribute |= TDRV_I2C_MSG_BUSY;
    old_flags = TDrvCriticalQueuePush(&meta -> queue, &_message -> node, I2C_RUNNING | I2C_NEW_MESSAGE);
    if(!(old_flags & I2C_RUNNING))
        return i2c_try_start_transfer(meta);

    return TDRV_OK;
}

void i2c_error_handle(TDrvI2CMeta *_meta)
{
    TDrvI2CMessage *msg;
    bi_list_node *node;
    uint32_t error_ctx;

    // notify user

    error_ctx = _meta -> regs -> SR1 | _meta -> regs -> SR2 << 16;
    for(;;)
    { 
        node = TDrvCriticalQueuePeek(&_meta -> queue, I2C_NEW_MESSAGE);
        if(node)
        {
            msg = TDRV_TO_I2C_MSG(node);
            if(_meta -> running_msg -> attribute & TDRV_I2C_MSG_PACKED)
            {
                _meta -> running_msg -> attribute |= TDRV_I2C_FAILED;
                _meta -> running_msg -> failed_info = (void*)error_ctx;
                i2c_notify_message_user(_meta -> running_msg);
                _meta -> running_msg = msg;
                TDrvCriticalQueuePop(&_meta -> queue, I2C_NEW_MESSAGE);
                continue;
            }
        }
        _meta -> running_msg -> attribute |= TDRV_I2C_FAILED;
        _meta -> running_msg -> failed_info = (void*)error_ctx;
        break;
    }
    //recover i2c from errors
    _meta -> regs -> SR1 = 0;
    _meta -> regs -> SR2 = 0;

    //start next transmission.
    i2c_try_start_transfer(_meta);
}

void i2c_status_notify(TDrvI2CMeta *_meta)
{
    uint32_t event;

    event = I2C_GetLastEvent(_meta -> regs);
    switch(event)
    {
    case I2C_EVENT_MASTER_BYTE_TRANSMITTING:
    case I2C_EVENT_MASTER_BYTE_TRANSMITTED:
        if(_meta -> running_msg -> address & 0x0001)
            break;
        if(_meta -> send_ptr == _meta -> running_msg -> size - 1)// Send last byte.
        {
            I2C_ITConfig(_meta -> regs, I2C_IT_BUF, DISABLE);
            _meta -> regs -> DR = ((uint8_t*)_meta -> running_msg -> data)[_meta -> send_ptr];
            _meta -> send_ptr++;
        }
        else if(_meta -> send_ptr == _meta -> running_msg -> size)
        {
            I2C_ITConfig(_meta -> regs, I2C_IT_BUF|I2C_IT_EVT, DISABLE);
            i2c_try_start_transfer(_meta);
        }
            
        break;
    #define BTF_FLAG        (1u << 2)
    case I2C_EVENT_MASTER_BYTE_RECEIVED | BTF_FLAG:
    case I2C_EVENT_MASTER_BYTE_RECEIVED:
    #undef BTF_FLAG
        //if(!(_meta -> running_msg -> address & 0x0001))
        //    break;
        ((uint8_t*)_meta -> running_msg -> data)[_meta -> send_ptr] = _meta -> regs -> DR;
        I2C_AcknowledgeConfig(_meta -> regs, DISABLE);
        if(_meta -> running_msg -> size - _meta -> send_ptr == 2)
            I2C_DMACmd(_meta -> regs, DISABLE);
        else if(_meta -> running_msg -> size - _meta -> send_ptr == 1)
            i2c_try_start_transfer(_meta);
        _meta -> send_ptr++;
        break;

    case I2C_EVENT_MASTER_MODE_SELECT:
        /*
            A new transaction begins.
            Put slave address into bus.
        */
        _meta -> regs -> DR = _meta -> running_msg -> address;
        break; 
    }
}

void I2C1_EV_IRQHandler(void)
{ i2c_status_notify(EXTRACT_I2C_META(i2c_devices_map[0]));}


void I2C2_EV_IRQHandler(void)
{ i2c_status_notify(EXTRACT_I2C_META(i2c_devices_map[1]));}

void I2C3_EV_IRQHandler(void)
{ i2c_status_notify(EXTRACT_I2C_META(i2c_devices_map[2]));}

void I2C1_ER_IRQHandler(void)
{i2c_error_handle(EXTRACT_I2C_META(i2c_devices_map[0])); }

void I2C2_ER_IRQHandler(void)
{ i2c_error_handle(EXTRACT_I2C_META(i2c_devices_map[1])); }

void I2C3_ER_IRQHandler(void)
{ i2c_error_handle(EXTRACT_I2C_META(i2c_devices_map[2])); }

// interfaces export
STM32F4xx_I2CInterfaceType I2CInterface = {
    i2c_init
    , i2c_deinit
    , i2c_queue_message
    , i2c_reconfigure
    , i2c_load_dma_info
};

