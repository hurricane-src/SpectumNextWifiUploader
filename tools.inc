;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Get a value from the Next registers
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;=

nextreg_get:
        push bc
        ld bc, $243B
        out (c), a
        inc b
        in a, (c)
        pop bc
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Compares two 16 bits values for equality
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

cmp_equals_hl_de: ; de, hl
        ld a, d
        xor h
        jr nz, .differs
        ld a, e
        xor l
.differs:
        ret

;; http://www.massmind.org/techref/zilog/z80/part4.htm

div_hl_d: ; hl/d
        xor a
        ld b, 16
.loop:
        add hl, hl
        rla
        cp d
        jp c, .next
        sub d
        inc l
.next:
        djnz .loop
        ret

strlen:
        ld bc, hl
        push af
        push hl
.loop:
        ld a, (hl)
        inc hl
        or a
        jp nz, .loop
        sub hl, bc
        ld bc, hl
        dec bc
        pop hl
        pop af
        ret

FRAME_COUNTER_ADDRESS EQU $5c78

wait_20ms: ; 50Hz => 
        push af
        push bc
        ld a, (FRAME_COUNTER_ADDRESS)
        inc a
        ld b, a
.wait:  ei
        halt
        ld a, (FRAME_COUNTER_ADDRESS)
        cp b
        jr z, .wait
        pop bc
        pop af
        ret
        
wait_100ms:
        push af
        push bc
        ld a, (FRAME_COUNTER_ADDRESS)
        add a, 5
        ld b, a
.wait:
        ei
        halt
        ld a, (FRAME_COUNTER_ADDRESS)
        cp b
        jr z, .wait
        pop bc
        pop af
        ret

wait_500ms:
        push af
        push bc
        ld a, (FRAME_COUNTER_ADDRESS)
        add a, 25
        ld b, a
.wait:
        ei
        halt
        ld a, (FRAME_COUNTER_ADDRESS)
        cp b
        jr z, .wait
        pop bc
        pop af
        ret

wait_1000ms:
        push af
        push bc
        ld a, (FRAME_COUNTER_ADDRESS)
        add a, 50
        ld b, a
.wait:
        ei
        halt
        ld a, (FRAME_COUNTER_ADDRESS)
        cp b
        jr z, .wait
        pop bc
        pop af
        ret
        