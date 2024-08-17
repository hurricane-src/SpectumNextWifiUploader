;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Spectrum Next test program
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        OPT --zxnext=cspect
        ; Allow the Next paging and instructions
        DEVICE ZXSPECTRUMNEXT
        SLDOPT COMMENT WPMEM, LOGPOINT, ASSERTION

        ; Generate a map file for use with Cspect
        CSPECTMAP "wifiupld.map"

        include "macros.inc"
        include "next_board_feature_control.inc"

KB_EDIT EQU $01 ; CTRL on CSPect
KB_BRK EQU $02  ; ?

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; The program is meant to be smaller than 8KB
;;
;; This is its location in memory.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SERVER_BANK EQU 223

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Entry point
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        ORG $8000
Start:  nextreg $50, SERVER_BANK
        ld sp, $1ffe ; Nothing must write outside of the first 8K page
        jp RealStart

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Bootstrap code. Unused here but injected from the client.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

bootstrap:
        di
        nextreg $50, $ff
        ld sp, $fffe
        jp $8000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; The real code of the program
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

bank_0:
        MMU 0,SERVER_BANK
        ORG $0000
key_buffer:
        DUP 16
        db $00
        EDUP
key_read_offset db $00, $00
key_write_offset db $00, $00
frame_counter db 0, 0, 0
        ; $38 bytes of real estate
        ORG $0038
interrupt_handler:
        ex af,af'
        exx

        ld a, (frame_counter)
        add 1
        ld (frame_counter), a
        ld a, (frame_counter + 1)
        adc 0
        ld (frame_counter + 1), a
        ld a, (frame_counter + 2)
        adc 0
        ld (frame_counter + 2), a

        ld a, $b0 ; Extended Keys 0 Register https://wiki.specnext.dev/Extended_Keys_0_Register
        call nextreg_get
        ld hl, kb_sc_dq_comma_period
        call kb_key8_process

        ld a, $b1 ; Extended Keys 1 Register https://wiki.specnext.dev/Extended_Keys_1_Register
        call nextreg_get
        ld hl, kb_del_ed_brk_invv_truv_graph_cpslck_xtnd
        call kb_key8_process
        
        ld bc, $7ffe
        in a, (c)
        ld hl, kb_bnm_shift_space
        call kb_key5_process

        ld bc, $bffe
        in a, (c)
        ld hl, kb_hjkl_enter
        call kb_key5_process

        ld bc, $dffe
        in a, (c)
        ld hl, kb_yuiop
        call kb_key5_process

        ld bc, $effe
        in a, (c)
        ld hl, kb_67890
        call kb_key5_process

        ld bc, $f7fe
        in a, (c)
        ld hl, kb_54321
        call kb_key5_process

        ld bc, $fbfe
        in a, (c)
        ld hl, kb_trewq
        call kb_key5_process

        ld bc, $fdfe
        in a, (c)
        ld hl, kb_gfdsa
        call kb_key5_process

        ld bc, $fefe
        in a, (c)
        ld hl, kb_vcxz_caps
        call kb_key5_process

        exx
        ex af,af'
        reti
interrupt_handler_end:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handler for 8-bits kind of keyboard IO
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;=

; 1 1 => ignore
; 0 0 => ignore
; 0 1 => ignore
; 1 0 => process

; 1 O => ignore
; 0 I => ignore
; 0 O => ignore
; 1 I => process

kb_key8_process:
        ; look at the bit
        ; see if it was clear
        ; get the char
        ; queue the char

        ld c, a         ; current state
        cpl
        ld b, (hl)      ; prev state
        ld (hl), a      ; save state
        cpl
        or a
        ret z           ; no keys are up
        and b           ; I only care if there are ones
        ; a is the mask of the relevant states
        ;and c           ; keeps only the keys that have been pressed
.loop:
        ld c, a
        inc hl
        test 1
        jp z, .not_pressed
        ld a, (hl)
        call kb_key_write
.not_pressed:
        ld a, c
        sra a           ; since I want to set Z, this is faster
        jp nz, .loop

        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Handler for 5-bits kind of keyboard IO
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;=

