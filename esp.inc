        ;DEFINE ESP_STATE_MACHINE_INPUT_ECHO_DEBUG 1
        ;DEFINE STATE_MACHINE_INT_DEBUG 1        
        ;DEFINE CLIENT_READ_DEBUG 1
        ;DEFINE STREAM_BUFFER_DEBUG 1
        DEFINE SET8KBANK_DEBUG 1
        ;DEFINE WRITEAT_DEBUG 1
        ;DEFINE WRITEAT_COPY_DEBUG 1
        ;DEFINE ZEROES_DEBUG 1

esp_test_at_startup:
        ld hl, at_test_at_startup_cmd
        call esp_command
        ret

esp_restart:
        ld hl, at_restart_cmd
        call esp_command
        ret

esp_echo_off:
        ld hl, at_echo_off_cmd
        call esp_command
        ret

esp_queue_uart_config:
        ld hl, at_uart_config_cmd
        call strlen
        call uart_send_bytes_echo
        ret

esp_uart_config:
        ld hl, at_uart_config_cmd
        call esp_command
        ret

esp_check_version_information:
        ld hl, at_check_version_information_cmd
        call strlen ; bc = length of the asciiz in hl
        call uart_send_bytes_echo
        ret c
        ld hl, temporary_buffer
        ld bc, temporary_buffer_end - temporary_buffer
        call uart_recv_line
        ld hl, temporary_buffer
        ld bc, temporary_buffer_end - temporary_buffer
        call uart_recv_line
        ld hl, temporary_buffer
        ld bc, temporary_buffer_end - temporary_buffer
        call uart_recv_line
        call uart_recv_response
        ret

esp_uart_config_query:
        ld hl, at_uart_config_query_cmd
        call strlen
        call uart_send_bytes_echo
        ret c
        ld hl, temporary_buffer
        ld bc, temporary_buffer_end - temporary_buffer
        call uart_recv_line
        call uart_recv_response
        ret

esp_queue_ip_settings:
        ld hl, at_get_ip_settings_cmd
        call strlen
        call uart_send_bytes_echo
        ret

esp_queue_multiple_connection_mode:
        ld hl, at_cipmux_1_cmd
        call strlen
        call uart_send_bytes_echo
        ret       

esp_queue_create_tcp_server:
        ld hl, at_cipserver_1_333
        call strlen
        call uart_send_bytes_echo
        ret

esp_get_connection_status:
        ld hl, at_get_connection_status_cmd
        call strlen
        call uart_send_bytes_echo
        ret c
        ld hl, temporary_buffer
        ld bc, temporary_buffer_end - temporary_buffer
        call uart_recv_line
        call uart_recv_response
        ret

esp_tcp_start:
        ld hl, at_tcp_start_cmd
        call esp_command
        ret

esp_tcp_send:
        ld hl, at_tcp_send_cmd
        call strlen
        call uart_send_bytes_echo
        ret c
.loop:
        ld hl, temporary_buffer
        ld bc, temporary_buffer_end - temporary_buffer
        call uart_recv_line
        jr nc, .loop
        ret

esp_tcp_close:
        ld hl, at_tcp_close_cmd
        call esp_command
        ret

esp_cwlap:
        ld hl, at_cwlap_cmd:
        call strlen
        call uart_send_bytes_echo
        ret

        MACRO at_command command
        db 'A','T', command, 13, 10, 0
        ENDM

esp_cwjap:
        push de ; password
        push hl ; ssid
        ld hl, at_cwjap_text_start
        call strlen
        call uart_send_bytes_echo
        pop hl ; ssid
        call strlen
        call uart_send_bytes_echo
        ld hl, at_cwjap_text_sep
        call strlen
        call uart_send_bytes_echo
        pop hl ; password
        call strlen
        call uart_send_bytes_echo
        ld hl, at_cwjap_text_end
        call strlen
        call uart_send_bytes_echo
        ret        

at_test_at_startup_cmd:
        db 'AT', 13, 10, 0

at_restart_cmd:
        at_command "+RST"

