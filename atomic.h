#ifndef TINY_DRIVER_ATOMIC
#define TINY_DRIVER_ATOMIC

#include "stdint.h"

uint16_t     TDrvExclusiveSwap16(volatile uint16_t* _address, uint16_t _value);
uint32_t     TDrvExclusiveSwap32(volatile uint32_t* _address, uint32_t _swap);
uint8_t      TDrvExclusiveSwap8(volatile uint8_t* _address, uint8_t _swap);
uint32_t    TDrvCAS32(volatile uint32_t* _address, uint32_t _compare, uint32_t _exchange);
uint16_t    TDrvCAS16(volatile uint16_t* _address, uint16_t _compare, uint16_t _exchange);
uint8_t     TDrvCAS8(volatile uint8_t* _address, uint8_t _compare, uint8_t _exchange);
int8_t      TDrvExclsuveAdd8(volatile int8_t* _address, int8_t _value);
int16_t     TDrvExclsuveAdd16(volatile int16_t* _address, int16_t _value);
int32_t     TDrvExclsuveAdd32(volatile int32_t* _address, int32_t _value);



/* 
    If complie error raise here, it means:
        No compatiable CAS operation to support CAS of pointer type
*/
typedef char ___tiny_driver_ptr_length_assert[1 - 2*!!(
        (sizeof(void*) != 1) && (sizeof(void*) != 2) && (sizeof(void*) != 4)
    )];

#define TDrvCASPointer(_pointer, _compare, _exchange) \
    (\
        (sizeof(void*) == 1) ? (TDrvCAS8((uint8_t*)(_pointer), (uint8_t)(uint32_t)(_compare), (uint8_t)(uint32_t)(_exchange))) : (\
        (sizeof(void*) == 2) ? (TDrvCAS16((uint16_t*)(_pointer), (uint16_t)(uint32_t)(_compare), (uint16_t)(uint32_t)(_exchange))) : (\
        (sizeof(void*) == 4) ? (TDrvCAS32((uint32_t*)(_pointer), (uint32_t)(uint32_t)(_compare), (uint32_t)(uint32_t)(_exchange))) : (\
        (0)\
        )))\
    )

#endif
