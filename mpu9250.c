#include <string.h>
#include "mpu9250.h"

const char mpu_gyro_name[] = "mpu9250_gyroscope";
const char mpu_accel_name[] = "mpu9250_accelerometer";
const char mpu_vendor[] = "Invensense";

/* ----------------------------------------*/
/*  Init message sequence                  */
/* ----------------------------------------*/
const uint8_t mpu_configure_data[] = {
    SMPLRT_DIV_ADDR
    ,0x07    /* Sample rate divider */
    ,0x01   /* Config */
    ,0x10   /* Gyroscope config with 1000dps full scale and DLPF Eanble */
    ,0x10   /* Accelerometer config with 8g full scale */
    ,0x00   /* Accelemeter config 2 */
};

const uint8_t mpu_bypass_i2c[] =
    {
        INT_PIN_CFG_ADDR
        ,0x02 /* INT as default. I2C Bypass */
    };

uint8_t mpu_power_up_config[] =
    {
        PWR_MGMT_1_ADDR
        , 0x80
    };


const uint8_t mpu_accel_out_addr = ACCEL_OUT_ADDR;
const uint8_t mpu_gyro_out_addr = GYRO_OUT_ADDR;

void* mpu9250_i2c_init_stage1_callback(TDrvI2CMessage *_msg);
void* mpu9250_i2c_finish_callback(TDrvI2CMessage *_msg);
void* mpu9250_update_gyroscope_finish_callback(TDrvI2CMessage *_msg);
void* mpu9250_update_accelerometer_finish_callback(TDrvI2CMessage *_msg);
void* mpu9250_i2c_init_stage1_callback(TDrvI2CMessage *_msg);
TDRVStatus mpu9250_do_req(TDevice *_device);


TDRVStatus mpu9250_get_req(MPU9250Runtime *_runtime)
{
    uint8_t req_old, req_chg, req_new;

    req_chg = _runtime -> flags;
    do
    {
        // Get a request, and return.
        req_old = req_chg;
        req_new = req_old & TMPU9250_REQ_FIELD;
        if(!req_new)
            break; // No request.
        
        // Clear a request flag.
        req_new = req_old & ~(((req_new - 1) & req_new) ^ req_new);

        req_chg = TDrvCAS8(&_runtime -> flags, req_old, req_new);
    }while(req_chg != req_old);

    return (req_chg ^ req_new) & TMPU9250_REQ_FIELD; // Get the cleared request.
}

void mpu9250_i2c_configure_put(TDevice *_device);
void mpu9250_i2c_wait_reset_put(TDevice* _device);

void* mpu9250_i2c_init_stage3_callback(TDrvI2CMessage *_msg)
{
    TDevice *device;
    MPU9250Runtime *runtime;

    device = (TDevice*)_msg -> callback_params;
    runtime = TSENS_TO_MPU9250_RUNTIME(device);

    if(_msg -> attribute & TDRV_I2C_FAILED)
    {
        if(TMPU9250_INITIAL_CONFIGURING_STAGE_3 == runtime -> drv_state )
            runtime -> drv_state = TMPU9250_CONFIGURING_LOST_RECOVER_1;
        else if(TMPU9250_CONFIGURING_LOST_RECOVER_6 == runtime -> drv_state)
        {
            runtime -> drv_state = TMPU9250_DEVICE_LOST;
            mpu9250_do_req(device);
            return 0;
        }
        runtime -> drv_state++; // Retry.
        mpu9250_i2c_configure_put(device);
        return 0;
    }

    runtime -> drv_state = TMPU9250_FREE;

    mpu9250_do_req(device);
    return 0;
}

void mpu9250_i2c_configure_put(TDevice *_device)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);
    TDrvI2CMessageInitAsWrite(&runtime -> msg[0]);
    runtime -> msg[0].address = runtime -> addr;
    runtime -> msg[0].data = (void*)mpu_configure_data;
    runtime -> msg[0].size = sizeof(mpu_configure_data);
    TDrvI2CMessageInitAsWrite(&runtime -> msg[1]);
    runtime -> msg[1].address = runtime -> addr;
    runtime -> msg[1].data = (void*)mpu_bypass_i2c;
    runtime -> msg[1].size = sizeof(mpu_bypass_i2c);
    runtime -> msg[1].callback = mpu9250_i2c_init_stage3_callback;
    runtime -> msg[1].callback_params = _device;
    TDrvI2CMessageLink(runtime -> msg + 0, runtime -> msg + 1, 0);
    TDRV_I2C_API(runtime -> bus).Put(runtime -> bus, runtime -> msg);
}

