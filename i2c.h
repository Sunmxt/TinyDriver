/*

    Tiny Driver I2C Driver Framework

    by Sunmxt
*/
#ifndef TINY_DRIVER_I2C
#define TINY_DRIVER_I2C

#include "tiny_driver.h"
#include "klist.h"

typedef struct _Tiny_Driver_I2C_Message
{
    uint8_t attribute;
    /*
        Receive

            Indicate that the message is read from slaves.
    */
    #define TDRV_I2C_MSG_RECEIVE_POS    0
    #define TDRV_I2C_MSG_RECEIVE        (1u << TDRV_I2C_MSG_RECEIVE_POS)
    /*
        Packed message

            A packed message is packed with the predecessor.
            If a predecessor message is not sent successfully, 
        the following packed messages will be canceled.
    */
    #define TDRV_I2C_MSG_PACKED_POS     1
    #define TDRV_I2C_MSG_PACKED         (1u << TDRV_I2C_MSG_PACKED_POS)
    /*
        Restart

            Generate a restart condition before tranfering next 
        packed message.
            If the next message is not packed, it will be ignored.
    */
    #define TDRV_I2C_MSG_RESTART_POS    2
    #define TDRV_I2C_MSG_RESTART        (1u << TDRV_I2C_MSG_RESTART_POS)
    /*
        Message Busy

            Message is ready for tranfering (or not totally transfer yet).
    */
    #define TDRV_I2C_MSG_BUSY_POS       3
    #define TDRV_I2C_MSG_BUSY           (1u << TDRV_I2C_MSG_BUSY_POS)
    /*
        Private Field (driver)
    */
    #define TDRV_I2C_MSG_PRIVATE_FLAGS_POS  4
    #define TDRV_I2C_MSG_PRIVATE_FLAGS  (0xFFu << TDRV_I2C_MSG_PRIVATE_FLAGS_POS)


    Word address;
    /*
        7-bit / 10-bit Address field
    */
    void *data;
    size_t size;

    union {
        void* callback_params;
        void* failed_info;
    };

    void* (*callback)(struct _Tiny_Driver_I2C_Message *msg);

    bi_list_node node;
}TDrvI2CMessage;

#define TDRV_TO_I2C_MSG(_ptr) S_LIST_TO_DATA(_ptr, TDrvI2CMessage, node)
#define TDrvI2CMessageInit(_message, _address, _data, _size) \
    ((_message) -> attribute = 0, (_message) -> address = _address, (_message) -> data = _data, (_message) -> size = _size, (_message) -> node.next = 0, (_message) -> node.prev = 0, 0)


typedef enum _I2C_Address_Mode { I2C_7BIT_ADDR, I2C_10BIT_ADDR} I2CAddressMode;
typedef enum _I2C_Speed {I2C_FAST_400KBPS, I2C_NORMAL_100KBPS} I2CSpeed;

typedef TDRVStatus (*TDrvI2CConfig)(TDevice *_device, I2CAddressMode _addr_mode, I2CSpeed _speed);
typedef TDRVStatus (*TDrvI2CSendMessageAsync)(TDevice *_device, TDrvI2CMessage *_message);
typedef TDRVStatus (*TDrvI2CReceiveMessageAsync)(TDevice *_device, TDrvI2CMessage *_message);

typedef struct _Tiny_Driver_I2C_Interface
{
    TDrvHAInit init;
    TDrvHADeinit deinit;
    TDrvI2CSendMessageAsync SendAsync;
    TDrvI2CReceiveMessageAsync ReceiveAsync;
    TDrvI2CConfig Config;
}TDrvI2CInterface;

extern TDrvI2CInterface I2CInterface;

#define TDRV_I2C_API(_index) TDRV_API(TDrvI2CInterface, _index)

#endif
