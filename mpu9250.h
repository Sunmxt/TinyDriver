/*
    
    MPU9250 Driver

*/
#ifndef TINY_DRIVER_MPU9250_DRIVER
#define TINY_DRIVER_MPU9250_DRIVER

#include "sensor.h"
#include "i2c.h"

/* ---------- Registers --------- */
#define SELF_TEST_GYRO_ADDR         0x00
typedef struct MPUXXXX_Self_Test_Gyroscope
{ uint8_t x, y, z; } GyroSelfTest;

#define SELF_TEST_ACCEL             0x0D
typedef struct MPUXXXX_Self_Test_Acceleration
{ uint8_t x, y, z; } AccelSelfTest;

#define GYRO_OFFSET_ADDR            0x13
typedef struct MPUXXXX_Offset_Gyroscope
{ uint8_t x_h, x_l, y_h, y_l, z_h, z_l; } GyroOffset;

#define SMPLRT_DIV_ADDR             0x19
#define CONFIG_ADDR                 0x1A
#define GYRO_CONFIG_ADDR            0x1B

#define ACCEL_CONFIG                0x1C
#define ACCEL_CONFIG_2_ADDR         0x1D

#define LP_ACCEL_ODR_ADDR           0x1E
#define WOM_THR_ADDR                0x1F
#define FIFO_EN_ADDR                0x23

#define I2C_MST_CTRL                0x24
#define I2C_MST_DELAY_CTRL          0x67

#define I2C_SLAVE0_ADDR             0x25
#define I2C_SLAVE1_ADDR             0x28
#define I2C_SLAVE2_ADDR             0x2B
#define I2C_SLAVE3_ADDR             0x2E
#define I2C_SLV0_DO                 0x63
#define I2C_SLV1_DO                 0x64
#define I2C_SLV2_DO                 0x65
#define I2C_SLV3_DO                 0x66
typedef struct MPUXXXX_I2C_Slave_Configure
{
    uint8_t I2C_SLV_ADDR;
    uint8_t I2C_SLV_REG;
    uint8_t I2C_SLV_CTRL;
}I2CSlaveConfig;
#define I2C_SLAVE4_ADDR             0x31
typedef struct MPUXXXX_I2C_Slave4_Configure
{
    uint8_t I2C_SLV4_ADDR;
    uint8_t I2C_SLV4_REG;
    uint8_t I2C_SLV4_DO;
    uint8_t I2C_SLV4_CTRL;
    uint8_t I2C_SLV4_DI;
}I2CSlave4Config;

#define I2C_MST_STATUS_ADDR         0x36
#define INT_PIN_CFG_ADDR            0x37
#define INT_ENABLE_ADDR             0x38
#define INT_STATUS_ADDR             0x3A

#define ACCEL_OUT_ADDR              0x3B
typedef struct MPUXXXX_Acceleration_Output
{
    uint8_t x_h, x_l;
    uint8_t y_h, y_l;
    uint8_t z_h, z_l;
}AccelOutput;

#define TEMP_OUT_ADDR               0x41
typedef struct MPUXXXX_Temperature_Output
{
    uint8_t temp_h;
    uint8_t temp_l;
}TempOutput;

#define GYRO_OUT_ADDR               0x43
#pragma pack(1)
typedef struct MPUXXXX_Gyroscope_Output
{
    uint8_t x_l;
	uint8_t x_h;
    uint8_t y_l;
	uint8_t y_h;
    uint8_t z_l;
	uint8_t z_h;
}GyroOutput;
#pragma pack()

#define EXT_SENS_DATA_BASE          0x49
#define EXT_SENS_DATA_MAX_SIZE      24

#define SIGNAL_PATH_RESET_ADDR      0x68
#define MOT_DETECT_CTRL_ADDR        0x69
#define USER_CTRL_ADDR              0x6A
#define PWR_MGMT_1_ADDR             0x6B
#define PWR_MGMT_2_ADDR             0x6C

#define FIFO_COUNT_ADDR             0x72
typedef struct MPUXXXX_FIFO_Count
{
    uint8_t h, l;
    uint8_t r_w;
}FIFOCount;
#define FIFO_R_W_ADDR               0x74

#define WHO_AM_I                    0x75
#define ACCEL_OFFSET_ADDR           0x77
#pragma pack(1)
typedef struct MPUXXXX_Acceleration_Offset
{
    uint8_t x_h;
	uint8_t x_l;
    uint8_t y_h;
    uint8_t y_l;
    uint8_t z_h;
    uint8_t z_l;
}AccelOffset;
#pragma pack()
/* ---------- Runtime ---------- */