void* mpu9250_i2c_init_stage2_callback(TDrvI2CMessage *_msg)
{
    MPU9250Runtime *runtime;
    TDevice *device;
    
    device = (TDevice*)_msg -> callback_params;
    runtime = TSENS_TO_MPU9250_RUNTIME(_msg -> callback_params);
    runtime -> drv_state = TMPU9250_INITIAL_CONFIGURING_STAGE_3;

    // configure
    mpu9250_i2c_configure_put(device);

    return 0;
}

void mpu9250_i2c_wait_reset_put(TDevice *_device)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);

    // Wait for reset.
    TDrvI2CMessageInitAsWrite(&runtime -> msg[0]);
    runtime -> msg[0].address = runtime -> addr;
    runtime -> msg[0].data = (void*)&mpu_power_up_config[0];
    runtime -> msg[0].size = 1;
    TDrvI2CMessageInitAsRead(&runtime -> msg[1]);
    runtime -> msg[1].address = runtime -> addr;
    runtime -> msg[1].data = (void*)&mpu_power_up_config[1];
    runtime -> msg[1].callback = mpu9250_i2c_init_stage1_callback;
    runtime -> msg[1].size = 1;
    runtime -> msg[1].callback_params = _device;
    TDrvI2CMessageLink(runtime -> msg + 0, runtime -> msg + 1, TDRV_I2C_MSG_RESTART);
    TDRV_I2C_API(runtime -> bus).Put(runtime -> bus, runtime -> msg);
}

void* mpu9250_i2c_init_stage1_callback(TDrvI2CMessage *_msg)
{
    MPU9250Runtime *runtime;
    TDevice *device;
    
    device = (TDevice*)_msg -> callback_params;
    runtime = TSENS_TO_MPU9250_RUNTIME(device);
    
    if(_msg -> attribute & TDRV_I2C_FAILED )
    {
        if(runtime -> drv_state != TMPU9250_DEVICE_LOST)
        {
            if( !(_msg -> attribute & TDRV_I2C_NO_ACK) )
            {
                TDrvExclusiveAnd8(&runtime -> flags, ~TMPU9250_CONFIGURED);
                mpu9250_do_req(device);
                return 0;
            } else
            {
                if(runtime -> drv_state == TMPU9250_RESETTING)
                    runtime -> drv_state = TMPU9250_CONFIGURING_LOST_RECOVER_1;
                else
                    runtime -> drv_state++;
                mpu9250_i2c_wait_reset_put(device);
                return 0;
            }
        }
        else
        {
            TDrvExclusiveAnd8(&runtime -> flags, ~TMPU9250_CONFIGURED);
            mpu9250_do_req(device);
            return 0;
        }
    }

    if( mpu_power_up_config[1] & 0x80 ) // Check reset
    {
        runtime -> drv_state = TMPU9250_RESETTING;
        mpu9250_i2c_wait_reset_put(device);
        return 0;
    }

    runtime -> drv_state = TMPU9250_INITIAL_CONFIGURING_STAGE_2;
    TDrvI2CMessageInitAsWrite(&runtime -> msg[0]);
    mpu_power_up_config[1] = 0x01;
    runtime -> msg[0].address = runtime -> addr;
    runtime -> msg[0].data = (void*)mpu_power_up_config;
    runtime -> msg[0].size = sizeof(mpu_power_up_config);
    runtime -> msg[0].callback = mpu9250_i2c_init_stage2_callback;
    runtime -> msg[0].callback_params = device;

    TDRV_I2C_API(runtime -> bus).Put(runtime -> bus, runtime -> msg);
    return 0;
}

void* mpu9250_i2c_reset_callback(TDrvI2CMessage *_msg)
{
    TDevice *device;
    device = (TDevice*)_msg -> callback_params;

    mpu9250_i2c_wait_reset_put(device);

    return TDRV_OK;
}

TDRVStatus mpu9250_init_by_i2c(TDevice *_device)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);
    TDrvExclusiveOr8(&runtime -> flags, TMPU9250_CONFIGURED);

    runtime -> drv_state = TMPU9250_RESETTING;
    mpu_power_up_config[1] = 0x03;

    // Reset device.
    TDrvI2CMessageInitAsWrite(&runtime -> msg[0]);
    runtime -> msg[0].address = runtime -> addr;
    runtime -> msg[0].data = (void*)mpu_power_up_config;
    runtime -> msg[0].size = sizeof(mpu_power_up_config);
    runtime -> msg[0].callback = mpu9250_i2c_reset_callback;
    //runtime -> msg[0].callback = mpu9250_i2c_init_stage2_callback;
    runtime -> msg[0].callback_params = _device;

    TDRV_I2C_API(runtime -> bus).Put(runtime -> bus, runtime -> msg);

    return TDRV_OK;
}

