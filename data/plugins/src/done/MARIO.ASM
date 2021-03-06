             .title      "Mario 7 in 1 Reader Plugin"


;06/11/00 
;Written by KH
;Version 1.0
             
             ;vectors for standard system calls

send_byte:   .equ 0200h
baton:       .equ 0203h
chk_vram:    .equ 0206h
chk_wram:    .equ 0209h
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
romsiz:      .equ 00e7h
curr_bank:   .equ 00e8h

             ;plugin header that describes what it does
             
             .org 0380h
             
             .db "Pirate Mario "
             .db "7 in 1 multi"
             .db "\n\r",0

             .fill 0400h-*,00h    ;all plugins must reside at 400h

             lda 01fch
             cmp #'K'         ;K
             bne firstinit
             lda 01fdh
             cmp #'H'
             bne firstinit
             lda 01feh
             cmp #'!'
             bne firstinit
             
             inc 01ffh
             lda #002h
             ldx 01ffh
             beq do_prg
             lda #004h
             dex
             beq do_prg
             lda #006h
             dex
             beq do_prg
             jmp do_chrs
             
firstinit:   lda #04h
             jsr send_byte
             
             lda #'K'
             sta 01fch
             lda #'H'
             sta 01fdh
             lda #'!'
             sta 01feh
             lda #0ffh
             sta 01ffh
             
             lda #000h
             jsr send_byte
             lda #004h
             jsr send_byte
             lda #004h
             jsr send_byte     ;reset every 256K
             
             lda #000h
             jsr send_byte
             lda #010h
             jsr send_byte
             lda #001h
             jsr send_byte     ;1mbyte PRG

             lda #000h
             beq do_prgx

do_prg:      pha
             lda #000h
             jsr send_byte
             lda #004h
             jsr send_byte      ;send next reset size
             pla

do_prgx:     ldx #080h
             stx 0a001h
             sta 06800h    ;store bank
             lda #00h
             sta 0a000h
             

             lda #8
             sta romsiz
             ldy #020h

gotit:       lsr romsiz
             lda #0
             ror a
             sta temp3_hi
        ;     jsr send_byte
             lda romsiz
         ;    jsr send_byte
             lda romsiz
             asl a
             asl a
             asl a
             sta temp3_lo    ;# 8K banks
             lda #01h
         ;    jsr send_byte

send_plp:    lda #6
             sta 08000h
             lda temp3_hi
             sta 08001h
             inc temp3_hi
             lda #0
             sta temp1_lo
             lda #080h
             sta temp1_hi
             ldx #020h
             ldy #0

send_plp2:   lda (temp1),y
             jsr send_byte
             iny
             bne send_plp2
             inc temp1_hi
             jsr baton
             dex
             bne send_plp2    ;send 8K bank
             dec temp3_lo
             bne send_plp
             

dead:        jmp dead         ;wait for PC to reset us

do_chrs:     dex
             beq do_chrin
             lda #004h
             dex
             beq do_chr
             lda #020h
             dex
             beq do_chr
             lda #024h
             dex
             beq do_chr
             lda #000h
             jsr send_byte
             lda #000h
             sta 01fch         ;reset counter
             jsr send_byte     ;bullshit write to keep copyNES happy
             jmp no_chr

do_chrin:    lda #000h
             jsr send_byte
             lda #000h
             jsr send_byte     ;bullshit write to keep copyNES happy
             
             lda #000h
             jsr send_byte
             lda #004h
             jsr send_byte
             lda #004h
             jsr send_byte     ;reset every 256K
             
             lda #000h
             jsr send_byte
             lda #010h
             jsr send_byte
             lda #002h
             jsr send_byte     ;1mbyte CHR

             lda #000h
             beq do_chrx

do_chr:      pha
             lda #000h
             jsr send_byte
             lda #004h
             jsr send_byte      ;send next reset size
             pla

do_chrx:     ldx #080h
             stx 0a001h
             sta 06800h    ;store bank
             
             lda #00h
             sta 0a000h
             
