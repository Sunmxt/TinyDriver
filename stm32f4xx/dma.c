#include <limits.h>

#include "dma.h"
#include "stm32f4xx.h"
#include "std/inc/stm32f4xx_rcc.h"
#include "std/inc/stm32f4xx_dma.h"
#include "std/inc/misc.h"

extern TDevice *tdrv_stm32f4_dma_irq_assoc[2];

void tdrv_dma_event_dispatch(Byte _dma_index, Byte _stream, uint32_t _TCIF_mask, uint32_t _TEIF_mask)
{
    TDrvDMAInfo *info;
    info = (TDrvDMAInfo*) tdrv_stm32f4_dma_irq_assoc[_dma_index] -> private_data;

    if(!info -> callback[_stream])
        return;

    if( SET == DMA_GetITStatus(info -> streams[_stream], _TCIF_mask) )
    {
        if( info -> init_flags[_stream] & TDRV_DMA_DOUBLE_BUFFER )
        {
            if( 0 == DMA_GetCurrentMemoryTarget(info -> streams[_stream]) )
                info -> callback[_stream](TDRV_DMA_EVT_BUFFER_SWAPED, (void*)info -> streams[_stream] -> M0AR, info -> callback_user_param[_stream]);
            else
                info -> callback[_stream](TDRV_DMA_EVT_BUFFER_SWAPED, (void*)info -> streams[_stream] -> M1AR, info -> callback_user_param[_stream]);
        }
        else
        {
            DMA_Cmd(info -> streams[_stream], DISABLE);
            info -> init_flags[_stream] &= ~TDRV_DMA_LOCK_CONFIG;
            info -> callback[_stream](TDRV_DMA_EVT_COMPLETED, (void*)info -> streams[_stream] -> M0AR, info -> callback_user_param[_stream]);
            
        }
    }

    if( SET == DMA_GetITStatus(info -> streams[_stream], _TEIF_mask) )
    {
        DMA_Cmd(info -> streams[_stream], DISABLE);
        info -> callback[_stream](TDRV_DMA_HARDWARE_ERROR, 0, info -> callback_user_param[_stream]);
        info -> init_flags[_stream] &= ~TDRV_DMA_LOCK_CONFIG;
    }
    
}


void DMA1_Stream0_IRQHandler() { tdrv_dma_event_dispatch(0, 0, DMA_IT_TCIF0, DMA_IT_TEIF0); }
void DMA1_Stream1_IRQHandler() { tdrv_dma_event_dispatch(0, 1, DMA_IT_TCIF1, DMA_IT_TEIF1); }
void DMA1_Stream2_IRQHandler() { tdrv_dma_event_dispatch(0, 2, DMA_IT_TCIF2, DMA_IT_TEIF2); }
void DMA1_Stream3_IRQHandler() { tdrv_dma_event_dispatch(0, 3, DMA_IT_TCIF3, DMA_IT_TEIF3); }
void DMA1_Stream4_IRQHandler() { tdrv_dma_event_dispatch(0, 4, DMA_IT_TCIF4, DMA_IT_TEIF4); }
void DMA1_Stream5_IRQHandler() { tdrv_dma_event_dispatch(0, 5, DMA_IT_TCIF5, DMA_IT_TEIF5); }
void DMA1_Stream6_IRQHandler() { tdrv_dma_event_dispatch(0, 6, DMA_IT_TCIF6, DMA_IT_TEIF6); }
void DMA1_Stream7_IRQHandler() { tdrv_dma_event_dispatch(0, 7, DMA_IT_TCIF7, DMA_IT_TEIF7); }
void DMA2_Stream0_IRQHandler() { tdrv_dma_event_dispatch(1, 0, DMA_IT_TCIF0, DMA_IT_TEIF0); }
void DMA2_Stream1_IRQHandler() { tdrv_dma_event_dispatch(1, 1, DMA_IT_TCIF1, DMA_IT_TEIF1); }
void DMA2_Stream2_IRQHandler() { tdrv_dma_event_dispatch(1, 2, DMA_IT_TCIF2, DMA_IT_TEIF2); }
void DMA2_Stream3_IRQHandler() { tdrv_dma_event_dispatch(1, 3, DMA_IT_TCIF3, DMA_IT_TEIF3); }
void DMA2_Stream4_IRQHandler() { tdrv_dma_event_dispatch(1, 4, DMA_IT_TCIF4, DMA_IT_TEIF4); }
void DMA2_Stream5_IRQHandler() { tdrv_dma_event_dispatch(1, 5, DMA_IT_TCIF5, DMA_IT_TEIF5); }
void DMA2_Stream6_IRQHandler() { tdrv_dma_event_dispatch(1, 6, DMA_IT_TCIF6, DMA_IT_TEIF6); }
void DMA2_Stream7_IRQHandler() { tdrv_dma_event_dispatch(1, 7, DMA_IT_TCIF7, DMA_IT_TEIF7); }