TDRVStatus mpu9250_do_update_gyroscope(TDevice *_device)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);

    runtime -> drv_state = TMPU9250_GYROSCOPE_UPDATING;

    TDrvI2CMessageInitAsWrite(&runtime -> msg[0]);
    TDrvI2CMessageInitAsRead(&runtime -> msg[1]);
    runtime -> msg[0].data = (void*)&mpu_gyro_out_addr;
    runtime -> msg[0].size = 1;
    runtime -> msg[0].address = runtime -> addr;
    runtime -> msg[1].data = &runtime -> gyro.vec;
    runtime -> msg[1].size = sizeof(GyroOutput);
    runtime -> msg[1].address = runtime -> addr;
    runtime -> msg[1].callback = mpu9250_update_gyroscope_finish_callback;
    runtime -> msg[1].callback_params = _device;
    TDrvI2CMessageLink(runtime -> msg + 0, runtime -> msg + 1, TDRV_I2C_MSG_RESTART);
    TDRV_I2C_API(runtime -> bus).Put(runtime -> bus, runtime -> msg);

    return TDRV_OK;
}

TDRVStatus mpu9250_do_update_acceleration(TDevice *_device)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);
    runtime -> drv_state = TMPU9250_ACCELEROMETER_UPDATING;

    TDrvI2CMessageInitAsWrite(runtime -> msg + 0);
    TDrvI2CMessageInitAsRead(runtime -> msg + 1);
    runtime -> msg[0].data = (void*)&mpu_accel_out_addr;
    runtime -> msg[0].size = 1;
    runtime -> msg[0].address = runtime -> addr;
    runtime -> msg[1].data = &runtime -> accel.vec;
    runtime -> msg[1].size = sizeof(AccelOutput);
    runtime -> msg[1].address = runtime -> addr;
    runtime -> msg[1].callback = mpu9250_update_accelerometer_finish_callback;
    runtime -> msg[1].callback_params = _device;
    TDrvI2CMessageLink(runtime -> msg + 0, runtime -> msg + 1, TDRV_I2C_MSG_RESTART);
    TDRV_I2C_API(runtime -> bus).Put(runtime -> bus, runtime -> msg);

    return TDRV_OK;
}

int mpu9250_stop_operating(MPU9250Runtime *_runtime)
{
    uint8_t old, chg;

    // Try clear operating flag.
    chg = _runtime -> flags;
    do
    {
        old = chg;
        if(old & TMPU9250_REQ_FIELD)
            return 0; // New request.
        else if(!(old & TMPU9250_OPERATING))
            return 1;
        chg = TDrvCAS8(&_runtime -> flags, old, old & ~(TMPU9250_OPERATING));
    }while(chg != old);

    return 1;
}

TDRVStatus mpu9250_do_req(TDevice *_device)
{
    MPU9250Runtime *runtime;
    uint8_t req;

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);
    // Get new operation
    for(;;)
    {
        req = mpu9250_get_req(runtime);
        if(!req)
        {
            if(mpu9250_stop_operating(runtime))
                break;
            continue;
        }
        switch(req)
        {
        case TMPU9250_INIT_REQ:
            return mpu9250_init_by_i2c(_device);
        case TMPU9250_ACCEL_UPDATE_REQ:
            return mpu9250_do_update_acceleration(_device);
        case TMPU9250_GYRO_UPDATE_REQ:
            return mpu9250_do_update_gyroscope(_device);
        }
    }
    
    return TDRV_OK;
}

uint8_t mpu9250_check_and_set_init(MPU9250Runtime *_runtime)
{
    uint8_t chg8, old8;
    for(old8 = _runtime -> flags; ; chg8 = old8)
    {
        if(old8 & TMPU9250_INITIALIZED)
            return 1;
        chg8 = TDrvCAS8(&_runtime -> flags, old8 ,old8 | TMPU9250_INITIALIZED);
        if(chg8 == old8)
            break;
    }

    return 0;
}

TDRVStatus mpu9250_init(TDevice *_device)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);
    if(mpu9250_check_and_set_init(runtime))
       return TDRV_OK;

    runtime -> bus = 0;
    runtime -> addr = 0;
    runtime -> gyro.state.name = mpu_gyro_name;
    runtime -> gyro.state.vendor = mpu_vendor;
    runtime -> gyro.state.type = TSENS_GYROSCOPE;
    memset(&runtime -> gyro.vec, 0, sizeof(GyroOutput));
    lsnr_hub_create(&runtime -> gyro.hub);

    runtime -> accel.state.name = mpu_accel_name;
    runtime -> accel.state.vendor = mpu_vendor;
    runtime -> accel.state.type = TSENS_ACCELEROMETER;
    memset(&runtime -> accel.vec, 0, sizeof(AccelOutput));
    lsnr_hub_create(&runtime -> accel.hub);

    runtime -> drv_state = TMPU9250_FREE;

    return TDRV_OK;
}

