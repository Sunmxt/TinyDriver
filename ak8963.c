/*
    AK8963 Driver
*/

#include <string.h>
#include "ak8963.h"

/* ------------------------------------------------ */
/*  Meta information                                */
/* ------------------------------------------------ */

const char *ak8963_name = "AK8963";
const char *ak8963_vendor = "AKM Semic";

const uint8_t ak8963_init_config_data[] = {
     AK8963_CONTROL_1_ADDR
    , AK8963_CONTINUOUS_MEASUREMENT_MODE_2 | AK8963_OUTPUT_16_BIT
};

const uint8_t ak8963_init_read_rom_config[] = {
    AK8963_CONTROL_1_ADDR
    , AK8963_FUSE_ROM_ACCESS_MODE | AK8963_OUTPUT_16_BIT
};

const uint8_t ak8963_power_down_config[] = {
	AK8963_CONTROL_1_ADDR
	, AK8963_POWERDOWN_MODE
};

const uint8_t ak8963_output_pack_addr[] = {AK8963_STATUS_1_ADDR};
const uint8_t ak8963_output_adjust_addr = AK8963_SENSITIVITY_ADJUST_ADDR;


/* ------------------------------------------------ */
/*  Core                                            */
/* ------------------------------------------------ */

TDRVStatus ak8963_operate(TDevice *_device);
TDRVStatus ak8963_set_operation_req(TDevice *_device, uint8_t _request);
void* ak8963_configure_stage2_callback(TDrvI2CMessage *_message);

TDRVStatus ak8963_i2c_error_check(AK8963Runtime* _runtime ,TDrvI2CMessage *_message)
{
    // Error report
    if( _message -> attribute & TDRV_I2C_FAILED )
    {
        if( _message -> attribute & TDRV_I2C_NO_ACK )
            _runtime -> drv_state = TAK8963_DEVICE_LOST;
        else
            _runtime -> drv_state = TAK8963_BUS_ERROR;
        return TDRV_ILLEGAL_STATE;
    }

    return TDRV_OK;
}

void* ak8963_i2c_transfering_error_handle(TDrvI2CMessage *_message)
{
	AK8963Runtime *runtime;

	runtime = TSENS_TO_AK8963_RUNTIME((TDevice*)_message -> callback_params);
    if(TDRV_OK != ak8963_i2c_error_check(runtime, _message))
    {
        runtime -> drv_state = TAK8963_BUS_ERROR;
        ak8963_operate(_message -> callback_params);
    }

    return 0;
}

void* ak8963_update_stage1_callback(TDrvI2CMessage *_message)
{
    AK8963Runtime *runtime;

    runtime = TSENS_TO_AK8963_RUNTIME((TDevice*)_message -> callback_params);

    if( TDRV_OK != ak8963_i2c_error_check(runtime, _message))
        return 0;

    runtime -> drv_state = TAK8963_UPDATE_NOTIFY;

    switch(lsnr_hub_notify(&runtime -> hub, TSENS_DATA_UPDATED, _message -> callback_params))
    {
    case TDRV_ILLEGAL_STATE:
    case TDRV_OK:
        runtime -> drv_state = TAK8963_FREE; break;
    default:
        ak8963_set_operation_req((TDevice*)_message -> callback_params, AK8963_UPDATE_REQ);
    }

    runtime -> drv_state = TAK8963_FREE;

    ak8963_operate((TDevice*)_message -> callback_params);

    return 0;
}

