        ;DEFINE BIOS_FONT 1

ULA_PIXELS_BASE EQU $4000
ULA_ATTRIBUTES_BASE EQU $5800
ULA_WIDTH EQU 256
ULA_HEIGHT EQU 192
        
        IFDEF BIOS_FONT
CHARMAP_BASE EQU $3d00
        ELSE
CHARMAP_BASE EQU full_8x8_font
        ENDIF

char_xy:
char_x  db 0
char_y  db 0

ula_xy:
             db 0
ula_y_offset db 0
ula_scrolling db 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; clears the screen
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

cls:
        ld hl, ULA_PIXELS_BASE
        ld de, hl
        ld a, $00
        ld (de), a
        inc de
        ld bc, ULA_ATTRIBUTES_BASE - ULA_PIXELS_BASE - 1
        ldir
        inc hl
        ld a, %00000111
        ld (de), a
        inc de
        ld bc, $300 - 1
        ldir
        xor a
        ld (char_x), a
        ld (char_y), a
        ld (ula_y_offset), a
        ld (ula_scrolling), a
        nextreg ULA_Y_OFFSET_REGISTER, a
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; prints a hexadecimal number nibble (a)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

print_hex_a_lo_unsafe:
        cp 10
        jr nc, .chr_af
        add '0'
        call putc_unsafe
        ret
.chr_af:
        add 'A' - 10
        call putc_unsafe
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; prints a hexadecimal number (a)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

print_hex_a:
        push af
        push bc
        push de
        push hl
        push af
        srl a
        srl a
        srl a
        srl a
        call print_hex_a_lo_unsafe    ; d = 10, e = 0
        pop af
        and $0f
        call print_hex_a_lo_unsafe    ; d = 10, e = 0
        pop hl
        pop de
        pop bc
        pop af
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; prints a hexadecimal number (HL)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

print_hex_hl:
        push af
        push bc
        push de
        push hl
        ld a, h
        srl a
        srl a
        srl a
        srl a
        push hl
        call print_hex_a_lo_unsafe    ; d = 10, e = 0
        pop hl
        ld a, h
        and $0f
        push hl
        call print_hex_a_lo_unsafe    ; d = 10, e = 0
        pop hl
        ld a, l
        srl a
        srl a
        srl a
        srl a
        push hl
        call print_hex_a_lo_unsafe    ; d = 10, e = 0
        pop hl
        ld a, l
        and $0f
        call print_hex_a_lo_unsafe    ; d = 10, e = 0
        pop hl
        pop de
        pop bc
        pop af
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; prints a decimal number (HL)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

print_hl:
        push af
        push bc
        push de
        push hl
        
        ld de, $0a00    ; d = 10, e = 0

.div_loop:
        call div_hl_d
        inc e
        add '0'
        push af
        ld a, l
        or h
        jp nz, .div_loop

.print_loop:
        pop af
        push de
        call putc_unsafe
        pop de
        dec e
        jp nz, .print_loop

        pop hl
        pop de
        pop bc
        pop af

        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; prints a C string
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

puts:
        push af
        push bc
        push de
.loop:
        ld a, (hl)
        or a
        jr z, .end
        push hl
        call putc_unsafe
        pop hl
        inc hl
        jp .loop
.end:
        pop de
        pop bc
        pop af
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; prints a char (a), preserves registers
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

putc:
        push af
        push bc
        push de
        push hl
        call putc_unsafe
        pop hl
        pop de
        pop bc
        pop af
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; prints a char (a), doesn't preserve registers
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

putc_unsafe:
        IFDEF BIOS_FONT
        cp a, 32
        jp c, .not_printable
        cp a, 128
        jp nc, .not_printable
        sub 32
        ELSE
        cp a, 10
        jp z, .char_is_line_feed
        ENDIF
.printable
        ld de, (char_xy)
        pixelad
        ld e, a
        ld d, 0
        add de, de
        add de, de
        add de, de
        add de, CHARMAP_BASE
        ld b, 8
.loop
        ld a, (de)
        ld (hl), a
        pixeldn
        inc de
        djnz .loop

        ld de, (char_xy)
        ld a, e
        add a, 8
        ld (char_x), a
        jp nz, .cursor_moved_right
        ld a, d

.one_line_down:
        add a, 8
        cp a, ULA_HEIGHT
        jp c, .cursor_moved_down
        xor a
        ld (char_y), a
        ld a, 1
        ld (ula_scrolling), a
        call ula_scroll_down
        ret

        IFDEF BIOS_FONT
.not_printable:
        cp a, 10
        ret nz
        ENDIF

.char_is_line_feed:
        xor a
        ld (char_x), a
        ld a, (char_y)
        jp .one_line_down

.cursor_moved_down:
        ld (char_y), a
        ld a, (ula_scrolling)
        or a
        ret z
        call ula_scroll_down       
.cursor_moved_right:
        ret

SCROLLUP_OFFSET EQU $100

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Scrolls the screen one line up (meaning doing down)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ula_scroll_down:
        ld a, (ula_y_offset)
        ld d, a
        add a, 8
        cp 192
        jp c, .cycle
        xor a
.cycle:
        ld (ula_y_offset), a
        nextreg ULA_Y_OFFSET_REGISTER, a
        xor a
        ld e, a
        ld b, 32
        pixelad
        ld de, hl
.fill_first_line:
        ld (hl), a
        inc hl
        djnz .fill_first_line
        ld c, 7
.next_line
        ld b, 32
        ld hl, de
        pixeldn
        ld de, hl
.fill_next_line
        ld (hl), a
        inc hl
        djnz .fill_next_line
        dec c
        jp nz, .next_line

        ret

        IFNDEF BIOS_FONT
full_8x8_font:        
        include "font.inc"
        ENDIF
