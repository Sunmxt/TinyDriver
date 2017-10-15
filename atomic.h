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

#endif