TDRVStatus ak8963_update_device(TDevice* _device)
{
    AK8963Runtime *runtime;

    runtime = TSENS_TO_AK8963_RUNTIME(_device);

    runtime -> drv_state = TAK8963_UPDATE_STAGE_1;

    TDrvI2CMessageInitAsWrite(runtime -> message + 0);
    TDrvI2CMessageInitAsRead(runtime -> message + 1);
    runtime -> message[0].address = runtime -> addr;
    runtime -> message[0].data = (void*)ak8963_output_pack_addr;
    runtime -> message[0].size = sizeof(ak8963_output_pack_addr);
    runtime -> message[0].callback = ak8963_i2c_transfering_error_handle;
    runtime -> message[0].callback_params = _device;
    runtime -> message[1].address = runtime -> addr;
    runtime -> message[1].data = &runtime -> measurement.out;
    runtime -> message[1].size = sizeof(runtime -> measurement.out);
    runtime -> message[1].callback = ak8963_update_stage1_callback;
    runtime -> message[1].callback_params = _device;
    TDrvI2CMessageLink(runtime -> message + 0, runtime -> message + 1, TDRV_I2C_MSG_RESTART);

    return TDRV_I2C_API(runtime -> bus).Put(runtime -> bus, runtime -> message + 0);
}


void ak8963_configure_read_adjust(TDevice *_device)
{
    AK8963Runtime *runtime;

    runtime = TSENS_TO_AK8963_RUNTIME(_device);

    TDrvI2CMessageInitAsWrite(runtime -> message + 0);
    runtime -> message[0].address = runtime -> addr;
    runtime -> message[0].data = (void*)&ak8963_output_adjust_addr;
    runtime -> message[0].size = 1;
    TDrvI2CMessageInitAsRead(runtime -> message + 1);
    runtime -> message[1].address = runtime -> addr;
    runtime -> message[1].data = &runtime -> measurement.adj;
    runtime -> message[1].size = sizeof(runtime -> measurement.adj);
    runtime -> message[1].callback = ak8963_configure_stage2_callback;
    runtime -> message[1].callback_params = _device;
    TDrvI2CMessageLink(runtime -> message + 0, runtime -> message + 1, TDRV_I2C_MSG_RESTART);
    TDRV_I2C_API(runtime -> bus).Put(runtime -> bus, runtime -> message + 0);

}

void* ak8963_i2c_finish_handler(TDrvI2CMessage *_message)
{
	AK8963Runtime *runtime;

	runtime = TSENS_TO_AK8963_RUNTIME((TDevice*)_message -> callback_params);
    if(TDRV_OK != ak8963_i2c_error_check(runtime, _message))
        runtime -> drv_state = TAK8963_BUS_ERROR;
    else
    	runtime -> drv_state = TAK8963_FREE;

    ak8963_operate((TDevice*)_message -> callback_params);

    return 0;
}

void* ak8963_configure_stage2_callback(TDrvI2CMessage *_message)
{
    TDevice *device;
    AK8963Runtime *runtime;

    runtime = TSENS_TO_AK8963_RUNTIME((TDevice*)_message -> callback_params);
    device = (TDevice*)_message -> callback_params;
    if(TDRV_OK != ak8963_i2c_error_check(runtime, _message))
    {
        if( TAK8963_CONFIG_ERR_RECOVER_3 == runtime -> drv_state )
        {
            runtime -> drv_state = TAK8963_CONFIG_ERR_RECOVER_1;
            TDrvExclusiveAnd8(&runtime -> flags, ~((uint8_t)AK8963_CONFIGURED));
        } else if(TAK8963_CONFIGURING == runtime -> drv_state )
        {
            runtime -> drv_state += 1;
            ak8963_configure_read_adjust((TDevice*)_message -> callback_params);
        }
        return 0;
    }

    TDrvI2CMessageInitAsWrite(runtime -> message + 0);
    TDrvI2CMessageInitAsWrite(runtime -> message + 1);
    runtime -> message[0].address = runtime -> addr;
    runtime -> message[0].data = (void*)ak8963_power_down_config;
    runtime -> message[0].size = sizeof(ak8963_power_down_config);
    runtime -> message[1].address = runtime -> addr;
    runtime -> message[1].data = (void*)&ak8963_init_config_data;
    runtime -> message[1].size = sizeof(ak8963_init_config_data);
    runtime -> message[1].callback = ak8963_i2c_finish_handler;
    runtime -> message[1].callback_params = device;
    TDrvI2CMessageLink(runtime -> message + 0, runtime -> message + 1, 0);
    TDRV_I2C_API(runtime -> bus).Put(runtime -> bus, runtime -> message + 0);

    return 0;
}