; 00011111 => 11100000 => 00000000
; 00000000 => 11111111 => 00011111

kb_key5_process:
        or $e0          ; make it look like something useful
        cpl
        ld c, a         ; current state
        cpl
        ld b, (hl)      ; prev state
        ld (hl), a
        cpl
        or a
        ret z        
        and b           ; I only care if there are ones
        ; a is the mask of the relevant states
        ;and c           ; keeps only the keys that have been pressed
.loop:
        ld c, a
        inc hl
        test 1
        jp z, .not_pressed
        ld a, (hl)
        call kb_key_write
.not_pressed:
        ld a, c
        sra a           ; since I want to set Z, this is faster
        jp nz, .loop

        ret

kb_key_write:
        ld de, (key_write_offset)
        ld (de), a
        push hl
        ld hl, (key_read_offset)
        ld a, e
        inc a
        and $0f
        cp l
        pop hl
        ret z
        ld (key_write_offset), a
        ret

kb_key_read:
        ld hl, (key_read_offset)
        ld a, l
        push hl
        ld hl, (key_write_offset)
        cp l
        jp z, .empty
        inc a
        and $0f
        ld (key_read_offset), a
        pop hl
        ld a, (hl)
        ret ; carry = 0
.empty: pop hl
        scf ; carry = 1
        ret

kb_sc_dq_comma_period db $ff
        db ';', '"', ',', '.', $10, $11, $12, $13

kb_del_ed_brk_invv_truv_graph_cpslck_xtnd db $ff
        db $08, KB_EDIT, KB_BRK, $03, $04, $05, $06, $07

kb_bnm_shift_space db $1f
        db ' ', $01, 'm', 'n', 'b'
        db $08, $01, 'M', 'N', 'B'
kb_hjkl_enter db $1f
        db $0a, 'l', 'k', 'j', 'h'
        db $0a, 'L', 'K', 'J', 'H'
kb_yuiop db $1f
        db 'p', 'o', 'i', 'u', 'y'
        db 'P', 'O', 'I', 'U', 'Y'
kb_67890 db $1f
        db '0', '9', '8', '7', '6'
        db '_   ', ')', '(', "'", '&'
kb_54321 db $1f
        db '1', '2', '3', '4', '5'
        db '!', '@', '#', '$', '%'
kb_trewq db $1f
        db 'q', 'w', 'e', 'r', 't'
        db 'Q', 'W', 'E', 'R', 'T'
kb_gfdsa db $1f
        db 'a', 's', 'd', 'f', 'g'
        db 'A', 'S', 'D', 'F', 'G'
kb_vcxz_caps db $1f
        db $03, 'z', 'x', 'c', 'v'
        db $03, 'Z', 'X', 'C', 'V'

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Real start of the program.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;=

RealStart:
        di
        nextreg CPU_SPEED_REGISTER, 3 ; 28 MHz
        call cls
        ei
        halt
        call uart_init

        call esp_print_input_until_timeout

        call esp_queue_uart_config
        call esp_print_input_until_timeout

        call esp_state_machine_execute        
        ;; flush the input
        
        ;; disable echo
        call esp_echo_off
        ;; flush the input
        call esp_print_input_until_timeout
        ;; test AT command
        call esp_test_at_startup
        call esp_print_input_until_timeout
        ;; print vesrion information
        call esp_check_version_information
        call esp_print_input_until_timeout

        ;; set multiple connection mode

        ld hl, at_get_ip_settings_cmd
        call esp_queue_command_and_wait
        jp c, .failure

        ld hl, at_cipmux_1_cmd
        call esp_queue_command_and_wait
        jp c, .failure

        ld hl, at_cipserver_1_333
        call esp_queue_command_and_wait
        jp c, .failure

        ld hl, at_get_connection_status_cmd
        call esp_queue_command_and_wait
        jp c, .failure
        jp .start_working      

.failure:
        ld hl, failure_text
        call puts

.start_working:

        ;; print all known symbols

        ld a, 0