no_ram3:     lda #8
             sta romsiz

gotit2:      lda romsiz
             pha
             lsr romsiz
             lda #0
             sta temp3_hi    ;start out at 0
             ror a
     ;        jsr send_byte
             lda romsiz
     ;        jsr send_byte
             lda #02h
     ;        jsr send_byte
             pla
             asl a
             asl a
             asl a
             asl a
             asl a  ;1,2,4,8 == 20,40,80,00
             sta temp3_lo

send_plp3:   lda #082h
             sta 08000h
             lda temp3_hi
             sta 08001h      ;current bank
             lda #00h
             jsr wr_ppu
             lda 2007h      ;set up PPU
             ldy #0
             ldx #4

send_plp4:   lda 2007h
             jsr send_byte
             iny
             bne send_plp4
             jsr baton
             dex
             bne send_plp4
             inc temp3_hi
             dec temp3_lo
             bne send_plp3
             
dead2:       jmp dead2        ;wait for PC to reset us

;check for save-game RAM and back it up if it exists             

no_chr:      lda #080h
             sta 0a001h
             
             jsr chk_wram
             bne no_ram2
             lda #020h
             tax
             jsr send_byte
             lda #0
             jsr send_byte
             lda #3
             jsr send_byte
             lda #0
             sta temp1_lo
             tay
             lda #060h
             sta temp1_hi
            
sr_lp:       lda (temp1),y
             jsr send_byte
             iny
             bne sr_lp
             inc temp1_hi
             jsr baton
             dex
             bne sr_lp
             lda #000h
             sta 0a001h
             

no_ram2:     lda #0
             jsr send_byte
             lda #0
             jsr send_byte
             lda #0
             jsr send_byte

             rts


;y = bank to compare
;z=same
comp_bank:   ldx #0
             stx temp3_lo    ;lower pointer
             sty temp3_hi    ;upper pointer
             sty temp4       ;# blocks to compare

cb_loop:     lda #000h       ;init pointers
             sta temp1_lo
             sta temp2_lo
             lda #080h
             sta temp1_hi
             lda #0a0h
             sta temp2_hi    ;pointers 1,2 to 8000/a000
             lda #006h
             sta 08000h
             lda temp3_lo
             sta 08001h
             lda #007h
             sta 08000h
             lda temp3_hi
             sta 08001h      ;write in current banks
             ldx #020h
             ldy #0

cb_loop2:    lda (temp1),y
             cmp (temp2),y
             bne diff
             iny
             bne cb_loop2
             inc temp1_hi
             inc temp2_hi
             dex
             bne cb_loop2
             inc temp3_lo
             inc temp3_hi
             dec temp4
             bne cb_loop

diff:        rts

;y = bank to compare
;z=same
;for CHR
comp_bank2:  ldx #0
             stx temp3_lo    ;lower pointer
             sty temp3_hi    ;upper pointer
             sty temp4       ;# blocks to compare

cc_loop:     lda #082h
             sta 08000h
             lda temp3_lo
             sta 08001h
             lda #083h
             sta 08000h
             lda temp3_hi
             sta 08001h      ;write in current banks
             ldx #004h
             lda #000h
             sta curr_bank   ;reset current bank

cc_loop2:    ldy #0
             lda curr_bank
             sta 2006h       ;pointer =000h
             sty 2006h
             lda 2007h       ;garbage read

ql:          lda 2007h
             sta 0300h,y
             iny
             bne ql          ;load 256 bytes for testing
             lda curr_bank
             clc
             adc #4
             inc curr_bank
             sta 2006h       ;pointer =400h
             sty 2006h
             lda 2007h       ;garbage read

cc_loop3:    lda 2007h
             cmp 0300h,y
             bne diff2
             iny
             bne cc_loop3
             dex
             bne cc_loop2
             inc temp3_lo
             inc temp3_hi
             dec temp4
             bne cc_loop

diff2:       rts


             .fill 0800h-*,0ffh   ;fill rest to get 1K of data

             .end