at_check_version_information_cmd:
        at_command "+GMR"

at_echo_off_cmd:
        at_command "E0"

at_uart_config_query_cmd:
        at_command "+UART_CUR?"

at_uart_config_cmd:
        at_command "+UART=115200,8,1,0,0"

at_get_ip_settings_cmd:
        at_command "+CIFSR"

at_get_connection_status_cmd:
        at_command "+CIPSTATUS"

at_cipmux_1_cmd:
        at_command "+CIPMUX=1"

at_cipserver_1_333:
        at_command "+CIPSERVER=1,1023"

at_tcp_start_cmd:
        at_command '+CIPSTART="TCP","www.hurrikhan.eu",80'

at_tcp_send_cmd
        db "AT+CIPSEND=18\r\nGET / HTTP/1.0\r\n\r\n", 0

at_tcp_close_cmd:
        at_command "+CIPCLOSE"

at_cwlap_cmd:
        at_command "+CWLAP"

at_cwjap_text_start:
        db 'AT+CWJAP="', 0
at_cwjap_text_sep:
        db '","', 0
at_cwjap_text_end:
        db '"\n', 0

esp_command:
        call strlen ; bc = length of the asciiz in hl
        call uart_send_bytes_echo
        ret c
        ld hl, temporary_buffer
        ld bc, temporary_buffer_end - temporary_buffer
        call uart_recv_response
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; There are way too many things coming from the ESP chip
;;
;; The only reasonable way to handle this is with a state machine
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ESP_STATE_MACHINE_CHAR                  EQU 0
ESP_STATE_MACHINE_EXPECTED              EQU 1
ESP_STATE_MACHINE_STRING                EQU 2
ESP_STATE_MACHINE_INT                   EQU 3
ESP_STATE_MACHINE_CALL                  EQU 4
ESP_STATE_MACHINE_SET_VALUE             EQU 5
ESP_STATE_MACHINE_END                   EQU 6
ESP_STATE_MACHINE_ERROR                 EQU 7
ESP_STATE_MACHINE_CRLF                  EQU 8
ESP_STATE_MACHINE_SKIP_UNTIL            EQU 9
ESP_STATE_MACHINE_BREAKPOINT            EQU 10 ; to debug the state machine

esp_state_machine_has_char  db 0
esp_state_machine_char      db 0
esp_state_machine_echoes    db 1

esp_state_machine_char_read:
        ld a, (esp_state_machine_has_char)
        or a
        jp nz, .return_char
        push bc
        push de
        push hl
        IFDEF ESP_STATE_MACHINE_INPUT_ECHO_DEBUG
        ld a, (esp_state_machine_echoes)
        or a
        jp nz, .echoes; echoes
        ENDIF
        call uart_recv_byte_noecho
        IFDEF ESP_STATE_MACHINE_INPUT_ECHO_DEBUG
        jp .got_char
.echoes:
        call uart_recv_byte
.got_char:
        ENDIF
        pop hl
        pop de
        pop bc
        ret c
        ld (esp_state_machine_char), a
        ld a, 1
        ld (esp_state_machine_has_char), a
        ld a, (esp_state_machine_char)
        ret
.return_char:
        ld a, (esp_state_machine_char)
        ret

        MACRO esp_state_machine_char_get
        call esp_state_machine_char_read
        ENDM

        MACRO esp_state_machine_char_clear
        push af
        xor a
        ld (esp_state_machine_has_char), a
        pop af
        ENDM

esp_queue_command_and_wait:
        call strlen
        call uart_send_bytes_echo
.wait:
        call esp_state_machine_execute
        jr c, .error
        call esp_state_is_ok
        jr z, .ok
.error:
        call esp_state_is_error
        jp z, .failure
        ld hl, timeout_text
        call puts
        jp .wait
.failure:
        scf
        ret
.ok:    
        scf
        ccf
        ret

esp_state_is_ok:
        ld a, (esp_state_ok)
        cp 1
        ret

esp_state_is_error:
        ld a, (esp_state_error)
        cp 1
        ret

