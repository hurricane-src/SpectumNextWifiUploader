/*
0x133B UART Tx

(R)

bit 7 = 1 if the Rx is in a break condition  ; external device has held Tx=0 for at least 20 bit periods
bit 6 = 1 if the Rx experienced a framing error  ; (clears on read, includes parity and stop bit errors)
bit 5 = 1 if the next Rx byte was received after an error condition was detected (framing, overflow)
bit 4 = 1 if the Tx buffer is empty
bit 3 = 1 if the Rx buffer is near full (3/4)
bit 2 = 1 if the Rx buffer overflowed  ; (clears on read)
bit 1 = 1 if the Tx buffer is full
bit 0 = 1 if the Rx buffer contains bytes
(W)

Send a byte to the connected device.
*/
UART_TX_PORT EQU $133b

/*
Read a byte from the receive buffer. If the buffer is empty, 0 is returned.

Writes the lower 14-bits of the UART's prescalar value that determines baud rate

If bit 7 = 1
        bits 6:0 = upper 7-bits of the 14-bit prescalar value
If bit 7 = 0
        bits 6:0 = lower 7-bits of the 14-bit prescalar value
The UART's baud rate is determined by the prescalar according to this formula:

prescalar = Fsys / baudrate ; Fsys = system clock from Video Timing Register ($11)
Eg: If the system is HDMI, nextreg 0x11 indicates that Fsys = 27000000. The prescalar for a baud rate of 115200 is 27000000 / 115200 = 234.
*/
UART_RX_PORT EQU $143b

/*
bit 6 = 0 to select the esp uart, 1 to select the pi uart †
bit 4 = 1 if the bits 2:0 are being written
bits 2:0 = most significant bits of the 17-bit prescalar setting baud rate
           pi gpio must be configured for uart, see Pi Peripheral Enable Register ($A0)
*/
UART_CTRL_PORT EQU $153b

/*
0x163B UART Frame (R/W) (hard reset = 0x18)
 bit 7 = 1 to immediately reset the Tx and Rx modules to idle and empty fifos
 bit 6 = 1 to assert break on Tx (Tx = 0) when Tx reaches idle
 bit 5 = 1 to enable hardware flow control *
 bits 4:3 = number of bits in a frame
   11 = 8 bits
   10 = 7 bits
   01 = 6 bits
   00 = 5 bits
 bit 2 = 1 to enable parity check
 bit 1 = 0 for even parity, 1 for odd parity
 bit 0 = 0 for one stop bit, 1 for two stop bits
*/
UART_FRAME_PORT EQU $163b

uart_init:
        ;nextreg NEXT_RESET_REGISTER, 128
        ;call wait_500ms
        ;nextreg NEXT_RESET_REGISTER, 0
        ;call wait_500ms

        ld a, VIDEO_TIMING_REGISTER
        call nextreg_get
        and $07 ; 07

        call print_hex_a
        push af
        ld a, 10        
        call putc
        pop af

        rlca
        ld hl, _115200_prescalars
        add_hl_a
        ld bc, UART_RX_PORT
        ld a, (hl)
        out (c), a
        inc hl
        ld a, (hl)
        out (c), a

        ld bc, UART_CTRL_PORT
        in a, (c) ; 00
        
        call print_hex_a
        push af
        ld a, 10        
        call putc
        pop af

        ;and %10111000 ; ESP, prescalar msb = 00
        ;or  %00010000 ; setting prescalar msb
        ld a, %00010000
        out (c), a

        ld bc, UART_FRAME_PORT
        in a, (c) ; 18
        
        call print_hex_a
        push af
        ld a, 10        
        call putc
        pop af

        ;ld a, %10011000 ; reset, 8 bits no parity 1 stop bit
        ;out (c), a

        ret

uart_send_bytes_echo:
        push af
        push bc
        push de
        push hl
        call puts
        pop hl
        pop de
        pop bc
        pop af
        call uart_send_bytes
        ret

uart_send_bytes: ; hl, bc
.loop:
        ld a, (hl)
        inc hl
        push bc
        call uart_send_byte
        pop bc
        ret c
        dec bc
        ld a, c
        or b
        jp nz, .loop
        ret
        
uart_recv_bytes:
        ld de, hl
        push hl
        add de, bc
.loop:
        call uart_recv_byte
        jp c, .error
        ld (hl), a
        inc hl
        call cmp_equals_hl_de
        jp nz, .loop
        pop bc
        sub hl, de
        ld bc, hl
        ccf
        ret
