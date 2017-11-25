#include "sensor.h"
#include "i2c.h"

/*
    AK8963 Registers
*/

#define AK8963_DEVICE_ID_ADDR       0x00
#define AK8963_INFO_ADDR            0x01
#define AK8963_STATUS_1_ADDR        0x02

#define AK8963_OUTPUT_ADDR          0x03
typedef struct _AK8963_Measurement_Output {
    uint8_t x_l, x_h;
    uint8_t y_l, y_h;
    uint8_t z_l, z_h;
}TAK8963Output;

#define AK8963_STATUS_2_ADDR            0x09

typedef struct _AK8963_Measurement_Read_Pack {
	uint8_t st1;
	TAK8963Output data;
	uint8_t st2;
}TAK8963OutputPack;

// Control Register
#define AK8963_CONTROL_1_ADDR           0x0A
    #define AK8963_POWERDOWN_MODE                   0x00
    #define AK8963_SINGLE_MEASUREMENT_MODE          0x01
    #define AK8963_CONTINUOUS_MEASUREMENT_MODE_1    0x02
    #define AK8963_CONTINUOUS_MEASUREMENT_MODE_2    0x06
    #define AK8963_TIGGER_MEASUREMENT_MODE          0x04
    #define AK8963_SELF_TEST_MODE                   0x08
    #define AK8963_FUSE_ROM_ACCESS_MODE             0x0F

    #define AK8963_OUTPUT_14_BIT                    0x00
    #define AK8963_OUTPUT_16_BIT                    0x10
#define AK8963_CONTROL_2_ADDR           0x0B
    #define AK8963_RESET                            0x01


#define AK8963_SELF_TEST_ADDR           0x0C
#define AK8963_TEST_1_ADDR              0x0D
#define AK8963_TEST_2_ADDR              0x0E
#define AK8963_I2C_DISABLE_ADDR         0x0F
#define AK8963_SENSITIVITY_ADJUST_ADDR  0x10
typedef struct _AK8963_Sensitivity_Adjustment
{
    uint8_t x;
    uint8_t y;
    uint8_t z;
}TAK8963AdjustData;


/*
    Driver runtime
*/

typedef struct _AK8963_Runtime_Infomarion {
    TDevice     *bus;
    uint8_t     addr;
    uint8_t     flags;
    #define     AK8963_CONFIGURED_POS       0
    #define     AK8963_CONFIGURED           (1u << AK8963_CONFIGURED_POS)
    #define     AK8963_OPERATING_POS        1
    #define     AK8963_OPERATING            (1u << AK8963_OPERATING_POS)
    #define     AK8963_REQUEST_MASK_POS     2
    #define     AK8963_REQUEST_MASK         (((uint8_t)-1) << AK8963_REQUEST_MASK_POS)

    #define     AK8963_CONFIGURE_REQ_POS    (AK8963_REQUEST_MASK_POS + 0)
    #define     AK8963_CONFIGURE_REQ        (1u << AK8963_CONFIGURE_REQ_POS)
    #define     AK8963_UPDATE_REQ_POS       (AK8963_REQUEST_MASK_POS + 1)
    #define     AK8963_UPDATE_REQ           (1u << AK8963_UPDATE_REQ_POS)

    uint8_t     drv_state;
    #define TAK8963_DUMMY_STATE                 0
    #define TAK8963_FREE                        1
    #define TAK8963_CONFIGURING                 2
    #define TAK8963_DEVICE_LOST                 3
    #define TAK8963_DEVICE_ERROR                4
    #define TAK8963_BUS_ERROR                   5
    #define TAK8963_UPDATE_STAGE_1              6
    #define TAK8963_UPDATE_STAGE_2              7
    #define TAK8963_UPDATE_NOTIFY               8
    #define TAK8963_CONFIG_ERR_RECOVER_1        9
    #define TAK8963_CONFIG_ERR_RECOVER_2        10
    #define TAK8963_CONFIG_ERR_RECOVER_3        11
    #define TAK8963_CONFIGURE_ERROR             12



    TDrvI2CMessage          message[2];
    TSensorState            state;

    TDrvListenerHub         hub; 

    struct {
        TAK8963AdjustData   adj;
        TAK8963OutputPack   out;
    }measurement;
}AK8963Runtime;

#define TSENS_TO_AK8963_RUNTIME(_device)    ((AK8963Runtime*)((_device) -> private_data))

/*
    Notification message
*/


/*
    Interfaces
*/

typedef struct _Tiny_Driver_AK8963_Interface
{
    TDrvHAInit init;
    TDrvHADeinit deinit;

    const TSensorState*     (*GetState)(TDevice *_device);
    TDRVStatus              (*Read)(TDevice *_device, TSensorVector3Float *_data);
    TDRVStatus              (*Update)(TDevice *_device);
    TDRVStatus              (*Listen)(TDevice *_device, TSensorHubListener *_listener);
    TDRVStatus              (*Unlisten)(TDevice *_device, TSensorHubListener *_listener);

    TDRVStatus              (*I2CBind)(TDevice *_device, TDevice *_bus, uint8_t address);
}TAK8963InterfaceType;

#define TAK8963_API(_device)    TDRV_API(TAK8963InterfaceType, _device)

extern TAK8963InterfaceType AK8963Interface;