esp_state_machine_execute:
        xor a
        ld (esp_state_ok), a
        ld (esp_state_error), a
        ld a, 1
        ld (esp_state_machine_echoes), a

.reset_command:
        ld hl, esp_state_machine
.next_command:
        ei
        ld a, (hl)
        inc hl
        add a
        ld ix, .esp_state_machine_jumps
        add_ix_a
        ld a, (ix)
        ld (.esp_jump_lo), a
        ld a, (ix + 1)
        ld (.esp_jump_hi), a
        db $c3
.esp_jump_lo db 0
.esp_jump_hi db 0
        ; jump esp_state_machine_jumps[a * 2]
.esp_state_machine_jumps:
        dw .esp_state_machine_execute_char      ; 0
        dw .esp_state_machine_execute_expected
        dw .esp_state_machine_execute_string
        dw .esp_state_machine_execute_int
        dw .esp_state_machine_execute_call
        dw .esp_state_machine_execute_set_value
        dw .esp_state_machine_execute_end
        dw .esp_state_machine_execute_error
        dw .esp_state_machine_execute_crlf      ; 8
        dw .esp_state_machine_execute_skip_until
        dw .esp_state_machine_execute_breakpoint

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handle ESP_STATE_MACHINE_CHAR command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.esp_state_machine_execute_char:
        esp_state_machine_char_get
        ret c
        ; a = next char
        cp (hl)
        inc hl ; no flags altered
        jp z, .esp_state_machine_execute_char_matched
        ld c, (hl)
        inc hl
        ld b, (hl)
        push bc
        pop hl
        jp .next_command
.esp_state_machine_execute_char_matched:
        esp_state_machine_char_clear
        inc hl
        inc hl
        jp .next_command

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handle ESP_STATE_MACHINE_EXPECTED command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.esp_state_machine_execute_expected:
        ld c, (hl)
        inc hl
.esp_state_machine_execute_expected_loop:
        esp_state_machine_char_get
        ret c
        cp (hl)
        inc hl
        jp nz, .esp_state_machine_execute_error
        esp_state_machine_char_clear

        dec c
        jp nz, .esp_state_machine_execute_expected_loop
        jp .next_command

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handle ESP_STATE_MACHINE_STRING command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.esp_state_machine_execute_string:
        esp_state_machine_char_get
        ret c
        esp_state_machine_char_clear
        cp '"'
        jp nz, .esp_state_machine_execute_error

        ld c, (hl)
        inc hl
        ld b, (hl)
        inc hl
        push hl
        push bc
        pop hl

        ld c, 63
.esp_state_machine_execute_string_loop:
        esp_state_machine_char_get
        jr c, .esp_state_machine_execute_string_timeout
        esp_state_machine_char_clear
        cp '"'
        jr z, .esp_state_machine_execute_string_end:
        ld (hl), a
        inc hl
        dec c
        jr nz, .esp_state_machine_execute_string_loop

.esp_state_machine_execute_string_end:
        xor a
        ld (hl), a
        pop hl
        jp .next_command
.esp_state_machine_execute_string_timeout:
        pop hl
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handle ESP_STATE_MACHINE_INT command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.esp_state_machine_execute_int:
        push hl
        ld a, 10
        push af
.esp_state_machine_execute_int_read_loop:        
        esp_state_machine_char_get
        esp_state_machine_char_clear
        jr c, .esp_state_machine_execute_int_read_loop_timeout
        cp '0'
        jr c, .esp_state_machine_char_get_not_a_digit
        cp '9' + 1
        jr nc, .esp_state_machine_char_get_not_a_digit
        sub '0'
        push af
        jr .esp_state_machine_execute_int_read_loop
.esp_state_machine_execute_int_read_loop_timeout:
        
.esp_state_machine_char_get_not_a_digit
        ;;
        pop af
        cp 10
        jr z, .esp_state_machine_execute_int_end_empty
        ld h, 0
        ld l, a ; x 1
        pop af
        cp 10
        jr z, .esp_state_machine_execute_int_end
        or a
        jr z, .esp_state_machine_execute_int_10_zero
        ld b, 0
        ld c, a
        ld de, 10