void dma_nvic_config(uint32_t _irqn, FunctionalState _state)
{
    NVIC_InitTypeDef ninit;
    ninit.NVIC_IRQChannel = _irqn;
    ninit.NVIC_IRQChannelCmd = _state;
    ninit.NVIC_IRQChannelPreemptionPriority = 0;
    ninit.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&ninit);
}

void dma_nvic_command(TDrvDMAInfo *_info, uint8_t _stream, FunctionalState _command)
{
    if(DMA1 == _info -> regs)
    {
        switch(_stream)
        {
        case 0:
            dma_nvic_config(DMA1_Stream0_IRQn, _command); break;
        case 1:
            dma_nvic_config(DMA1_Stream1_IRQn, _command); break;
        case 2:
            dma_nvic_config(DMA1_Stream2_IRQn, _command); break;
        case 3:
            dma_nvic_config(DMA1_Stream3_IRQn, _command); break;
        case 4:
            dma_nvic_config(DMA1_Stream4_IRQn, _command); break;
        case 5:
            dma_nvic_config(DMA1_Stream5_IRQn, _command); break;
        case 6:
            dma_nvic_config(DMA1_Stream6_IRQn, _command); break;
        case 7:
            dma_nvic_config(DMA1_Stream7_IRQn, _command); break;
        }
    }
    else if(DMA2 == _info -> regs)
    {
        switch(_stream)
        {
        case 0:
            dma_nvic_config(DMA2_Stream0_IRQn, _command); break;
        case 1:
            dma_nvic_config(DMA2_Stream1_IRQn, _command); break;
        case 2:
            dma_nvic_config(DMA2_Stream2_IRQn, _command); break;
        case 3:
            dma_nvic_config(DMA2_Stream3_IRQn, _command); break;
        case 4:
            dma_nvic_config(DMA2_Stream4_IRQn, _command); break;
        case 5:
            dma_nvic_config(DMA2_Stream5_IRQn, _command); break;
        case 6:
            dma_nvic_config(DMA2_Stream6_IRQn, _command); break;
        case 7:
            dma_nvic_config(DMA2_Stream7_IRQn, _command); break;
        }
    }   
}

TDRVStatus tdrv_dma_init(TDevice *_ha_instance)
{
    Uint i;
    TDrvDMAInfo *info;
    info = (TDrvDMAInfo*) _ha_instance -> private_data;

    if(DMA1 == info -> regs)
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    else if(DMA2 == info -> regs)
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    for(i = 0 ; i < 8 ; i++)
    {
        info -> init_flags[i] = 0;
        info -> callback[i] = 0;
        info -> callback_user_param[i] = 0;
    }
		
    return TDRV_OK;
}

TDRVStatus tdrv_dma_deinit(TDevice *_ha_instance)
{
    Uint i;
    TDrvDMAInfo *info;
    info = (TDrvDMAInfo*) _ha_instance -> private_data;

    if(DMA1 == info -> regs)
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, DISABLE);
    else if(DMA2 == info -> regs)
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, DISABLE);

    for(i = 0 ; i < 8 ; i++)
        DMA_DeInit(info -> streams[i]);
		
		return TDRV_OK;
}

