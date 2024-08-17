
SPRITE_STATUS__SLOT_SELECT              EQU $303B
; Write:
; Used to select the sprite index/slot (0..63 for pattern slots and 0..127 for sprite attribute slots)
; used by Sprite Attribute Upload ($xx57 / 87)
; and Sprite Pattern Upload ($xx5B / 91)
;
; Read:
; 2-7	Reserved
; 1 	Maximum sprites per line flag. Set to 1 when sprite renderer ran out of time when preparing next scanline. Reading the port will reset it to 0.
; 0 	Collision flag. Set to 1 when non transparent pixels of any 2 sprites overlap during rendering scanline. Reading the port will reset it to 0.

SPRITE_ATTRIBUTE_UPLOAD                 EQU $0057

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


MACHINE_ID_REGISTER                     EQU $00 ; Identifies TBBlue board type. Should always be 10 on Next.
CORE_VERSION_REGISTER                   EQU $01 ; Identifies core (FPGA image) version.
NEXT_RESET_REGISTER                     EQU $02 ; Identifies type of last reset. Can be written to force reset.
MACHINE_TYPE_REGISTER                   EQU $03 ; Identifies timing and machine type.
CONFIG_MAPPING_REGISTER                 EQU $04 ; In config mode, allows RAM to be mapped to ROM area.
PERIPHERAL_1_REGISTER                   EQU $05 ; Sets joystick mode, video frequency and Scandoubler.
PERIPHERAL_2_REGISTER                   EQU $06 ; Enables CPU Speed key, DivMMC, Multiface, Mouse and AY audio.
CPU_SPEED_REGISTER                      EQU $07 ; Sets CPU Speed, reads actual speed.
; 7-2	Reserved, must be 0
; 1-0	Set CPU speed (is set to %00 on soft reset)
;       %00 = 3.5MHz
;       %01 = 7MHz
;       %10 = 14MHz
;       %11 = 28MHz (works since core 3.0)

PERIPHERAL_3_REGISTER                   EQU $08 ; ABC/ACB Stereo, Internal Speaker, SpecDrum, Timex Video Modes, Turbo Sound Next, RAM contention and [un]lock 128k paging.
PERIPHERAL_4_REGISTER                   EQU $09 ; Sets scanlines, AY mono output, Sprite-id lockstep, reset DivMMC mapram and disable HDMI audio.
PERIPHERAL_5_REGISTER                   EQU $0A ; Mouse buttons and DPI config
CORE_VERSION_REGISTER_SUB_MINOR         EQU $0E ; Identifies core (FPGA image) version (sub minor number).
ANTI_BRICK_REGISTER                     EQU $10 ; Used within the Anti-brick system.
VIDEO_TIMING_REGISTER                   EQU $11 ; Sets video output timing variant.
LAYER_2_RAM_PAGE_REGTISTER              EQU $12 ; Sets the bank number where Layer 2 video memory begins.
LAYER_2_RAM_SHADOW_PAGE_REGISTER        EQU $13 ; Sets the bank number where the Layer 2 shadow screen begins.
GLOBAL_TRANSPARENCY_REGISTER            EQU $14 ; Sets the "transparent" colour for Layer 2, ULA and LoRes pixel data.

SPRITE_AND_LAYERS_SYSTEM_REGISTER       EQU $15	; ; Enables/disables Sprites and Lores Layer, and chooses priority of sprites and Layer 2.
; 7	    Enable Lores Layer
; 6	    If 1, the sprite rendering priority is flipped, i.e. sprite 0 is on top of other sprites (0 after reset)
; 5	    If 1, the clipping works even in "over border" mode (doubling X-axis coordinates of clip window) (0 after reset)
; 4-2	Layers priority and mixing
; 1	    Enable sprites over border (0 after reset)
; 0	    Enable sprite visibility (0 after reset)
;
; %000	S L U (Sprites are at top, Layer 2 under, Enhanced_ULA at bottom)
; %001	L S U
; %010	S U L
; %011	L U S
; %100	U S L
; %101	U L S
; %110	since core3.1.1: (U|T)S(T|U)(B+L) Blending layer and Layer 2 combined, colours clamped to [0,7]
;       S (U+L) Colours from ULA and Layer 2 are added (per R/G/B channel) and clamped to 7.
; %111	since core3.1.1: (U|T)S(T|U)(B+L-5) Blending layer and Layer 2 combined, colours clamped to [0,7]
;       S (U+L-5) Similar as previous, but per R/G/B channel (U+L-5) is calculated and result is clamped by 0..7 range.