.esp_state_machine_execute_int_10:
        add hl, de ; x 10
        dec c
        jr nz, .esp_state_machine_execute_int_10
.esp_state_machine_execute_int_10_zero:
        ;;
        pop af
        cp 10
        jr z, .esp_state_machine_execute_int_end
        or a
        jr z, .esp_state_machine_execute_int_100_zero
        ld c, a
        ld de, 100
.esp_state_machine_execute_int_100:
        add hl, de ; x 100
        dec c
        jr nz, .esp_state_machine_execute_int_100
.esp_state_machine_execute_int_100_zero:
        ;;
        pop af
        cp 10
        jr z, .esp_state_machine_execute_int_end
        or a
        jr z, .esp_state_machine_execute_int_1000_zero
        ld c, a
        ld de, 1000
.esp_state_machine_execute_int_1000:
        add hl, de ; x 1000
        dec c
        jr nz, .esp_state_machine_execute_int_1000
.esp_state_machine_execute_int_1000_zero:
        ;;
        pop af
        cp 10
        jr z, .esp_state_machine_execute_int_end
        or a
        jr z, .esp_state_machine_execute_int_10000_zero
        ld c, a
        ld de, 10000
.esp_state_machine_execute_int_10000:
        add hl, de ; x 10000
        dec c
        jr nz, .esp_state_machine_execute_int_10000
.esp_state_machine_execute_int_10000_zero:
.esp_state_machine_execute_int_end:

        ; hl is the value to write
        IFDEF STATE_MACHINE_INT_DEBUG
        push af
        push bc
        push de
        push hl        
        ld a, '['
        call putc
        call print_hex_hl
        ld a, ']'
        call putc
        pop hl
        pop de
        pop bc
        pop af
        ENDIF

        push hl
        pop bc
        pop hl ; addess of the address
        ld e, (hl)
        inc hl
        ld d, (hl)
        inc hl
        push hl
        push de
        pop hl
        ld (hl), c
        inc hl
        ld (hl), b
        pop hl
        jp .next_command
.esp_state_machine_execute_int_end_empty
        pop hl
        jr .esp_state_machine_execute_error

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handle ESP_STATE_MACHINE_CALL command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.esp_state_machine_execute_call:
        ld a, (hl)
        inc hl
        ld (.esp_state_machine_execute_call_address), a
        ld a, (hl)
        inc hl
        ld (.esp_state_machine_execute_call_address + 1), a
        push hl
        xor a
        ld (esp_state_machine_echoes), a
        db $cd ; call opcode
.esp_state_machine_execute_call_address db 0, 0
        ld a, 1
        ld (esp_state_machine_echoes), a
        pop hl
        jp .next_command

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handle ESP_STATE_MACHINE_SET_VALUE command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.esp_state_machine_execute_set_value:
        ld e, (hl)
        inc hl
        ld d, (hl)
        inc hl
        ld a, (hl)
        inc hl
        ld (de), a
        jp .next_command

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handle ESP_STATE_MACHINE_END command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.esp_state_machine_execute_end:
        scf
        ccf
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handle ESP_STATE_MACHINE_ERROR command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.esp_state_machine_execute_error:
        ; flush until CRLF
.esp_state_machine_execute_error_cr:
        esp_state_machine_char_get
        ret c
        esp_state_machine_char_clear
        cp 13
        jr nz, .esp_state_machine_execute_error_cr
