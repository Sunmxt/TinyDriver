#ifndef TINY_DRIVER_SENSOR
#define TINY_DRIVER_SENSOR

#include "tiny_driver.h"
#include "klist.h"
#include "manage.h"

/* --- Sensor Feature ---- */
typedef struct _Tiny_Driver_Sensor_State
{
    const char* name;
    const char* vendor;
    uint32_t type;
    #define TSENS_UNKNOWN_TYPE      0
    #define TSENS_THERMOMETER       1
    #define TSENS_MAGNETMETER       2
    #define TSENS_BAROMETER         3
    #define TSENS_ATTITUDE_TACKER   4
    #define TSENS_GYROSCOPE         5
    #define TSENS_ACCELEROMETER     6

    uint8_t state;
    /*
        Available.

        the sensor is available.
    */
        #define TSENS_STATE_AVAILABLE_POS   0
        #define TSENS_STATE_AVAILABLE       (1u << TSENS_STATE_RUNNING)

    /*
        Can listen.

        The sensor will generate messages when the device is being listened to.
    */
        #define TSENS_CAN_LISTEN_POS        1
        #define TSENS_CAN_LISTEN            (1u << TSENS_CAN_LISTEN_POS)

    /* 
        Message Disabled
        
        The messages of the sensor will be ignored.
    */
        #define TSENS_MESSAGE_DISABLED_POS  2
        #define TSENS_MESSAGE_DISABLED      (1u << TSENS_MESSAGE_DISABLED_POS)
}TSensorState;

/* ------ Sensor Message ------ */
#define TSENSOR_DATA_COMMON_HEADER() uint32_t type; uint64_t timestamp_us

typedef struct _Tiny_Driver_Sensor_Data
{
    /*
        Common header
    */
    uint32_t type;
    #define TSENS_UNKNOWN_TYPE      0
    #define TSENS_VECTOR_3_FLOAT    1
    #define TSENS_VECTOR_3_DOUBLE   2
    #define TSENS_FLOAT             3
    #define TSENS_DOUBLE            4
    #define TSENS_INTEGAR_8         5
    #define TSENS_INTEGAR_16        6
    #define TSENS_INTEGAR_32        7
    #define TSENS_INTEGAR_64        8
    #define TSENS_UNSIGNED_8        9
    #define TSENS_UNSIGNED_16       10
    #define TSENS_UNSIGNED_32       11
    #define TSENS_UNSIGNED_64       12
    #define TSENS_FLOAT_ARRAY       13
    #define TSENS_DOUBLE_ARRAY      14
    #define TSENS_INTEGAR_8_ARRAY   15
    uint64_t timestamp_us;

    /* Data field */
    uint8_t data[1];
}TSensorData;

typedef struct _Tiny_Driver_Sensor_Vector_3_Data_Float
{
    TSENSOR_DATA_COMMON_HEADER();
    float x, y, z;
}TSensorVector3Float;

typedef struct _Tiny_Driver_Sensor_Vector_3_Data_Double
{
    TSENSOR_DATA_COMMON_HEADER();
    double x, y, z;
}TSensorVector3Double;

/* ---------------------------- */

/* ------ Listener Hub ------ */


typedef struct _Tiny_Driver_Sensor_Hub_Listener
{
    TDRV_HUB_LISTENER_COMMON_HEADER();

    #define TSENS_DATA_UPDATED      TDRV_LSNR_USER_MSG
    #define TSENS_USER_MSG          (TDRV_LSNR_USER_MSG + 30)
    void *user_params;
}TSensorHubListener;


/* --------------------------- */


typedef struct _Tiny_Driver_Sensor_Interface
{
    TDrvHAInit init;
    TDrvHADeinit deinit;

    const TSensorState*     (*GetState)(TDevice *_device);
    TDRVStatus              (*Read)(TDevice *_device, void* _data);
    TDRVStatus              (*Update)(TDevice *_device);
    TDRVStatus              (*Listen)(TDevice *_device, TSensorHubListener *_token);
    TDRVStatus              (*Unlisten)(TDevice *_device, TSensorHubListener *_token);
}TDrvSensorInterface;

#define TDRV_SENSOR_API(_device) TDRV_API(TDrvSensorInterface, _device)

#endif