LAYER_2_X_OFFSET_REGISTER               EQU $16 ; Sets the pixel offset used for drawing Layer 2 graphics on the screen.
LAYER_2_Y_OFFSET_REGISTER               EQU $17 ; Sets the Y offset used when drawing Layer 2 graphics on the screen.
CLIP_WINDOW_LAYER_2_REGISTER            EQU $18 ; Sets and reads clip-window for Layer 2.
CLIP_WINDOW_SPRITES_REGISTER            EQU $19 ; Sets and reads clip-window for Sprites
CLIP_WINDOW_ULA_LORES_REGISTER          EQU $1A ; Sets and reads clip-window for ULA/LoRes layer.
CLIP_WINDOW_TILEMAP_REGISTER            EQU $1B ; Sets and reads clip-window for Tilemap.

    MACRO clip_window_tilemap x0,x1,y0,y1
        nextreg CLIP_WINDOW_TILEMAP_REGISTER, x0
        nextreg CLIP_WINDOW_TILEMAP_REGISTER, x1
        nextreg CLIP_WINDOW_TILEMAP_REGISTER, y0
        nextreg CLIP_WINDOW_TILEMAP_REGISTER, y1
    ENDM

CLIP_WINDOW_CONTROL_REGISTER            EQU $1C ; Controls (resets) the clip-window registers indices.
ACTIVE_VIDEO_LINE_MSB_REGISTER          EQU $1E ; Holds the MSB (only, as bit 0) of the raster line currently being drawn.
ACTIVE_VIDEO_LINE_LSB_REGISTER          EQU $1F ; Holds the eight LSBs of the raster line currently being drawn.
VIDEO_LINE_INTERRUPT_CONTROL_REGISTER   EQU $22 ; Controls the timing of raster interrupts and the ULA frame interrupt.
VIDEO_LINE_INTERRUPT_VALUE_REGISTER     EQU $23 ; Holds the eight LSBs of the line on which a raster interrupt should occur.
ULA_X_OFFSET_REGISTER                   EQU $26 ; Pixel X offset (0..255) to use when drawing ULA Layer.
ULA_Y_OFFSET_REGISTER                   EQU $27 ; Pixel Y offset (0..191) to use when drawing ULA Layer.
KEYMAP_HIGH_ADDRESS_REGISTER            EQU $28 ; PS/2 Keymap address MSB, read (pending) first byte of palette colour
KEYMAP_LOW_ADDRESS_REGISTER             EQU $29 ; PS/2 Keymap address LSB.
KEYMAP_HIGH_DATA_REGISTER               EQU $2A ; High data to PS/2 Keymap (MSB of data in bit 0)
KEYMAP_LOW_DATA_REGISTER                EQU $2B ; Low eight LSBs of PS/2 Keymap data.
DAC_B_LEFT_MIRROR_REGISTER              EQU $2C ; DAC B mirror, read current I2S left MSB
DAC_A_D_MONO_MIRROR_REGISTER            EQU $2D ; SpecDrum port 0xDF / DAC A+D mirror, read current I2S LSB
DAC_C_RIGHT_MIRROR_REGISTER             EQU $2E ; DAC C mirror, read current I2S right MSB
TILEMAP_OFFSET_X_MSB_REGISTER           EQU $2F ; Sets the pixel offset (two high bits) used for drawing Tilemap graphics on the screen.
TILEMAP_OFFSET_X_LSB_REGISTER           EQU $30 ; Sets the pixel offset (eight low bits) used for drawing Tilemap graphics on the screen.
TILEMAP_OFFSET_Y_REGISTER               EQU $31 ; Sets the pixel offset used for drawing Tilemap graphics on the screen.

    MACRO tilemap_offset_x_msb hix
        nextreg TILEMAP_OFFSET_X_MSB_REGISTER, hix
    ENDM

    MACRO tilemap_offset_x_lsb lox
        nextreg TILEMAP_OFFSET_X_LSB_REGISTER, lox
    ENDM

    MACRO tilemap_offset_y loy
        nextreg TILEMAP_OFFSET_Y_REGISTER, loy
    ENDM

