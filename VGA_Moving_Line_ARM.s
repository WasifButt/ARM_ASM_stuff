.text
.global _start 

.equ pixel_ctrl_ptr, 0xFF203020         // pixel ctrl ptr address 

_start:
    LDR     R12, =pixel_ctrl_ptr        // intialize R12 
    MOV     SP, #0x1000
    B       MAIN                        // jump to main 
WAIT_FOR_VSYNC:
    PUSH    {R0-R3} 
    MOV     R0, #1                      // set R12 value to 1 
    STR     R0, [R12] 
LOOP:
    LDR     R1, [R12, #0xC]             // Load R12 shifted by 3 bytes into R1 
    MOV     R0, #0x1
	AND     R2, R1, R0
    CMP     R2, #0                      // Loop again if not 0  
    BNE     LOOP 

    POP     {R0-R3} 
    MOV     PC, LR                      // return 

// R0 is x, R1 is y, R2 is colour 
PLOT_PIXEL:
    PUSH    {R0-R3}
    LDR     R3, [R12, #0x4]             // R3 will be back buffer 
    LSL     R0, #1                      // shift by 1 
    LSL     R1, #10                     // shift by 10 
    ADD     R3, R0                      
    ADD     R3, R1          
    STRH    R2, [R3]                    // store half word since colour is 16 
    POP     {R0-R3}
    MOV     PC, LR                      // return 

CLEAR_SCREEN: 
    MOV     R2, #0                      // Colour black 
    MOV     R0, #0                      // X
    MOV     R1, #0                      // Y
    PUSH    {LR} 
    INNER_LOOP:
        PUSH    {R0-R3}
        BL      PLOT_PIXEL          // draw 
        POP     {R0-R3}
        ADD     R1, #1              // increment y 
        CMP     R1, #240            // done inner loop if y > 240
        BNE     INNER_LOOP 
        MOV     R1, #0              // else reset y 

        ADD     R0, #1              // increment x
        CMP     R0, #320                // Done if x > 320 
        BNE     INNER_LOOP  

        POP {LR} 
        MOV PC, LR 

// R0 is x0, R1 is x1, R2 is y, R3 is colour  
DRAW_LINE:
    PUSH    {R4, LR} 
    MOV     R4, R1 
    PUSH    {R1}                        // swap R1 and R2 
    MOV     R1, R2 
    PUSH    {R2} 
    MOV     R2, R3                      // swap R2 and R3
    PUSH    {R3} 
DRAW_LINE_LOOP: 
    BL      PLOT_PIXEL                  // draw 
    CMP     R0, R4                      // return if r0 greater than r1 
    ADDNE   R0, #1 
    BNE     DRAW_LINE_LOOP
    POP     {R1-R4, LR} 
    MOV     PC, LR 

MAIN:
    PUSH    {R0, R1, R2, R3}            // clear screen 
    BL      CLEAR_SCREEN
    POP     {R0, R1, R2, R3} 

    MOV     R0, #100                    // X0 
    MOV     R1, #220                    // X1
    MOV     R2, #0                      // Y
    LDR     R3, =0xFFFF                 // Colour 
    MOV     R4, #1                      // Y_dir

    MAIN_LOOP: 
        BL      WAIT_FOR_VSYNC          // wait 
        MOV     R3, #0                  // draw black 
        BL      DRAW_LINE
        MOV     R0, #100                // X0 
        MOV     R1, #220                // X1
        ADD     R2, R4                  // increment y 

        CMP     R2, #0                  // change y direction if y is 0 or 239
        MVNEQ   R4, R4 
        ADDEQ   R4, #1 
        BEQ     SKIP 
        CMP     R2, #239 
        MVNEQ   R4, R4 
        ADDEQ   R4, #1
        BEQ     SKIP 

        SKIP:        
        LDR     R3, =0xFFFF              // draw white line 
        BL      DRAW_LINE
        MOV     R0, #100                 // X0 
        MOV     R1, #220                 // X1
        B       MAIN_LOOP 

END:
    B       END 

.end 