.debug_print_symbols_loop:
        call putc
        inc a
        cp 10
        jr nz, .debug_print_symbols_notlf
        ld a, 32
        call putc
        ld a, 11
.debug_print_symbols_notlf:
        cp 0
        jr nz, .debug_print_symbols_loop
        ld a, 10
        call putc

        ;; print input menu

        ld hl, .menu_text
        call puts
        
.wait_forever:
        ei
        halt
        call esp_state_machine_execute
        call kb_key_read
        jr c, .wait_forever
        call putc
        ;cp 'w'
        ;jr z, .wifi_setup
        cp 'p'
        jp z, .print_info
        jr .wait_forever

.menu_text:
        ;db "w - Wi-Fi setup\n"
        db "p - Print information\n"
        db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Wi-Fi setup
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.wifi_setup:
        ; print all wifi antenas
        ; chose one
        ; enter password
        ld a, 'A'
        ld (.wifi_setup_next_letter), a
        ld hl, temporary_buffer
        ld (.wifi_ssid_buffer_offset), hl
        ld a, $ff
        ld (wifi_ap_ecn), a             ; so I'll know if a CWLAP line is read
        call esp_cwlap                  ; give the order
.wifi_setup_loop:
        ei
        halt
        call esp_state_machine_execute  ; get the results, one CWLAP result will be read by call
        ld a, (wifi_ap_ecn)
        cp $ff
        jp nz, .wifi_setup_got_an_ap
        call kb_key_read
        jr c, .wifi_setup_loop
        call putc
        cp 'a'
        jr c, .wifi_setup_not_a_letter ; < 'a'
        cp 'z' + 1
        jr c, .wifi_setup_choice
.wifi_setup_not_a_letter:
        cp KB_EDIT                   ; because I can't find BRK on CSPect ....
        jp z, .start_working         ; exit this menu
        cp KB_BRK
        jp nz, .wifi_setup_loop
        jp .start_working            ; exit this menu

.wifi_setup_got_an_ap:
        ld a, $ff
        ld (wifi_ap_ecn), a             ; reset the line "flag"
        ld a, (.wifi_setup_next_letter)
        cp a, 'Z'                       ; too many APs?
        jp z, .wifi_setup_loop
        call putc
        inc a
        ld (.wifi_setup_next_letter), a
        ld hl, .wifi_setup_separator_text
        call puts
        ld hl, wifi_ap_ssid
        call puts
        ld a, 10
        call putc
        ; copy the SSID to the temporary buffer
        ld de, (.wifi_ssid_buffer_offset)
        ld hl, wifi_ap_ssid
.wifi_setup_got_an_ap_ssid_copy_loop:
        ld a, (hl)
        ld (de), a
        inc hl
        inc de
        or a
        jr nz, .wifi_setup_got_an_ap_ssid_copy_loop
        ld (.wifi_ssid_buffer_offset), de
        ld (hl), a ; double zero to terminate
        jp .wifi_setup_loop

.wifi_setup_choice:
        ; A-'a' is the choice made
        sub 'a'
        ld hl, temporary_buffer
        or a
        jr z, .wifi_setup_choice_found
        ld c, a
        ld a, 10
        call putc
.wifi_setup_choice_search:
        ld a, (hl)
        inc hl
        or a
        jr nz, .wifi_setup_choice_search
        ; got one zero
        dec c
        jp nz, .wifi_setup_choice_search
.wifi_setup_choice_found:
        ; hl is the choice
        ld (.wifi_setup_ssid_ptr), hl
        call puts
        ld hl, .wifi_setup_password_text
        call puts
        ; enter password (with echo)
        ld hl, password_buffer
        ld c, 63
.wifi_setup_enter_password_loop:
        ei
        call kb_key_read
        jp c, .wifi_setup_enter_password_loop
        ; cancel and exit
        cp KB_BRK
        jp z, .start_working
        cp KB_EDIT
        jp z, .start_working
        ; accept
        cp 10
        jr z, .wifi_setup_submit_ssid_password
        call putc
        ; any other key is game
        ld (hl), a
        inc hl
        dec c
        jr z, .wifi_setup_submit_ssid_password_too_big  ; password is too big, cancel
        jr .wifi_setup_enter_password_loop