;LoRes X Offset Register	$32 ; Pixel X offset (0..255) to use when drawing LoRes Layer.
;LoRes Y Offset Register	$33 ; Pixel Y offset (0..191) to use when drawing LoRes Layer.
;Sprite port-mirror Index Register	$34 ; Selects sprite index 0..127 to be affected by writes to other Sprite ports (and mirrors).
;Sprite port-mirror Attribute 0 Register	$35 ; Nextreg port-mirror to write directly into "byte 1" of Sprite Attribute Upload ($xx57 / 87).
;Sprite port-mirror Attribute 1 Register	$36 ; Nextreg port-mirror to write directly into "byte 2" of Sprite Attribute Upload ($xx57 / 87).
;Sprite port-mirror Attribute 2 Register	$37 ; Nextreg port-mirror to write directly into "byte 3" of Sprite Attribute Upload ($xx57 / 87).
;Sprite port-mirror Attribute 3 Register	$38 ; Nextreg port-mirror to write directly into "byte 4" of Sprite Attribute Upload ($xx57 / 87).
;Sprite port-mirror Attribute 4 Register	$39 ; Nextreg port-mirror to write directly into "byte 5" of Sprite Attribute Upload ($xx57 / 87).

PALETTE_INDEX_REGISTER                  EQU $40 ; Chooses an palette element (index) to manipulate with

    MACRO palette_index index
        nextreg PALETTE_INDEX_REGISTER, index
    ENDM

PALETTE_VALUE_REGISTER                  EQU $41 ; Use to set/read 8-bit colours of the ULANext palette.
ENHANCED_ULA_INK_COLOR_MASK             EQU $42 ; Specifies mask to extract ink colour from attribute cell value in ULANext mode.

ENHANCED_ULA_CONTROL_REGISTER           EQU $43 ; Enables or disables Enhanced ULA interpretation of attribute values and toggles active palette.
; 7     1 to disable palette index write auto-increment
; 6-4   Select palette for reading or writing
; 3     Select Sprites palette (0 = first palette, 1 = second palette)
; 2     Select Layer 2 palette (0 = first palette, 1 = second palette)
; 1     Select ULA palette (0 = first palette, 1 = second palette)
; 0     Enable ULANext mode if 1. (0 after a reset)
;
; bits 6-4
; %000 ULA first palette
; %100 ULA second palette
; %001 Layer 2 first palette
; %101 Layer 2 second palette
; %010 Sprites first palette
; %110 Sprites second palette
; %011 Tilemap first palette
; %111 Tilemap second palette

    MACRO palette_layer2_1_set
        nextreg ENHANCED_ULA_CONTROL_REGISTER, %00010000
    ENDM

    MACRO palette_tilemap_1_set
        nextreg ENHANCED_ULA_CONTROL_REGISTER, %00110000
    ENDM

ENHANCED_ULA_PALETTE_EXTENSION          EQU $44 ; Use to set 9-bit (2-byte) colours of the Enhanced ULA palette, or to read second byte of colour.

TRANSPARENCY_COLOUR_FALLBACK_REGISTER   EQU $4A ; 8-bit colour to be used when all layers contain transparent pixel.
SPRITES_TRANSPARENCY_INDEX_REGISTER     EQU $4B ; Index into sprite palette (of "transparent" colour).
TILEMAP_TRANSPARENCY_INDEX_REGISTER     EQU $4C ; Index into Tilemap palette (of "transparent" colour).

MEMORY_MANAGEMENT_SLOT_O_BANK           EQU $50 ; Selects the 8k-bank stored in 8k-slot 0 (see Memory map).
MEMORY_MANAGEMENT_SLOT_1_BANK           EQU $51 ; Selects the 8k-bank stored in 8k-slot 1 (see Memory map).
MEMORY_MANAGEMENT_SLOT_2_BANK           EQU $52 ; Selects the 8k-bank stored in 8k-slot 2 (see Memory map).
MEMORY_MANAGEMENT_SLOT_3_BANK           EQU $53 ; Selects the 8k-bank stored in 8k-slot 3 (see Memory map).
MEMORY_MANAGEMENT_SLOT_4_BANK           EQU $54 ; Selects the 8k-bank stored in 8k-slot 4 (see Memory map).
MEMORY_MANAGEMENT_SLOT_5_BANK           EQU $55 ; Selects the 8k-bank stored in 8k-slot 5 (see Memory map).
MEMORY_MANAGEMENT_SLOT_6_BANK           EQU $56 ; Selects the 8k-bank stored in 8k-slot 6 (see Memory map).
MEMORY_MANAGEMENT_SLOT_7_BANK           EQU $57 ; Selects the 8k-bank stored in 8k-slot 7 (see Memory map).

    MACRO mmu_slot_bank_set __slot__, __bank__
        /*
        push af
        push bc
        push af
        ld a, __slot__
        ld b, a
        pop af
        ld a, __bank__
        call mmu_slot_bank_change_check ; b = slot, a = bank
        pop bc
        pop af
        */
        
        nextreg MEMORY_MANAGEMENT_SLOT_O_BANK + __slot__, __bank__
    ENDM