TDRVStatus mpu9250_deinit(TDevice *_device)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device)
            return TDRV_INVAILD_PARAMETER;
    #endif

    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);
    runtime -> flags = 0;

    return TDRV_OK;
}

TDRVStatus mpu9250_accelerometer_listen(TDevice *_device, TSensorHubListener *_listener)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device || !_listener)
            return TDRV_INVAILD_PARAMETER;
    #endif

    return lsnr_hub_listener_connect(&TSENS_TO_MPU9250_RUNTIME(_device) -> accel.hub, (TDrvHubListenerBase*)_listener);
}

TDRVStatus mpu9250_accelerometer_unlisten(TDevice *_device, TSensorHubListener *_listener)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device || !_listener)
            return TDRV_INVAILD_PARAMETER;
    #endif

    return lsnr_hub_listener_disconnect(&TSENS_TO_MPU9250_RUNTIME(_device) -> accel.hub, (TDrvHubListenerBase*)_listener);
}

TDRVStatus mpu9250_gyroscope_listen(TDevice *_device, TSensorHubListener *_listener)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device || !_listener)
            return TDRV_INVAILD_PARAMETER;
    #endif

    return lsnr_hub_listener_connect(&TSENS_TO_MPU9250_RUNTIME(_device) -> gyro.hub, (TDrvHubListenerBase*)_listener);
}

TDRVStatus mpu9250_gyroscope_unlisten(TDevice *_device, TSensorHubListener *_listener)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device || !_listener)
            return TDRV_INVAILD_PARAMETER;
    #endif

    return lsnr_hub_listener_disconnect(&TSENS_TO_MPU9250_RUNTIME(_device) -> gyro.hub, (TDrvHubListenerBase*)_listener);
}

TDRVStatus mpu9250_set_req(TDevice *_device, uint8_t req_mask)
{
    MPU9250Runtime *runtime;
    uint8_t old, chg;

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);

    chg = runtime -> flags;
    {
        old = chg;
        if(old & req_mask )
            return TDRV_OK;
        
        chg = TDrvCAS8(&runtime -> flags, old, old | TMPU9250_OPERATING | req_mask);
    }while(old != chg);

    // If driver is not operating, start it.
    if(!(chg & TMPU9250_OPERATING))
        return mpu9250_do_req(_device);

    return TDRV_OK;
}


TDRVStatus mpu9250_load_i2c_info(TDevice *_device, TDevice *_bus, uint8_t _address)
{
    MPU9250Runtime *runtime;
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device || !_bus)
            return TDRV_INVAILD_PARAMETER;
    #endif
    runtime = TSENS_TO_MPU9250_RUNTIME(_device);

    runtime -> bus = _bus;
    runtime -> addr = _address & (~1u);

    return mpu9250_set_req(_device, TMPU9250_INIT_REQ);
}

/* ---------- Accelerometer ---------- */
const TSensorState* mpu9250_get_state_accelerometer(TDevice *_device)
{
    MPU9250Runtime *runtime;

    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device)
            return 0;
    #endif

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);
    return &runtime -> accel.state;
}

TDRVStatus mpu9250_read_accelerometer(MPU9250Runtime *_runtime, TSensorVector3Float *_buffer)
{
    _buffer -> type = TSENS_VECTOR_3_FLOAT;
    _buffer -> timestamp_us = 0;

    _buffer -> x = - (int16_t)((_runtime -> accel.vec.x_h << 8) | (_runtime -> accel.vec.x_l));
    _buffer -> y = - (int16_t)((_runtime -> accel.vec.y_h << 8) | (_runtime -> accel.vec.y_l));
    _buffer -> z = - (int16_t)((_runtime -> accel.vec.z_h << 8) | (_runtime -> accel.vec.z_l));
    _buffer -> x /= 4096;
    _buffer -> y /= 4096;
    _buffer -> z /= 4096;

    return TDRV_OK;
}

TDRVStatus mpu9250_read_accelerometer_wrapper(TDevice *_device, TSensorVector3Float *_buffer)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device || !_buffer)
            return TDRV_INVAILD_PARAMETER;
    #endif

    return mpu9250_read_accelerometer(TSENS_TO_MPU9250_RUNTIME(_device), _buffer);
}