.error: 
        pop bc
        sub hl, bc
        ld bc, hl
        scf
        ret

uart_recv_line:
        ld de, hl
        push hl
        add de, bc
.loop:
        call uart_recv_byte
        jp c, .error
        ld (hl), a
        inc hl
        cp 10
        jp z, .end
        call cmp_equals_hl_de
        jp nz, .loop
        pop bc
        sub hl, de
        ld bc, hl
        ccf
.end:
        pop bc
        sub hl, bc
        ld bc, hl
        scf
        ccf
        ret
.error: 
        pop bc
        sub hl, bc
        ld bc, hl
        scf
        ret

uart_recv_response:
        call uart_recv_byte
        jp c, .error

        cp 'O'
        jp z, .maybe_OK
        cp 'E'
        jp z, .maybe_ERROR
        cp 'W'
        jp z, .maybe_WIFI_DISCONNECT

        cp 10 ; broken CSpect
        jp z, uart_recv_response

        cp 13 ; broken CSpect
        jp z, uart_recv_response

        scf ; error
.error:
        ret

.maybe_OK:
        ld hl, _response_OK + 1
        ld c, _response_OK_end - _response_OK - 1
        jp uart_recv_expected_bytes
.maybe_ERROR:
        ld hl, _response_ERROR + 1
        ld c, _response_ERROR_end - _response_ERROR - 1
        jp uart_recv_expected_bytes
.maybe_WIFI_DISCONNECT:
        ld hl, _response_WIFI_DISCONNECT + 1
        ld c, _response_WIFI_DISCONNECT_end - _response_WIFI_DISCONNECT - 1
        ;jp uart_recv_expected_bytes
        push uart_recv_response

uart_recv_expected_bytes: ; hl, c
.loop:  push bc
        call uart_recv_byte
        jp c, .error
        cp (hl)
        jp nz, .no_match
        inc hl
        pop bc
        dec c
        jp nz, .loop
.error:
        ret
.no_match:
        pop bc
        ccf
        scf
        ret
        
uart_send_wait_timeout:
        push de
        push af
        ld de, 0
        ld bc, UART_TX_PORT
.wait:  ei
        in a, (c)
        test %00000010       ; TX buffer full
        jr z, .ok
        dec de
        ld a, e
        or d
        jp nz,  .wait
        pop af
        pop de
        scf
        ret
.ok:
        pop af
        pop de
        scf
        ccf
        ret

uart_send_byte:
        call uart_send_wait_timeout
        ret c
        out (c), a
        ret

        ; if success, carry = 0
uart_recv_wait_timeout:
        push de
        ld de, 0
        ld bc, UART_TX_PORT
.wait:  ei
        in a, (c)
;        cp 0
;        jr z, .nodump        
;        call print_hex_a
;.nodump:
        test %00000001       ; RX contains bytes
        jp nz, .ok
        dec de
        ld a, e
        or d
        jp nz, .wait
        scf
.ok:    
        pop de
        ret

        ; if success, carry = 0
uart_recv_byte:
        call uart_recv_wait_timeout
        ret c
        ld bc, UART_RX_PORT
        in a, (c)       
        call putc
        ret

uart_recv_byte_noecho:
        call uart_recv_wait_timeout
        ret c
        ld bc, UART_RX_PORT
        in a, (c)
        ret

_response_OK:
        db "OK\r\n"
_response_OK_end:
_response_ERROR:
        db "ERROR\r\n"
_response_ERROR_end:
_response_WIFI_DISCONNECT:
        db "WIFI DISCONNECT\r\n"
_response_WIFI_DISCONNECT_end:
/*
%000	Base VGA timing, clk28 = 28000000
%001	VGA setting 1,   clk28 = 28571429
%010	VGA setting 2,   clk28 = 29464286
%011	VGA setting 3,   clk28 = 30000000
%100	VGA setting 4,   clk28 = 31000000
%101	VGA setting 5,   clk28 = 32000000
%110	VGA setting 6,   clk28 = 33000000
%111	HDMI,            clk28 = 27000000
*/
_115200_prescalars:
        ; 243 = f3
        db $81, $73
        ; 248 = f8
        db $81, $78
        ; 255 = ff
        db $81, $7f
        ; 260 = 104
        db $82, $04
        ; 269 = 10d
        db $82, $0d
        ; 277 = 115
        db $82, $15
        ; 286 = 11e
        db $82, $1e
        ; 234 = ea
        db $81, $6a