TDRVStatus tdrv_dma_get_capability(TDevice *_ha_instance, TDrvDMACapability *_capacity)
{
    _capacity -> stream_count = 8;
    _capacity -> channels_count = 8;
    _capacity -> stream_capability = TDRV_DMA_CAP_MEM2MEM|TDRV_DMA_CAP_MEM2DEV|TDRV_DMA_CAP_DEV2MEM|TDRV_DMA_CAP_8BIT|TDRV_DMA_CAP_16BIT|TDRV_DMA_CAP_32BIT|TDRV_DMA_CAP_DOUBLE_BUFFER;
    _capacity -> max_buffer_size = 0xFFFF;

    return TDRV_OK;
}

TDRVStatus tdrv_dma_stream_connect(TDevice *_ha_instance, Byte _stream, Byte _channel)
{
    Word init_flags;
    TDrvDMAInfo *info;
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(_stream >= 8 || _channel >= 8)
            return TDRV_INVAILD_PARAMETER;
    #endif
    info = (TDrvDMAInfo*) _ha_instance -> private_data;

    init_flags = info -> init_flags[_stream];
    if( init_flags & TDRV_DMA_LOCK_CONFIG )
        return TDRV_BUSY;

    init_flags &= ~TDRV_DMA_CHANNEL_MASK;
    init_flags |= (_channel << TDRV_DMA_CHANNEL_BIT) & TDRV_DMA_CHANNEL_MASK;

    #warning "should use CAS-Based Swap or be Lock-Protected in Multi-Thread enviroment."
    info -> init_flags[_stream] = init_flags;

    return TDRV_OK;
}

TDRVStatus tdrv_dma_set_mode(TDevice *_ha_instance, Byte _stream, Word _mode)
{
    Word init_flags;
    TDrvDMAInfo *info;
    info = (TDrvDMAInfo*) _ha_instance -> private_data;
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(_stream >= 8)
            return TDRV_INVAILD_PARAMETER;
    #endif

    init_flags = info -> init_flags[_stream];
    if( init_flags & TDRV_DMA_LOCK_CONFIG )
        return TDRV_BUSY;

    init_flags &= ~TDRV_DMA_COMMON_FLAGS_MASK;
    init_flags |= _mode & TDRV_DMA_COMMON_FLAGS_MASK;

    #warning "should use CAS-Based Swap or be Lock-Protected in Multi-Thread enviroment."
    info -> init_flags[_stream] = init_flags;
    
    return TDRV_OK;
}


TDRVStatus tdrv_dma_stop(TDevice *_ha_instance, Byte _stream)
{
    TDrvDMAInfo *info;
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(_stream > 7)
            return TDRV_INVAILD_PARAMETER;
    #endif
    info = (TDrvDMAInfo*) _ha_instance -> private_data;

    DMA_Cmd(info -> streams[_stream], DISABLE);
    DMA_ITConfig(info -> streams[_stream], DMA_IT_TC|DMA_IT_TE, DISABLE);

    return TDRV_OK;

}

TDRVStatus tdrv_dma_stream_alloc(TDevice *_device, uint8_t _stream)
{
    TDrvDMAInfo *info;
    uint8_t old, chg;

    #ifndef TDRV_NO_PARAMETER_CHECK
        if(_stream >= 8)
            return TDRV_INVAILD_PARAMETER;
    #endif

    info = (TDrvDMAInfo*)_device -> private_data;
    for(old = info -> stream_lock; ; )
    {
        if(old & (1u << _stream))
            return TDRV_BUSY;
        chg = TDrvCAS8(&info -> stream_lock, old, old|(1u << _stream));
        if(chg == old)
            break;
    }

    dma_nvic_command(info, _stream, ENABLE);

    return TDRV_OK;
}

TDRVStatus tdrv_dma_stream_free(TDevice *_device, uint8_t _stream)
{
    TDrvDMAInfo *info;
    uint8_t old, chg;

    #ifndef TDRV_NO_PARAMETER_CHECK
        if(_stream >= 8)
            return TDRV_INVAILD_PARAMETER;
    #endif

    info = (TDrvDMAInfo*)_device -> private_data;
    for(old = info -> stream_lock; old & (1u << _stream); )
    {
        chg = TDrvCAS8(&info -> stream_lock, old, old & (~(1u << _stream)));
        if(chg == old)
        {
            dma_nvic_command(info, _stream, DISABLE);
            break;
        }
    }


    return TDRV_OK;
}

