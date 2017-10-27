#include <string.h>
#include "mpu9250.h"

const char mpu_gyro_name[] = "mpu9250_gyroscope";
const char mpu_accel_name[] = "mpu9250_accelerometer";
const char mpu_vendor[] = "Invensense";

const uint8_t mpu_configure_data[] = {
    0x03    /* Sample rate divider */
    ,0x64   /* Config */
    ,0x13   /* Gyroscope config with 1000dps full scale and DLPF Eanble */
    ,0x10   /* Accelerometer config with 8g full scale */
    ,0x07   /* Accelemeter config 2 */
};

const uint8_t mpu_bypass_i2c = 0x02; /* INT as default. I2C Bypass */
const uint8_t mpu_accel_out_addr = ACCEL_OUT_ADDR;
const uint8_t mpu_gyro_out_addr = GYRO_OUT_ADDR;

void* mpu9250_i2c_init_stage1_callback(TDrvI2CMessage *_msg);
void* mpu9250_i2c_finish_callback(TDrvI2CMessage *_msg);
void* mpu9250_update_gyroscope_finish_callback(TDrvI2CMessage *_msg);
void* mpu9250_update_accelerometer_finish_callback(TDrvI2CMessage *_msg);


TDRVStatus mpu9250_get_req(MPU9250Runtime *_runtime)
{
    uint8_t req_old, req_chg, req_new;

    req_chg = _runtime -> flags;
    do
    {
        // Get a request, and return.
        req_old = req_chg;
        req_new = req_old & TMPU9250_REQ_FIELD_POS;
        if(!req_new)
            break; // No request.
        
        // Clear a request flag.
        req_new = req_old & ~(((req_new - 1) & req_new) ^ req_new);

        req_chg = TDrvCAS8(&_runtime -> flags, req_old, req_new);
    }while(req_chg != req_old);

    return req_chg ^ req_new; // Get the cleared request.
}

TDRVStatus mpu9250_init_by_i2c(TDevice *_device)
{
    MPU9250Runtime *runtime;
    uint8_t old8, chg8;

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);
    for(old8 = runtime -> flags; ; chg8 = old8)
    {
        if(old8 & TMPU9250_CONFIGURED)
            return TDRV_OK;
        chg8 = TDrvCAS8(&runtime -> flags, old8, old8 | TMPU9250_CONFIGURED);
        if(chg8 == old8)
            break;
    }

    runtime -> drv_state = TMPU9250_INITIAL_CONFIGURING_STAGE_1;

    TDrvI2CMessageInitAsWrite(&runtime -> msg[0]);
    runtime -> msg[0].address = runtime -> addr;
    runtime -> msg[0].data = (void*)mpu_configure_data;
    runtime -> msg[0].size = sizeof(mpu_configure_data);
    runtime -> msg[0].callback = mpu9250_i2c_init_stage1_callback;
    runtime -> msg[0].callback_params = runtime;
    TDRV_I2C_API_NEW(runtime -> bus).Put(runtime -> bus, runtime -> msg);

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
    runtime -> msg[1].callback_params = runtime;
    TDrvI2CMessageLink(runtime -> msg + 0, runtime -> msg + 1, TDRV_I2C_MSG_RESTART);
    TDRV_I2C_API_NEW(runtime -> bus).Put(runtime -> bus, runtime -> msg);

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
    runtime -> msg[1].data = &runtime -> gyro.vec;
    runtime -> msg[1].size = sizeof(AccelOutput);
    runtime -> msg[1].address = runtime -> addr;
    runtime -> msg[1].callback = mpu9250_update_accelerometer_finish_callback;
    runtime -> msg[1].callback_params = runtime;
    TDrvI2CMessageLink(runtime -> msg + 0, runtime -> msg + 1, TDRV_I2C_MSG_RESTART);
    TDRV_I2C_API_NEW(runtime -> bus).Put(runtime -> bus, runtime -> msg);

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
        if(old & TMPU9250_REQ_FIELD_MASK)
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



TDRVStatus mpu9250_set_req(TDevice *_device, uint8_t req_mask)
{
    MPU9250Runtime *runtime;
    uint8_t old, chg;

    runtime = TSENS_TO_MPU9250_RUNTIME(_device);

    chg = runtime -> flags;
    {
        old = chg;
        if(old & (req_mask | TMPU9250_OPERATING))
            return TDRV_OK;
        
        chg = TDrvCAS8(&runtime -> flags, old, old | TMPU9250_OPERATING | req_mask);
    }while(old != chg);

    // If driver is not operating, start it.
    if(chg & TMPU9250_OPERATING)
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
    _buffer -> x = ((_runtime -> accel.vec.x_h << 8) | (_runtime -> accel.vec.x_l)) / 1000;
    _buffer -> y = ((_runtime -> accel.vec.y_h << 8) | (_runtime -> accel.vec.y_l)) / 1000;
    _buffer -> z = ((_runtime -> accel.vec.z_h << 8) | (_runtime -> accel.vec.z_l)) / 1000;

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
    _buffer -> x = ((_runtime -> gyro.vec.x_h << 8) | (_runtime -> gyro.vec.x_l)) / 1000;
    _buffer -> y = ((_runtime -> gyro.vec.y_h << 8) | (_runtime -> gyro.vec.y_l)) / 1000;
    _buffer -> z = ((_runtime -> gyro.vec.z_h << 8) | (_runtime -> gyro.vec.z_l)) / 1000;

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

void* mpu9250_i2c_init_stage1_callback(TDrvI2CMessage *_msg)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_msg -> callback_params);

    runtime -> drv_state = TMPU9250_INITIAL_CONFIGURING_STAGE_2;

    TDrvI2CMessageInitAsWrite(_msg);
    _msg -> address = runtime -> addr;
    _msg -> size = 1;
    _msg -> data = (void*)&mpu_bypass_i2c;
    _msg -> callback_params = runtime;
    _msg -> callback = mpu9250_i2c_finish_callback;
    TDRV_I2C_API_NEW(runtime -> bus).Put(runtime -> bus, _msg);

    return 0;
}

void* mpu9250_update_accelerometer_finish_callback(TDrvI2CMessage *_msg)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_msg -> callback_params);
    runtime -> drv_state = TMPU9250_FREE;

    lsnr_hub_notify(&runtime -> accel.hub, TSENS_DATA_UPDATED, _msg -> callback_params);

    return 0;
}

void* mpu9250_update_gyroscope_finish_callback(TDrvI2CMessage *_msg)
{
    MPU9250Runtime *runtime;

    runtime = TSENS_TO_MPU9250_RUNTIME(_msg -> callback_params);
    runtime -> drv_state = TMPU9250_FREE;

    lsnr_hub_notify(&runtime -> gyro.hub, TSENS_DATA_UPDATED, _msg -> callback_params);

    return 0;
}

/* ---------------------------------------- */