void* ak8963_configure_stage1_callback(TDrvI2CMessage *_message)
{
    TDevice *device;
    AK8963Runtime *runtime;

    runtime = TSENS_TO_AK8963_RUNTIME((TDevice*)_message -> callback_params);
    device = (TDevice*)_message -> callback_params;

    if(TDRV_OK != ak8963_i2c_error_check(runtime, _message))
    {
        TDrvExclusiveAnd8(&runtime -> flags, ~((uint8_t)AK8963_CONFIGURED));
        ak8963_operate((TDevice*)_message -> callback_params);
    }

    ak8963_configure_read_adjust((TDevice*)_message -> callback_params);

    return 0;
}

TDRVStatus ak8963_configure_device(TDevice* _device)
{
    AK8963Runtime* runtime;
    uint8_t cmd[2], id;

    runtime = TSENS_TO_AK8963_RUNTIME(_device);

    runtime -> drv_state = TAK8963_CONFIGURING;
    TDrvExclusiveOr8(&runtime -> flags, AK8963_CONFIGURED);

    TDrvI2CMessageInitAsWrite(runtime -> message + 0);
    TDrvI2CMessageInitAsWrite(runtime -> message + 1);
    runtime -> message[0].address = runtime -> addr;
    runtime -> message[0].data = (void*)ak8963_power_down_config;
    runtime -> message[0].size = sizeof(ak8963_power_down_config);
    runtime -> message[1].address = runtime -> addr;
    runtime -> message[1].data = (void*)ak8963_init_read_rom_config;
    runtime -> message[1].size = sizeof(ak8963_init_read_rom_config);
    runtime -> message[1].callback_params = _device;
    runtime -> message[1].callback = &ak8963_configure_stage1_callback;
    TDrvI2CMessageLink(runtime -> message + 0, runtime -> message + 1, 0);

    return TDRV_I2C_API(runtime -> bus).Put(runtime -> bus, runtime -> message + 0);
}

uint8_t ak8963_get_operation_request(AK8963Runtime *_runtime)
{
    uint8_t chg, old, new;

    chg = _runtime -> flags;
    do
    {
        old = chg;
        if(!(old & AK8963_REQUEST_MASK))
        {
            // If no request exists, try to stop operating.
            if(old & AK8963_OPERATING)
                new = old & (~AK8963_OPERATING);
            else
                return 0;
        }
        else
            // otherwise, get one.
            new = ((old & AK8963_REQUEST_MASK) - 1) & old;
        chg = TDrvCAS8(&_runtime -> flags, old, new);

    }while(chg != old);

    return (chg ^ new) & AK8963_REQUEST_MASK;
}


TDRVStatus ak8963_operate(TDevice* _device)
{
    uint8_t req;
    AK8963Runtime *runtime;

    runtime = TSENS_TO_AK8963_RUNTIME(_device);
    req = ak8963_get_operation_request(runtime);
    if(req)
    {
        switch(req)
        {
        case AK8963_CONFIGURE_REQ:
            return ak8963_configure_device(_device);
        case AK8963_UPDATE_REQ:
            return ak8963_update_device(_device);
        }
    }

    return TDRV_OK;
}

TDRVStatus ak8963_set_operation_req(TDevice* _device, uint8_t _request)
{
    uint8_t old, chg;
    AK8963Runtime *runtime;

    runtime = TSENS_TO_AK8963_RUNTIME(_device);

    _request &= AK8963_REQUEST_MASK;
    chg = runtime -> flags;
    do
    {
        old = chg;
        if(chg & _request)
            return TDRV_BUSY;
        chg = TDrvCAS8(&runtime -> flags, old, old | _request | AK8963_OPERATING);    
    }while(chg != old);

    if(!(chg & AK8963_OPERATING)) /* Not operating, start */
        return ak8963_operate(_device);

    return TDRV_OK;
}