TDRVStatus mpu9250_update_accelerometer(TDevice *_device)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device)
            return TDRV_INVAILD_PARAMETER;
    #endif

    return mpu9250_set_req(_device, TMPU9250_ACCEL_UPDATE_REQ);
}

/* ---------- Gyroscope ---------- */

const TSensorState* mpu9250_get_state_gyroscope(TDevice *_device)
{
    MPU9250Runtime *runtime;

    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device)
            return 0;
    #endif

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);
    return &runtime -> gyro.state;
}

TDRVStatus mpu9250_update_gyroscope(TDevice *_device)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device)
            return TDRV_INVAILD_PARAMETER;
    #endif

    return mpu9250_set_req(_device, TMPU9250_GYRO_UPDATE_REQ);
}

TDRVStatus mpu9250_read_gyroscope(MPU9250Runtime *_runtime, TSensorVector3Float *_buffer)
{
    _buffer -> type = TSENS_VECTOR_3_FLOAT;
    _buffer -> timestamp_us = 0;
    _buffer -> x = (int16_t)((_runtime -> gyro.vec.x_h << 8) | (_runtime -> gyro.vec.x_l));
    _buffer -> y = (int16_t)((_runtime -> gyro.vec.y_h << 8) | (_runtime -> gyro.vec.y_l));
    _buffer -> z = (int16_t)((_runtime -> gyro.vec.z_h << 8) | (_runtime -> gyro.vec.z_l));
    _buffer -> x *=  (float)1000 / 32768;
    _buffer -> y *=  (float)1000 / 32768;
    _buffer -> z *=  (float)1000 / 32768;
    return TDRV_OK;
}

TDRVStatus mpu9250_read_gyroscope_wrapper(TDevice *_device, TSensorVector3Float *_buffer)
{
    #ifndef TDRV_NO_PARAMETER_CHECK
        if(!_device)
            return TDRV_INVAILD_PARAMETER;
    #endif

    return mpu9250_read_gyroscope(TSENS_TO_MPU9250_RUNTIME(_device), _buffer);
}

/* ---------- Callback Procedure ---------- */

void* mpu9250_i2c_finish_callback(TDrvI2CMessage *_msg)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_msg -> callback_params);
    runtime -> drv_state = TMPU9250_FREE;

    mpu9250_do_req((TDevice*)_msg -> callback_params);
    
    return 0;
}


void* mpu9250_update_accelerometer_finish_callback(TDrvI2CMessage *_msg)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_msg -> callback_params);
    runtime -> drv_state = TMPU9250_FREE;

    switch(lsnr_hub_notify(&runtime -> accel.hub, TSENS_DATA_UPDATED, _msg -> callback_params))
    {
    case TDRV_OK:
    case TDRV_ILLEGAL_STATE:
        break;
    default:
        mpu9250_set_req((TDevice*)_msg -> callback_params, TMPU9250_ACCEL_UPDATE_REQ);
        break;
    }

    mpu9250_do_req((TDevice*)_msg -> callback_params);

    return 0;
}

void* mpu9250_update_gyroscope_finish_callback(TDrvI2CMessage *_msg)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_msg -> callback_params);
    runtime -> drv_state = TMPU9250_FREE;

    switch(lsnr_hub_notify(&runtime -> gyro.hub, TSENS_DATA_UPDATED, _msg -> callback_params))
    {
    case TDRV_OK:
    case TDRV_ILLEGAL_STATE:
        break;
    default:
        mpu9250_set_req((TDevice*)_msg -> callback_params, TMPU9250_GYRO_UPDATE_REQ);
        break;
    }

    mpu9250_do_req((TDevice*)_msg -> callback_params);
    return 0;
}

/* Interface Export */

const MPU9250InterfaceType MPU9250GyroscopeInterface =
{
    mpu9250_init
    , mpu9250_deinit
    , mpu9250_get_state_gyroscope
    , mpu9250_read_gyroscope_wrapper
    , mpu9250_update_gyroscope
    , mpu9250_gyroscope_listen
    , mpu9250_gyroscope_unlisten
    , mpu9250_load_i2c_info
};

const MPU9250InterfaceType MPU9250AccelerometerInterface = {
    mpu9250_init
    , mpu9250_deinit
    , mpu9250_get_state_accelerometer
    , mpu9250_read_accelerometer_wrapper
    , mpu9250_update_accelerometer
    , mpu9250_accelerometer_listen
    , mpu9250_accelerometer_unlisten
    , mpu9250_load_i2c_info
};