.wifi_setup_submit_ssid_password_too_big:
        ld hl, .wifi_setup_toobig_text
        call puts
        jp .start_working
.wifi_setup_submit_ssid_password:
        xor a
        ld (hl), a      ; asciiz
        ld hl, .wifi_setup_confirm_password_text
        call puts
.wifi_setup_submit_ssid_password_confirm:
        ei
        halt
        call kb_key_read
        jr c, .wifi_setup_submit_ssid_password_confirm
        or $20
        cp 'y'
        jr z, .wifi_setup_submit_ssid_password_write
        cp 'n'
        jr z, .wifi_setup_submit_ssid_password_cancel
        jp .wifi_setup_submit_ssid_password_confirm ; loops
.wifi_setup_submit_ssid_password_write:
        ld hl, .wifi_setup_ssid_ptr
        ld de, password_buffer
        call esp_cwjap
        jp .start_working
.wifi_setup_submit_ssid_password_cancel
        ld hl, .wifi_setup_cancelled
        call puts
        jp .start_working
                
.wifi_ssid_buffer_offset dw 0
.wifi_setup_ssid_ptr dw 0
.wifi_setup_separator_text db ': ', 0
.wifi_setup_password_text db 10, 'Password?', 10, 0
.wifi_setup_toobig_text db 10, 'Too big!', 10, 0
.wifi_setup_confirm_password_text db 10, 'Write password to ESP: (Y/N)', 10, 0
.wifi_setup_cancelled db 10, 'Cancelled', 10, 0
.wifi_setup_next_letter db 'A'
        
.print_info:
        ld hl, .information_text
        call puts
        ld hl, .wifi_mac_text
        call puts
        ld hl, wifi_mac_address
        call puts
        ld a, (wifi_ip_obtained)
        cp 0
        jr z, .print_info_no_wifi_ip
        ld hl, .wifi_ip_text
        call puts
        ld hl, wifi_ip_address
        call puts
.print_info_no_wifi_ip:
        ld a, 10
        call putc
        jp .start_working

.information_text db 10, 'Information:', 10, 0
.wifi_mac_text db 'MAC: ', 0
.wifi_ip_text db 10, 'IP: ', 0

esp_print_input:
.loop
        ld hl, temporary_buffer
        ld bc, temporary_buffer_end - temporary_buffer
        call uart_recv_line
        ret c
        ld hl, timeout_text
        call puts
        jr .loop

esp_print_input_until_timeout:
.loop:
        call uart_recv_byte
        ret c
        jp .loop

hello_world db "Hello World!\n", 0
timeout_text db "timeout\n", 0
failure_text db "failure\n", 0

        include "ula_text.inc"

        include "esp.inc"

        include "uart.inc"

        include "tools.inc"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Used to read one line at a time from the ESP.
;;
;; Size is probably overkill.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


temporary_buffer:
        DUP 1024 - 64
        db 0
        EDUP
password_buffer:
        DUP 64
        db 0
        EDUP
temporary_buffer_end:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Set up the Nex output
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        ; This sets the name of the project, the start address, 
        ; and the initial stack pointer.
        SAVENEX OPEN "wifiupld.nex", Start, $fffe

        ; This asserts the minimum core version.  Set it to the core version 
        ; you are developing on.
        SAVENEX CORE 2,0,0

        ; This sets the border colour while loading (in this case white),
        ; what to do with the file handle of the nex file when starting (0 = 
        ; close file handle as we're not going to access the project.nex 
        ; file after starting.  See sjasmplus documentation), whether
        ; we preserve the next registers (0 = no, we set to default), and 
        ; whether we require the full 2MB expansion (0 = no we don't).
        SAVENEX CFG 0,0,0,1

        ; Generate the Nex file automatically based on which pages you use.
        SAVENEX AUTO