COPPER_DATA                             EQU $60 ; Used to upload code to the Copper.
COPPER_CONTROL_LOW_BYTE                 EQU $61 ; Holds low byte of Copper control bits.
COPPER_CONTROL_HIGH_BYTE                EQU $62 ; Holds high byte of Copper control flags.
COPPER_DATA_16BITS_WRITE_REGISTER       EQU $63 ; Used to upload code to the Copper.
VERTICAL_VIDEO_LINE_OFFSET_REGISTER     EQU $64 ; Offset numbering of raster lines in copper/interrupt/active register
ULA_CONTROL_REGISTER                    EQU $68 ; Disable ULA, controls ULA mixing/blending, enable ULA+
; 7     1 to disable ULA output (soft reset = 0)
; 6-5	since core3.1.1: Blending in SLU modes 6 & 7 (soft reset = 0)
;       = 00 for ULA as blend colour
;       = 10 for ULA/tilemap as blend colour
;       = 11 for tilemap as blend colour
;       = 01 for no blending
;       bit 6: 0 to select the ULA colour for blending in SLU modes 6 & 7 (soft reset = 0) 1 to select the ULA/tilemap mix for blending in SLU modes 6 & 7
; 4     since core3.1.4: Cancel entries in 8x5 matrix for extended keys
; 3 	ULA+ enable (soft reset = 0)
; 2 	may change (ULA half pixel scroll) (soft reset = 0)
; 1 	Reserved, must be 0
; 0 	1 to enable stencil mode when both the ULA and tilemap are enabled
;       (if either are transparent the result is transparent otherwise the result is a logical AND of both colours)

DISPLAY_CONTROL_1_REGISTER              EQU $69 ; Layer2, ULA shadow, Timex $FF port
LORES_CONTROL_REGISTER                  EQU $6A ; LoRes Radastan mode

TILEMAP_CONTROL_REGISTER                EQU $6B ; Controls Tilemap mode.
; 7	1 to enable the tilemap
; 6	0 for 40x32, 1 for 80x32
; 5	1 to eliminate the attribute entry in the tilemap
; 4	palette select (0 = first Tilemap palette, 1 = second)
; 3	enable "text mode"
; 2	Reserved, must be 0
; 1	1 to activate 512 tile mode (bit 0 of tile attribute is ninth bit of tile-id)
;   0 to use bit 0 of tile attribute as "ULA over tilemap" per-tile-selector
; 0	1 to enforce "tilemap over ULA" layer priority

    MACRO tilemap_enable_40x32_pal0
        nextreg TILEMAP_CONTROL_REGISTER, %10000000
    ENDM

    MACRO tilemap_enable_40x32_noattrib_pal0
        nextreg TILEMAP_CONTROL_REGISTER, %10100000
    ENDM

DEFAULT_TILEMAP_ATTRIBUTE_REGISTER      EQU $6C ; Default tile attribute for 8-bit only maps.
; 7-4	Palette Offset (copied as bits 7-4 in final palette index)
; 3	X mirror
; 2	Y mirror
; 1	Rotate
; 0	If in 512-tile-mode: bit 8 of tile-id
;   else draw priority: 1 = ULA over tilemap, 0 = tilemap over ULA
    MACRO default_tilemap_attribute attrib
        nextreg DEFAULT_TILEMAP_ATTRIBUTE_REGISTER, attrib
    ENDM

TILEMAP_BASE_ADDRESS_REGISTER           EQU $6E ; Base address of the 40x32 or 80x32 tile map (similar to text-mode of other computers).
; bits 7-6 = Read back as zero, write values ignored
; bits 5-0 = MSB of address of the tile map in Bank 5

    MACRO tilemap_base_address_set mult_of_256
        nextreg TILEMAP_BASE_ADDRESS_REGISTER, mult_of_256
    ENDM

TILE_DEFINITION_BASE_ADDRESS_REGISTER   EQU $6F ; Base address of the tiles' graphics.
; bits 7-6 = Read back as zero, write values ignored
; bits 5-0 = MSB of address of the tile definitions in Bank 5

    MACRO tile_definition_base_address_set mult_of_256
        nextreg TILE_DEFINITION_BASE_ADDRESS_REGISTER, mult_of_256
    ENDM

LAYER_2_CONTROL_REGISTER                EQU $70 ; Layer 2 resolution, palette offset
; 7-6	Reserved, must be 0
; 5-4	Layer 2 resolution (soft reset = 0)
;       %00 = 256x192x8bpp
;       %01 = 320x256x8bpp
;       %10 = 640x256x4bpp
; 3-0	Palette offset (soft reset = 0)