TDRVStatus tdrv_dma_start_transfer(TDevice *_ha_instance, Byte _stream, void* _buffer, void* _device_target, size_t _buffer_size)
{
    TDRVStatus status;
    DMA_InitTypeDef init;
    TDrvDMAInfo *info;
    Word init_flags;

    const uint32_t mem_datasize_map[] = {DMA_MemoryDataSize_Byte, DMA_MemoryDataSize_HalfWord, DMA_MemoryDataSize_Word};
    const uint32_t dev_datasize_map[] = {DMA_PeripheralDataSize_Byte, DMA_PeripheralDataSize_HalfWord, DMA_PeripheralDataSize_Word};

    #ifndef TDRV_NO_PARAMETER_CHECK
        if(_stream >= 8)
            return TDRV_INVAILD_PARAMETER;
    #endif

    status = TDRV_OK;

    info = (TDrvDMAInfo*) _ha_instance -> private_data;
    if( DISABLE != DMA_GetCmdStatus(info -> streams[_stream]) )
        return TDRV_BUSY;
    if( _buffer_size > 0xFFFF )
        return TDRV_NOT_SUPPORTED;

    init_flags = info -> init_flags[_stream];

    init.DMA_PeripheralBaseAddr = (uint32_t)_device_target;
    init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    init.DMA_Memory0BaseAddr = (uint32_t)_buffer;
    init.DMA_MemoryBurst = DMA_MemoryBurst_Single;

    init.DMA_Channel = (init_flags & TDRV_DMA_CHANNEL_MASK) >> TDRV_DMA_CHANNEL_BIT << 25;
    
    init.DMA_MemoryDataSize = (init_flags & TDRV_DMA_MEM_DATA_WIDTH_MASK) >> TDRV_DMA_MEM_DATA_WIDTH_BIT;
    if(init.DMA_MemoryDataSize > 2)
        return TDRV_NOT_SUPPORTED;
    init.DMA_MemoryDataSize = mem_datasize_map[init.DMA_MemoryDataSize];

    init.DMA_PeripheralDataSize = (init_flags & TDRV_DMA_DEV_DATA_WIDTH_MASK) >> TDRV_DMA_DEV_DATA_WIDTH_BIT;
    if(init.DMA_PeripheralDataSize > 2)
        return TDRV_NOT_SUPPORTED;
    init.DMA_BufferSize = _buffer_size & (~((1 << init.DMA_PeripheralDataSize) - 1));
    if(init.DMA_BufferSize != _buffer_size)
        status = TDRV_WARN_DATA_CUT;
    init.DMA_PeripheralDataSize = dev_datasize_map[init.DMA_PeripheralDataSize];
    
    if(init_flags & TDRV_DMA_MEM_INC)
        init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    else
        init.DMA_MemoryInc = DMA_MemoryInc_Disable;
    
    if(init_flags & TDRV_DMA_DEV_INC)
        init.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
    else
        init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

    init.DMA_FIFOMode = DMA_FIFOMode_Enable;
    init.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    init.DMA_Priority = DMA_Priority_VeryHigh;

    if( TDRV_DMA_MEM2MEM == (init_flags & TDRV_DMA_DIRECTION_MASK) )
    {
        #ifndef TDRV_NO_STRICT_PARAMETER_CHECK
            if( init_flags & TDRV_DMA_REPEAT )
                return TDRV_INVAILD_PARAMETER;
        #else
            init_flags &= ~TDRV_DMA_REPEAT;
        #endif

        
        #warning "should use CAS-Based Swap or be Lock-Protected in Multi-Thread enviroment."
        info -> init_flags[_stream] = init_flags | TDRV_DMA_LOCK_CONFIG;
        init.DMA_DIR = DMA_DIR_MemoryToMemory;
    }
    else
    {
        #ifndef TDRV_NO_STRICT_PARAMETER_CHECK
            if( init_flags & (TDRV_DMA_REPEAT | TDRV_DMA_DEV_TERMINATE) )
                return TDRV_INVAILD_PARAMETER;
        #else
            if( init_flags & TDRV_DMA_DEV_TERMINATE )
                init_flags &= ~TDRV_DMA_REPEAT;
        #endif

        if( TDRV_DMA_MEM2DEV == (init_flags & TDRV_DMA_DIRECTION_MASK))
            init.DMA_DIR = DMA_DIR_MemoryToPeripheral;
        else
            init.DMA_DIR = DMA_DIR_PeripheralToMemory;

        if(init_flags & TDRV_DMA_REPEAT)
            init.DMA_Mode = DMA_Mode_Circular;
        else
            init.DMA_Mode = DMA_Mode_Normal;

        if(init_flags & TDRV_DMA_DOUBLE_BUFFER)
            DMA_DoubleBufferModeCmd(info -> streams[_stream], ENABLE);
        else
            DMA_DoubleBufferModeCmd(info -> streams[_stream], DISABLE);

        if(init_flags & TDRV_DMA_DEV_TERMINATE)
            DMA_FlowControllerConfig(info -> streams[_stream], DMA_FlowCtrl_Peripheral);
        else
            DMA_FlowControllerConfig(info -> streams[_stream], DMA_FlowCtrl_Memory);
        
        #warning "should use CAS-Based Swap or be Lock-Protected in Multi-Thread enviroment."
        info -> init_flags[_stream] = init_flags | TDRV_DMA_LOCK_CONFIG;
    }

    DMA_DeInit(info -> streams[_stream]);
    DMA_Init(info -> streams[_stream] , &init);
    DMA_ITConfig(info -> streams[_stream], DMA_IT_TC | DMA_IT_TE, ENABLE);
    
    DMA_Cmd(info -> streams[_stream], ENABLE);

    return status;
}