typedef struct _MPU9250_Runtime_Infomation
{
    TDevice *bus;
    uint8_t addr;
    uint8_t flags;
    #define TMPU9250_INITIALIZED_POS        0
    #define TMPU9250_INITIALIZED            (1u << TMPU9250_INITIALIZED_POS)
    #define TMPU9250_CONFIGURED_POS         1
    #define TMPU9250_CONFIGURED             ((uint8_t)1u << TMPU9250_CONFIGURED_POS)
    #define TMPU9250_OPERATING_POS          2
    #define TMPU9250_OPERATING              (1u << TMPU9250_OPERATING_POS)
    #define TMPU9250_ON_RESET_POS           3
    #define TMPU9250_ON_RESET               (1u << TMPU9250_ON_RESET_POS)


    #define TMPU9250_REQ_FIELD_POS          4
    #define TMPU9250_REQ_FIELD              (((uint8_t)(-1)) << TMPU9250_REQ_FIELD_POS)
    #define TMPU9250_GYRO_UPDATE_REQ_POS    (TMPU9250_REQ_FIELD_POS + 0)
    #define TMPU9250_GYRO_UPDATE_REQ        (1u << TMPU9250_GYRO_UPDATE_REQ_POS)
    #define TMPU9250_ACCEL_UPDATE_REQ_POS   (TMPU9250_REQ_FIELD_POS + 1)
    #define TMPU9250_ACCEL_UPDATE_REQ       (1u << TMPU9250_ACCEL_UPDATE_REQ_POS)
    #define TMPU9250_INIT_REQ_POS           (TMPU9250_REQ_FIELD_POS + 2)
    #define TMPU9250_INIT_REQ               (1u << TMPU9250_INIT_REQ_POS)

    uint8_t drv_state;
    #define TMPU9250_FREE                           0
    #define TMPU9250_INITIAL_CONFIGURING_STAGE_1    1
    #define TMPU9250_INITIAL_CONFIGURING_STAGE_2    2
    #define TMPU9250_INITIAL_CONFIGURING_STAGE_3    3
    #define TMPU9250_GYROSCOPE_UPDATING             4
    #define TMPU9250_ACCELEROMETER_UPDATING         5
    #define TMPU9250_CONFIGURING_LOST_RECOVER_1     6
    #define TMPU9250_CONFIGURING_LOST_RECOVER_2     7
    #define TMPU9250_CONFIGURING_LOST_RECOVER_3     8
    #define TMPU9250_CONFIGURING_LOST_RECOVER_4     9
    #define TMPU9250_CONFIGURING_LOST_RECOVER_5     10
    #define TMPU9250_CONFIGURING_LOST_RECOVER_6     11
    #define TMPU9250_DEVICE_LOST                    12
    #define TMPU9250_RESETTING                      13

    
    TDrvI2CMessage msg[2];

    struct
    {
        TSensorState state;
        TDrvListenerHub hub;
        GyroOutput vec;
    }gyro;

    struct
    {
        TSensorState state;
        TDrvListenerHub hub;
        AccelOutput vec;
    }accel;
}MPU9250Runtime;

/* ---------- Interface ---------- */

typedef struct _MPU9250_Interface
{
    TDrvHAInit init;
    TDrvHADeinit deinit;

    const TSensorState*     (*GetState)(TDevice *_device);
    TDRVStatus              (*Read)(TDevice *_device, TSensorVector3Float *_buffer);
    TDRVStatus              (*Update)(TDevice *_device);
    TDRVStatus              (*Listen)(TDevice *_device, TSensorHubListener *_listener);
    TDRVStatus              (*Unlisten)(TDevice *_device, TSensorHubListener *_listener);

    TDRVStatus              (*LoadI2CInfo)(TDevice *_device, TDevice *_bus, uint8_t _address);
}MPU9250InterfaceType;

#define TDRV_MPU9250_API(_device) TDRV_API(MPU9250InterfaceType, _device)
#define TSENS_TO_MPU9250_RUNTIME(_device) ((MPU9250Runtime*)((TDevice*)_device) -> private_data)
extern const MPU9250InterfaceType MPU9250GyroscopeInterface;
extern const MPU9250InterfaceType MPU9250AccelerometerInterface;

#endif