TDRVStatus ak8963_init(TDevice* _device)
{
    AK8963Runtime *runtime;

    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device)
            return TDRV_INVAILD_PARAMETER;
    #endif

    runtime = TSENS_TO_AK8963_RUNTIME(_device);

    runtime -> bus = 0;
    runtime -> addr = 0;
    runtime -> state.name = ak8963_name;
    runtime -> state.vendor = ak8963_vendor;
    runtime -> state.type = TSENS_MAGNETMETER;
    runtime -> state.state = TSENS_CAN_LISTEN;
    runtime -> flags = 0;
    memset(&runtime -> measurement, 0, sizeof(runtime -> measurement));
    lsnr_hub_create(&runtime -> hub);

    runtime -> drv_state = TAK8963_FREE;

    return TDRV_OK;
}

TDRVStatus ak8963_deinit(TDevice *_device)
{
    AK8963Runtime *runtime;

    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device)
            return TDRV_INVAILD_PARAMETER;
    #endif

    runtime = TSENS_TO_AK8963_RUNTIME(_device);
    lsnr_hub_destroy(&runtime -> hub);

    return TDRV_OK;
}

TDRVStatus ak8963_i2c_bind(TDevice *_device, TDevice *_bus, uint8_t _address)
{
    AK8963Runtime *runtime;

    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device || !_bus)
            return TDRV_INVAILD_PARAMETER;
    #endif

    runtime = TSENS_TO_AK8963_RUNTIME(_device);
    runtime -> bus = _bus;
    runtime -> addr = _address;

    return ak8963_set_operation_req(_device, AK8963_CONFIGURE_REQ); 
}

TDRVStatus ak8963_read(TDevice *_device, TSensorVector3Float *_data)
{
	AK8963Runtime *runtime;
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device || !_data)
            return TDRV_INVAILD_PARAMETER;
    #endif

    runtime = TSENS_TO_AK8963_RUNTIME(_device);

    _data -> type = TSENS_VECTOR_3_FLOAT;
    _data -> timestamp_us = 0;

    _data -> x = (int16_t)((runtime -> measurement.out.data.x_h << 8) | runtime -> measurement.out.data.x_l);
    _data -> y = (int16_t)((runtime -> measurement.out.data.y_h << 8) | runtime -> measurement.out.data.y_l);
    _data -> z = (int16_t)((runtime -> measurement.out.data.z_h << 8) | runtime -> measurement.out.data.z_l);

    _data -> x *= ((float)runtime -> measurement.adj.x / 256 + 0.5) * 0.15;
    _data -> y *= ((float)runtime -> measurement.adj.y / 256 + 0.5) * 0.15;
    _data -> z *= ((float)runtime -> measurement.adj.z / 256 + 0.5) * 0.15;

    return TDRV_OK;
}

TDRVStatus ak8963_update(TDevice *_device)
{
	AK8963Runtime *runtime;
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device)
            return TDRV_INVAILD_PARAMETER;
    #endif

    runtime = TSENS_TO_AK8963_RUNTIME(_device);

    return ak8963_set_operation_req(_device, AK8963_UPDATE_REQ);
}

TDRVStatus ak8963_listen(TDevice *_device, TSensorHubListener *_listener)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device || !_listener)
            return TDRV_INVAILD_PARAMETER;
    #endif

    return lsnr_hub_listener_connect(&TSENS_TO_AK8963_RUNTIME(_device) -> hub, (TDrvHubListenerBase*)_listener);
}

TDRVStatus ak8963_unlisten(TDevice *_device, TSensorHubListener *_listener)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device || !_listener)
            return TDRV_INVAILD_PARAMETER;
    #endif

    return lsnr_hub_listener_disconnect(&TSENS_TO_AK8963_RUNTIME(_device) -> hub, (TDrvHubListenerBase*)_listener);
}

const TSensorState* ak8963_get_state(TDevice *_device)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device)
            return 0;
    #endif

    return &TSENS_TO_AK8963_RUNTIME(_device) -> state;
}

TAK8963InterfaceType AK8963Interface = {
    ak8963_init
    , ak8963_deinit
	, ak8963_get_state
    , ak8963_read
    , ak8963_update
    , ak8963_listen
    , ak8963_unlisten
    , ak8963_i2c_bind
};