TDRVStatus tdrv_dma_set_swap_buffer(TDevice *_ha_instance, Byte _stream, void* _buffer)
{
    TDrvDMAInfo *info;

    #ifndef TDRV_NO_PARAMETER_CHECK
        if(_stream > 7)
            return TDRV_INVAILD_PARAMETER;
    #endif
    info = (TDrvDMAInfo*) _ha_instance -> private_data;

    
    if( 0 == DMA_GetCurrentMemoryTarget(info -> streams[_stream]) )
        DMA_MemoryTargetConfig(info -> streams[_stream], (uint32_t)_buffer, DMA_Memory_1);
    else
        DMA_MemoryTargetConfig(info -> streams[_stream], (uint32_t)_buffer, DMA_Memory_0);

    return TDRV_OK;
}

size_t tdrv_dma_get_buffer_remain_size(TDevice *_ha_instance , Byte _stream)
{
    TDrvDMAInfo *info;
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(_stream > 7)
            return 0;
    #endif
    info = (TDrvDMAInfo*) _ha_instance -> private_data;

    return DMA_GetCurrDataCounter(info -> streams[_stream]);
}


TDRVStatus tdrv_dma_set_callback(TDevice *_ha_instance, Byte _stream, TDrvDMACallback _callback, void* _user_param)
{
    TDrvDMAInfo *info;
    #ifndef TDRV_NO_PARAMETER_CHECK
        if( _stream > 7 )
            return TDRV_INVAILD_PARAMETER;
    #endif
    info = (TDrvDMAInfo*) _ha_instance -> private_data;
    
    info -> callback[_stream] = _callback;
    info -> callback_user_param[_stream] = _user_param;

    return TDRV_OK;
}

TDrvDMAInterface DMAInterface = {
    tdrv_dma_init
    , tdrv_dma_deinit
    , tdrv_dma_stream_alloc
    , tdrv_dma_stream_free
    , tdrv_dma_get_capability
    , tdrv_dma_stream_connect
    , tdrv_dma_set_mode
    , tdrv_dma_stop
    , tdrv_dma_start_transfer
    , tdrv_dma_set_swap_buffer
    , tdrv_dma_get_buffer_remain_size
    , tdrv_dma_set_callback
};
