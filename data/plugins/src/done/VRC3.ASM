             .title        "VRC3 Reader Plugin"


;10/29/00 
;Written by KH
;Version 1.0
             
             ;vectors for standard system calls

send_byte:   .equ 0200h
baton:       .equ 0203h
wr_ppu:      .equ 020ch

temp1:       .equ 00e0h
temp1_lo:    .equ 00e0h
temp1_hi:    .equ 00e1h
temp2:       .equ 00e2h
temp2_lo:    .equ 00e2h
temp2_hi:    .equ 00e3h
temp3:       .equ 00e4h
temp3_lo:    .equ 00e4h
temp3_hi:    .equ 00e5h
temp4:       .equ 00e6h
temp4_lo:    .equ 00e6h
temp4_hi:    .equ 00e7h
temp5:       .equ 00e8h
temp5_lo:    .equ 00e8h
temp5_hi:    .equ 00e9h
x_temp:      .equ 00eah
y_temp:      .equ 00ebh
temp_bank:   .equ 00ech

             ;plugin header that describes what it does
             
             .org 0380h
             
             .db "VRC3 -- 32K to 256K" 
             .db " carts w/ 8K VRAM",0

             .fill 0400h-*,00h    ;all plugins must reside at 400h

             
;check mirroring             

             ldx #00h
             
             lda #020h
             jsr wr_ppu
             lda #055h
             sta 2007h
             lda #0aah
             sta 2007h
             lda #024h
             jsr wr_ppu
             lda 2007h
             lda 2007h
             cmp #055h
             bne horz_mir
             lda 2007h
             cmp #0aah
             bne horz_mir
             lda #020h
             jsr wr_ppu
             lda #0aah
             sta 2007h
             lda #024h
             jsr wr_ppu
             lda 2007h
             lda 2007h
             cmp #0aah
             bne horz_mir
             inx

horz_mir:    txa
             jsr send_byte   ;send byte
             
             lda #1
             sta temp_bank
             lda #007h
             jsr test_bank
             beq got_it
             asl temp_bank
             lda #00fh
             jsr test_bank
             beq got_it
             asl temp_bank
             
got_it:      ldx temp_bank
             lda #0
             jsr send_byte
             txa             ;send size
             jsr send_byte
             lda #001h
             jsr send_byte   ;send PRG 
             
             lda #0
             sta temp3_lo
             txa
             asl a
             asl a           ;4, 8 or 16 banks
             sta temp3_hi
            
             
di_loop2:    lda temp3_lo
             jsr wr_bank
             ldx #040h
             ldy #0
             sty temp1_lo
             lda #080h
             sta temp1_hi

dump_it:     lda (temp1),y
             jsr send_byte
             iny
             bne dump_it
             jsr baton
             inc temp1_hi
             dex
             bne dump_it
             inc temp3_lo
             dec temp3_hi
             bne di_loop2
             
             lda #000h
             jsr send_byte
             lda #000h
             jsr send_byte
             lda #000h
             jsr send_byte  ;send end block
             rts
             
wr_bank:     stx x_temp
             sty y_temp
             sta temp4_lo   ;store desired bank
             lda #0
             sta temp5_lo
             lda #0f0h
             sta temp5_hi   ;start addr
             ldx #010h      ;# iterations
             ldy #0

wrb_loop:    lda (temp5),y
             and #0fh
             cmp temp4_lo   ;check current byte
             beq gotit
             iny
             bne wrb_loop
             inc temp5_hi
             bne wrb_loop   ;keep checking
             lda temp4_lo
             sta 0f000h
             ldx x_temp
             ldy y_temp
             rts            ;if it wasn't found, bag it
             
gotit:       lda (temp5),y             
             sta (temp5),y  ;if we found it, do the switch
             ldx x_temp
             ldy y_temp
             rts



test_bank:   jsr wr_bank
             lda #0
             sta temp1_lo
             sta temp2_lo
             lda #080h
             sta temp1_hi
             lda #0c0h
             sta temp2_hi
             
             ldx #040h
             ldy #0

chk_loopq:   lda (temp1),y
             cmp (temp2),y
             bne not_it
             iny
             bne chk_loopq
             inc temp1_hi
             inc temp2_hi
             dex
             bne chk_loopq

not_it:      rts


             .fill 0800h-*,0ffh   ;fill rest to get 1K of data

             .end