.esp_state_machine_execute_error_lf:
        esp_state_machine_char_get
        ret c
        esp_state_machine_char_clear
        cp 10
        jr nz, .esp_state_machine_execute_error_cr
        scf
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handle ESP_STATE_MACHINE_CRLF command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.esp_state_machine_execute_crlf:
        esp_state_machine_char_get
        ret c
        cp 13
        jr nz, .esp_state_machine_execute_error
        esp_state_machine_char_clear
        esp_state_machine_char_get
        ret c
        cp 10
        jr nz, .esp_state_machine_execute_error
        esp_state_machine_char_clear
        jp .next_command

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handle ESP_STATE_MACHINE_SKIP_UNTIL command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.esp_state_machine_execute_skip_until:
        esp_state_machine_char_get
        ret c
        esp_state_machine_char_clear
        ; a = next char
        cp (hl)
        jp nz, .esp_state_machine_execute_skip_until
        inc hl
        jp .next_command


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handle ESP_STATE_MACHINE_BREAKPOINT command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.esp_state_machine_execute_breakpoint
        break
        jp .next_command

        include "esp-state-machine.inc"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Executes the current command
;;
;; By default, wait for a command, then ...
;;
;; Ping = 0,       // -> byte
;; GetBanks = 1,   // -> page[0..8]
;; Set8KBank = 2,  // page, bank
;; WriteAt = 3,    // offset, len10, bytes
;; CallTo = 4,     // offset
;;
;; I could probably have made-up a state machine for this too but the original
;; set of commands was pretty small.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

client_execute_command:
; while there are bytes to read, call the current command

        IFDEF CLIENT_EXECUTE_BOUNDS_DEBUG
        ld hl, client_execute_command_text_begin
        call puts
        ENDIF
        
client_execute_command_loop:
        ld hl, (esp_recv_count)
        
        IFDEF STREAM_BUFFER_DEBUG
        push hl
        ld a, '('
        call putc
        call print_hex_hl
        ld a, ')'
        call putc
        pop hl
        ENDIF

        ld a, h
        or l
        IFDEF CLIENT_EXECUTE_BOUNDS_DEBUG
        jr z, client_execute_command_call_end
        ELSE
        ret z
        ENDIF
        db $cd ; call opcode
client_execute_command_call_address dw client_read_command
        jr client_execute_command_loop

        IFDEF CLIENT_EXECUTE_BOUNDS_DEBUG
client_execute_command_call_end:
        ld hl, client_execute_command_text_end
        call puts
        ret       

client_execute_command_text_begin db "<EX>", 0
client_execute_command_text_end db "<ex>", 0
        ENDIF

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; resets the state to say that next byte is a command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

client_read_command_reset:
        ld hl, client_read_command
        ld (client_execute_command_call_address), hl
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; closes the client connection
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

client_close_connection_msg:
        at_command "+CIPCLOSE=0"

client_close_connection:
        ld hl, client_close_connection_msg
        call strlen ; bc = length of the asciiz in hl
        call uart_send_bytes_echo
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; closes the connection, flushes input and wait for another command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

client_disconnect_return_error:
        ld hl, .error_text
        call puts

        call client_close_connection
.loop:  ; flush input
        call client_fetch_byte
        jr nc, .loop
        call client_read_command_reset
        scf
        ret

.error_text db "client_disconnect_return_error: protocol failure\n", 0

;; the  tables of supported commands

client_read_command_table:
        dw client_command_ping
        dw client_command_getbanks
        dw client_command_set8kbank
        dw client_command_writeat
        dw client_command_call
        dw client_command_echo
        dw client_command_zeroes

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; fetch a byte, uses the esp_recv_count from the state machine
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

client_fetch_byte:
        push hl
        ld hl, (esp_recv_count)
        ld a, h
        or l
        jr z, .error_scf ; no bytes
        ; fetch the byte
        dec hl
        ld (esp_recv_count), hl
        esp_state_machine_char_get
        jr c, .error
        esp_state_machine_char_clear
        pop hl
        ret
.error_scf:
        scf
.error:
        pop hl
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; reads the next command
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

client_read_command:
        call client_fetch_byte
        IFDEF CLIENT_READ_DEBUG
        push af
        push af
        ld a, '['
        call putc
        pop af
        call print_hex_a
        ld a, ']'
        call putc
        pop af
        ENDIF
        cp 7
        jr nc, .error ; not a supported code
        add a
        ld hl, client_read_command_table
        add hl, a
        ld bc, (hl)
        ld hl, bc
        ld (client_execute_command_call_address), hl
        jp (hl)
