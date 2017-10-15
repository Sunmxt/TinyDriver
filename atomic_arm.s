    export  TDrvExclusiveSwap8
    export  TDrvExclusiveSwap16
    export  TDrvExclusiveSwap32
    export  TDrvCAS8
    export  TDrvCAS16
    export  TDrvCAS32
    export  TDrvExclusiveAdd8
    export  TDrvExclusiveAdd16
    export  TDrvExclusiveAdd32

;
;   8-bit Exclusive Swap
;
;   int8_t TDrvExclusiveSwap8(int8_t *_address, int8_t _swap);
;
    area    |.text|, code, readonly
TDrvExclusiveSwap8  proc
    
__tdrv_es8_begin
        ldrexb  r2, [r0]
        strexb  r3, r1, [r0]
        cbz     r3, __tdrv_es8_end
        b       __tdrv_es8_begin
__tdrv_es8_end
        mov     r0, r2
        bx      lr
    endp

;
;   16-bit Exclusive Swap
;
;   int16_t TDrvExclusiveSwap16(int16_t *_address, int16_t _swap);
;
TDrvExclusiveSwap16  proc

__tdrv_es16_begin
        ldrexh  r2, [r0]
        strexh  r3, r1, [r0]
        cbz     r3, __tdrv_es16_end
        b       __tdrv_es16_begin
__tdrv_es16_end
        mov     r0, r2
        bx      lr
    endp

;
;   32-bit Exclusive Swap
;
;   int32_t TDrvExclusiveSwap32(int32_t *_address, int32_t _swap);
;
TDrvExclusiveSwap32  proc

__tdrv_es32_begin
        ldrex   r2, [r0]
        strex   r3, r1, [r0]
        cbz     r3, __tdrv_es32_end
        b       __tdrv_es32_begin
__tdrv_es32_end
        mov     r0, r2
        bx      lr
    endp

;
;   8-bit Compare and Exchange
;
;   int8_t TDrvCAS8(int8_t *_address, int8_t _compare, int8_t _exchange);
;
TDrvCAS8    proc

__tdrv_c8_begin
        ldrexb  r3, [r0]
        eor     r1, r3
        cbnz    r1, __tdrv_c8_ret
        strexb  r1, r2, [r0]
        cbz     r1, __tdrv_c8_ret
        b       __tdrv_c8_begin
__tdrv_c8_ret
        mov     r0, r3
        bx      lr
    endp

;
;   16-bit Compare and Exchange
;
;   int16_t TDrvCAS16(int16_t *_address, int16_t _compare, int16_t _exchange);
;
TDrvCAS16    proc

__tdrv_c16_begin
        ldrexh  r3, [r0]
        eor     r1, r3
        cbnz    r1, __tdrv_c16_ret
        strexh  r1, r2, [r0]
        cbz     r1, __tdrv_c16_ret
        b       __tdrv_c16_begin
__tdrv_c16_ret
        mov     r0, r3
        bx      lr
    endp

;
;   32-bit Compare and Exchange
;
;   int32_t TDrvCAS32(int32_t *_address, int32_t _compare, int32_t _exchange);
;
TDrvCAS32   proc

__tdrv_c32_begin
        ldrex   r3, [r0]
        eor     r1, r3
        cbnz    r1, __tdrv_c32_ret
        strex   r1, r2, [r0]
        cbz     r1, __tdrv_c32_ret
        b       __tdrv_c32_begin
__tdrv_c32_ret
        mov     r0, r3
        bx      lr
    endp

;
;   8-bit Exclusive Add
;
;   int8_t TDrvExclusiveAdd8(int8_t *_address, int8_t _value);
;
TDrvExclusiveAdd8  proc

__tdrv_ea8_begin
        ldrexb  r2, [r0]
        add     r2, r2, r1
        strexb  r3, r2, [r0]
        cbz     r3, __tdrv_ea8_ok
        b       __tdrv_ea8_begin

__tdrv_ea8_ok
        bx      lr
    endp

;
;   16-bit Exclusive Add
;
;   int16_t TDrvExclusiveAdd16(int16_t *_address, int16_t _value);
;
TDrvExclusiveAdd16  proc

__tdrv_ea16_begin
        ldrexh  r2, [r0]
        add     r2, r2, r1
        strexh  r3, r2, [r0]
        cbz     r3, __tdrv_ea16_ok
        b       __tdrv_ea16_begin

__tdrv_ea16_ok
        bx      lr
    endp

;
;   32-bit Exclusive Add
;
;   int32_t TDrvExclusiveAdd32(int32_t *_address, int32_t _value);
;
TDrvExclusiveAdd32  proc

__tdrv_ea32_begin
        ldrex   r2, [r0]
        add     r2, r2, r1
        strex   r3, r2, [r0]
        cbz     r3, __tdrv_ea32_ok
        b       __tdrv_ea32_begin

__tdrv_ea32_ok
        bx      lr
    endp
        
    END