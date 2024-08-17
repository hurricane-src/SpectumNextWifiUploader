        MACRO add_bc_a
        add a, c ; 30 cycles
        ld  c, a
        adc a, b
        sub c
        ld b, a
        ENDM

        MACRO add_de_a
        add a, e ; 30 cycles
        ld  e, a
        adc a, d
        sub e
        ld d, a
        ENDM

        MACRO add_hl_a
        add a, l ; 30 cycles
        ld  l, a
        adc a, h
        sub l
        ld h, a
        ENDM

        MACRO add_ix_a
        add a, ixl ; 30 cycles
        ld  ixl, a
        adc a, ixh
        sub ixl
        ld ixh, a
        ENDM

        MACRO add_iy_a
        add a, iyl ; 30 cycles
        ld  iyl, a
        adc a, iyh
        sub iyl
        ld iyh, a
        ENDM

        MACRO border_color_change
        ld bc, $00fe ; border color change
        out (c), a
        ENDM