.error:
        call print_hex_a
        ld hl, .error_text
        call puts
        call client_disconnect_return_error
        ret

.error_text db ": command error\n", 0
        

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; ping
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

client_command_ping:
        call client_read_command_reset ; no parameter, easy reset

        ld hl, .send_cmd
        ld bc, .send_cmd_end - .send_cmd
        call uart_send_bytes: ; hl, bc
.wait_send:
        call uart_recv_byte
        jr c, .wait_send
        call putc
        cp '>'
        jr nz, .wait_send
        ; send 1 bytes
        ld a, (.ping_value)
        inc a
        ld (.ping_value), a
        call uart_send_byte
        jr c, .error
        ret ; the "SEND OK" will be handled by the next state machine call
.error:
        ld hl, .error_text
        call puts
        call client_disconnect_return_error
        ret

.send_cmd          db "AT+CIPSEND=0,1\r\n"
.send_cmd_end:
.error_text        db "ping failed\n", 0
.ping_value        db $ff

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; sends back the current list of banks assigned to pages 0 to 7
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

client_command_getbanks:
        call client_read_command_reset ; no parameter, easy reset

        ld hl, .send_cmd
        ld bc, .send_cmd_end - .send_cmd
        call uart_send_bytes: ; hl, bc
.wait_send:
        call uart_recv_byte
        jr c, .wait_send
        cp '>'
        jr nz, .wait_send
        ; send 8 bytes
        ld a, $50
.nextbank:
        push af
        call nextreg_get
        call uart_send_byte
        pop af
        inc a
        cp $58
        jr nz, .nextbank
        ret ; the "SEND OK" will be handled by the next state machine call
.error:
        ld hl, .error_text
        call puts
        call client_disconnect_return_error
        ret

.send_cmd       db "AT+CIPSEND=0,8\r\n"
.send_cmd_end:
.error_text     db "getbank failed\n", 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; sets a 8k bank to a page
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

client_command_set8kbank:
        ; read the bank
        ; read the page
        ld a, (.header_len)
        ld hl, .header
        add hl, a
        inc a
        ld (.header_len), a
        call client_fetch_byte
        jr c, .error
        ld (hl), a
        ld a, (.header_len)
        cp 2
        ret nz

        ld bc, $243b
        ld a, (.bank)
        and $07
        add $50
        out (c), a
        inc b
        ld a, (.page)
        out (c), a


        IFDEF SET8KBANK_DEBUG
; debug begin

        ld hl, .debug_text
        call puts
        ld a, (.bank)
        call print_hex_a
        ld a, ' '
        call putc
        ld a, (.page)
        call print_hex_a
        ld a, 10
        call putc

; debug end
        ENDIF

        xor a
        ld (.header_len), a  
        call client_read_command_reset
        ret
.error:
        xor a
        ld (.header_len), a
        ld hl, .error_text
        call puts
        call client_disconnect_return_error
        ret

.header:
.bank           db 0
.page           db 0
.header_len     db 0
.error_text     db "set8kbank error\n",0
.debug_text     db "set8kbank ", 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; copy bytes from the network to a specific address in the memory
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; part 1: read the header

client_command_writeat:
        ; offset, len, bytes
        ld a, (.header_len)
        ld hl, .header
        add hl, a
        inc a
        ld (.header_len), a
        call client_fetch_byte
        jr c, .error
        ld (hl), a
        ld a, (.header_len)
        cp 4
        ret nz


        IFDEF WRITEAT_DEBUG
; debug begin

        ld hl, .debug_text
        call puts
        
        ld hl, (.offset)
        call print_hex_hl
        ld a, ' '
        call putc
        ld hl, (.length)
        call print_hex_hl
        ld a, 10
        call putc

        ENDIF

; debug end
        
        xor a
        ld (.header_len), a

        ld hl, .read_and_copy_the_bytes
        ld (client_execute_command_call_address), hl
        ret

; part 2: copy the bytes

.read_and_copy_the_bytes:
        IFDEF WRITEAT_COPY_DEBUG
        ld a, '{'
        call putc
        ld hl, (.length)
        call print_hex_hl
        ld a, ','
        call putc
        ld hl, (.offset)
        call print_hex_hl
        ld a, '}'
        call putc
        ENDIF

        ld bc, (.length)
        ld hl, (.offset)
        call client_fetch_byte
        jr c, .error
        

        IFDEF WRITEAT_COPY_DEBUG
        push af
        push af
        ld a, '='
        call putc
        pop af
        call print_hex_a
        ld a, 10
        call putc
        pop af
        ENDIF

        ld (hl), a
        inc hl
        ld (.offset), hl
        dec bc
        ld (.length), bc
        ld a, c
        or b
        ret nz

        call client_read_command_reset
        ret

.error:
        ; reset
        xor a
        ld (.header_len), a

        ld hl, .error_text
        call puts
        call client_disconnect_return_error
        ret

.header:
.offset         dw 0
.length         dw 0
.header_len     db 0
.error_text     db "writeat error\n", 0
.debug_text     db "writeat ", 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; write zeroes to a specific address in the memory
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; part 1: read the header

client_command_zeroes:
        ; offset, len, bytes
        ld a, (.header_len)
        ld hl, .header
        add hl, a
        inc a
        ld (.header_len), a
        call client_fetch_byte
        jr c, .error
        ld (hl), a
        ld a, (.header_len)
        cp 4
        ret nz

        IFDEF ZEROES_DEBUG
; debug begin

        ld hl, .debug_text
        call puts
        
        ld hl, (.offset)
        call print_hex_hl
        ld a, ' '
        call putc
        ld hl, (.length)
        call print_hex_hl
        ld a, 10
        call putc

        ENDIF

; debug end
        
        ld hl, (.offset)
        ld bc, (.length)
        xor a
        ld (.header_len), a
.loop:
        xor a
        ld (hl), a
        inc hl
        dec bc        
        ld a, b
        or c
        jr nz, .loop

        call client_read_command_reset
        ret

.error:
        ; reset
        xor a
        ld (.header_len), a

        ld hl, .error_text
        call puts
        call client_disconnect_return_error
        ret

.header:
.offset         dw 0
.length         dw 0
.header_len     db 0
.error_text     db "zeroes error\n", 0
.debug_text     db "zeroes ", 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; calls an address in memory
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

client_command_call:
        ; offset
        ld a, (.header_len)
        ld hl, .call_address
        add hl, a
        inc a
        ld (.header_len), a
        call client_fetch_byte
        jr c, .error
        ld (hl), a
        ld a, (.header_len)
        cp 2
        ret nz
        call client_read_command_reset
        xor a
        ld (.header_len), a
        db $cd ; call to a 16 bits address
.call_address           dw 0
        ret
.error:
        xor a
        ld (.header_len), a
        ld hl, .error_text
        call puts
        call client_disconnect_return_error
        ret
.header_len     db 0   
.error_text     db "jumpto error\n", 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; prints characters from the stream until a CHR(0) is found.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

client_command_echo:
        call client_fetch_byte
        jr c, .error
        or a
        jr z, .done
        call putc
        ret
.done:
        call client_read_command_reset
        ret
.error:
        ld hl, .error_text
        call puts
        call client_disconnect_return_error
        ret

.error_text     db "echo error\n", 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; variables associated to the state machine
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

esp_state_ok            db 0
esp_state_error         db 0
esp_recv_count          dw 0
connection_current      dw 0
wifi_connected_flag     db 0
wifi_ip_obtained        db 0
wifi_ap_ecn             dw 0
wifi_ip_address
        DUP 40 ; max 39 chars + NUL terminator
        db 0
        EDUP
wifi_mac_address
        DUP 18 ; max 17 chars + NUL terminator
        db 0
        EDUP
wifi_ap_ssid
        DUP 33 ; max 32 chars + NUL terminator
        db 0
        EDUP
