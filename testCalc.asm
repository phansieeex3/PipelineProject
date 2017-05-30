.ORIG 		x3000

		LD R0, StackBaseLocation
		ADD R6, R0, #0
		ADD R6,R6,#-1
		LD R0, HeaderInfoLocation
		PUTS
		LD R0, NewlineChar
		OUT
		LEA R0,PromptMsg
		PUTS
		GETC
		OUT

Test 		LD R1,NegX 
		ADD R1,R1,R0
		BRz Exit
		LD R1, NegC 
		ADD R1,R1 , R0
		BRz OpClear 
		LD R1, NegPlus 
		ADD R1, R1,R0
		BRnp NotAdd
		JSR OpAdd
		LD R2, ErrorValue
		ADD R3, R0, R2
		BRz NewCommand
		JSR PUSH
		JSR OpDisplay
		BRnzp NewCommand

NotAdd		LD R1, NegMult 
		ADD R1, R1, R0
		BRnp NotMult
		JSR OpMult
		LD R2, ErrorValue
		ADD R3, R0, R2
		BRz NewCommand
		JSR PUSH
		JSR OpDisplay
		BRnzp NewCommand

NotMult 	LD R1, NegExl 
		ADD R1,R1,R0
		BRnp NotNeg
		JSR OpNeg
		JSR PUSH
		JSR OpDisplay
		BRnzp NewCommand

NotNeg		LD R1,NegD
		ADD R1,R1,R0
		BRnp NotDisplay
		JSR OpDisplay
                BRnzp NewCommand

NotDisplay 	LD R1, NegMinus
		ADD R1, R0, R1
		BRnp NotMinus
		JSR OpSub
		LD R1, ErrorValue
		ADD R1, R1, R0
		BRz NewCommand
		JSR PUSH
		JSR OpDisplay
		BRnzp NewCommand

NewCommand 	LEA R0,PromptMsg
		PUTS
		GETC
		OUT
		BRnzp Test
		Exit HALT

NotMinus	LD R1, NegMod
		ADD R1, R1, R0
		BRnp NotMod
		JSR OpMod
		LD R2, ErrorValue
		ADD R3, R0, R2
		BRz NewCommand
		JSR PUSH
		JSR OpDisplay
		BRnzp NewCommand

NotMod		LD R1, NegS
		ADD R1, R1, R0
		BRnp NotSave
		JSR POP
		ADD R4, R0, #0
		LD R2, ErrorValue
		ADD R3, R0, R2
		BRz NewCommand
		STI R4, SavedValuePointer 
		LD R0, NewlineChar
		OUT
		BRnzp NewCommand




NotSave		LD R1, NegL
		ADD R1, R1, R0
		BRnp NotLoad
		LDI R0, SavedValuePointer 
		LD R3, ErrorValue
		ADD R4, R3, R0
		BRz DisplayLoadError
		JSR PUSH
		ADD R4, R3, #0
		BRp NewCommand
		JSR POP
		LD R0, NewlineChar
		OUT
		BRnzp NewCommand


DisplayLoadError LEA R0, LoadValueErrorMessage
		 PUTS
		 LD R0, NewlineChar
		 OUT
		 BRnzp NewCommand
		

NotLoad		LD R1, ASCIIBUFFLocation
		LD R2,MaxDigits
		ValueLoop ADD R3,R0,xFFF6 
		BRz Goodlnput
		ADD R2,R2,#0
		BRz TooLargeInput
		ADD R2,R2,#-1
		STR R0,R1,#0 
		ADD R1,R1,#1
		GETC
		OUT 
		BRnzp ValueLoop

Goodlnput     	LD R2, ASCIIBUFFLocation
        	NOT R2,R2
        	ADD R2,R2,#1
        	ADD R0,R1,R2
                JSR PUSH
                LD R0, ASCIIBUFFLocation
                JSR PUSH 
        	JSR ASCIItoBinary
        	JSR PUSH
        	BRnzp NewCommand

TooLargeInput 	GETC 
		OUT
		ADD R3,R0,xFFF6
		BRnp TooLargeInput
		LEA R0, TooManyDigits
		PUTS
		LD R0, NewlineChar
		OUT
		BRnzp NewCommand

TooManyDigits 	  .STRINGZ "Too many digits"

		  
MaxDigits 	  .FILL x0003  
PromptMsg 	  .STRINGZ "Enter a command: "
NegX 		  .FILL XFFA8
NegC 		  .FILL xFFBD
NegPlus 	  .FILL XFFD5
NegMinus	  .FILL XFFD3
NegMult 	  .FILL XFFD6
NegD 		  .FILL xFFBC  
NegExl     	  .FILL xFFDF
NegMod		  .FILL xFFDB
NegL		  .FILL XFFB4
NegS		  .FILL	XFFAD

StackBaseLocation .FILL StackBase
SavedValuePointer .FILL SavedValue
LoadValueErrorMessage .STRINGZ "No Value Saved"
HeaderInfoLocation	.FILL HeaderInfo  

OpAdd		AND R4, R4, #0
		ADD R4, R7, R4 // R7 saved in R4
		JSR POP
		LD R3, ErrorValue
		ADD R5, R0, R3
		BRz AddRestoreR7	
		ADD R1, R0, #0
		JSR POP
		ADD R5, R0, R3
		BRz AddRestoreR7	
		ADD R3, R0, R1
		AND R0, R0, #0
		ADD R0, R4, R0
		JSR PUSH
		ADD R0, R3, #0
		JSR PUSH
		JSR RangeCheck
		ADD R3, R0, #0
		JSR POP
		ADD R4, R0, #0
		JSR POP
		ADD R7, R0, #0 
		ADD R3, R3, #0
		BRp ExitAdd
		ADD R0, R4, #0
		RET
AddRestoreR7	ADD R7, R4, #0
ExitAdd 	RET

NewlineChar	.FILL x000A

OpSub		AND R4, R4, #0
		ADD R4, R7, R4 // R7 saved in R4
		JSR POP
		LD R3, ErrorValue
		ADD R5, R0, R3
		BRz SubRestoreR7	
		ADD R1, R0, #0
		JSR POP
		ADD R5, R0, R3
		BRz SubRestoreR7	
		NOT R1, R1
		ADD R1, R1, #1
		ADD R3, R0, R1
		AND R0, R0, #0
		ADD R0, R4, R0
		JSR PUSH
		ADD R0, R3, #0
		JSR PUSH
		JSR RangeCheck
		ADD R3, R0, #0
		JSR POP
		ADD R4, R0, #0
		JSR POP
		ADD R7, R0, #0 
		ADD R3, R3, #0
		BRp ExitSub
		ADD R0, R4, #0
		RET
SubRestoreR7	ADD R7, R4, #0
ExitSub		RET

ErrorValue      .FILL #-1000

OpClear	        LEA R6,StackBase
		ADD R6,R6,#-1
		BRnzp NewCommand

OpDisplay 	JSR POP
		LD R3, ErrorValue
		ADD R5, R0, R3
		BRz NewCommand
		JSR BinarytoASCII
		LD R0, NewlineChar
		OUT
		LEA R0, ASCIIBUFF
		PUTS
		ADD R6,R6,#-1
		LD R0, NewlineChar
		OUT
		BRnzp NewCommand



OpMult		AND R3,R3,#0
		ADD R2, R7, #0 // R7 saved in R2
		JSR POP 
		LD R4, ErrorValue
		ADD R5, R0, R4 
		BRz MultRestoreR7
		ADD R1,R0,#0 
		JSR POP
		ADD R5,R0, R4 
		BRz MultRestoreR7
		ADD R4, R2, #0 //R7 Saved in R4
		ADD R2,R0,#0 
		BRzp PosMultiplier
		ADD R3,R3,#1 
		NOT R2, R2
		ADD R2,R2,#1 

PosMultiplier   AND R0,R0,#0 
		ADD R2,R2,#0
		BRz ExitMult

MultLoop        ADD R0,R0,R1 // The Actual Multiplication
		ADD R2,R2,#-1 
		BRp MultLoop

		ADD R1, R0, #0 //Save Result
		ADD R0, R4, #0 // Push Return(R7) onto stack
		JSR PUSH
		ADD R0, R1, #0
		JSR PUSH
		JSR RangeCheck
		ADD R4, R0, #0 //Store result of Range Check
		JSR POP
		ADD R2, R0, #0
		JSR POP
		ADD R7, R0, #0
		ADD R0, R2, #0
		ADD R4, R4, #0
		BRp ExitMult

		ADD R3,R3,#0
		BRz ExitMult	// Checks to see if the sign was negitive

		NOT R0, R0 //Is a Negitive sign
		ADD R0,R0,#1
		ADD R7, R1, #0 
		BRnzp ExitMult
MultRestoreR7	ADD R7, R2, #0
ExitMult	RET



; Calculates MOD(a,m)
; Returns a%m or an error code in R0
; Consumes 2 operands from the stack
OpMod 	ADD R4, R7, #0
    	LD R3, ErrorValue
     	JSR POP
     	ADD R5, R0, R3
     	BRz EXITMOD
    	ADD R1, R0, #0
    	JSR POP
     	ADD R5, R0, R3
     	BRz EXITMOD

	NOT R1, R1
     	ADD R1, R1, #1      ; Negate R1 (m)

     	LOOP               ; Continue a - m until result is negative
        	ADD R0, R0, R1
     	BRzp LOOP

     	NOT R1, R1
     	ADD R1, R1, #1      ; Reverse negation of R1 (m)
     	ADD R0, R0, R1     ; Reverse last subtraction
EXITMOD	ADD R7, R4, #0
 	RET


OpNeg 		ADD R4, R7, #0 // R7 saved in R4
		JSR POP
		LD R3, ErrorValue
		ADD R5,R0, R3
		BRz NegExit
		NOT R0, R0
		ADD R0 ,R0, #1
		ADD R7, R4, #0 // R7 is Restored
		RET

NegExit 	ADD R7, R4, #0 // R7 is Restored
		RET 




ASCIIBUFFLocation .FILL ASCIIBUFF

FailureValue    .FILL #1000
	





ASCIItoBinary   ADD R4, R7, #0    ; Store R7
        	JSR POP         ; Get Buffer Address
        	ADD R2, R0, #0  ; Set to R2
                JSR POP         ; Get Input Size
                ADD R7, R4, #0  ; Restore R7
                ADD R1, R0, #0  ; Set Input Size to R1
        	AND R0, R0, #0  ; Set Return Value to 0
        	ADD R1, R1, #0  ; Check R1's condition
        	BRz DoneAtoB    ; Return a 0 if size is 0

        	LD R3, NegASCIIOffest
        	ADD R2, R2, R1
		ADD R2, R2, #-1
		LDR R4, R2, #0
		ADD R4, R4, R3
		ADD R0, R0, R4
		ADD R1, R1, #-1
		BRz DoneAtoB
		
		ADD R2, R2, #-1
		LDR R4, R2, #0
		ADD R4, R4, R3
		LEA R5 LookUp10
		ADD R5, R5, R4
		LDR R4, R5, #0
		ADD R0, R0, R4
		ADD R1, R1, #-1
		BRz DoneAtoB
		
		ADD R2, R2, #-1
		LDR R4, R2, #0
		ADD R4, R4, R3
		LEA R5, LookUp100
		ADD R5, R5, R4
		LDR R4, R5, #0
		ADD R0, R0, R4

DoneAtoB 	RET


BinarytoASCII   LEA R1,ASCIIBUFF
		ADD R0,R0,#0 
		BRn NegSign
		LD R2,ASCIIplus
		STR R2,R1,#0
		BRnzp Begin100

NegSign		LD R2,ASCIIminus
		STR R2,R1,#0
		NOT R0,R0
		ADD R0,R0,#1

Begin100 	LD R2,ASCIIoffset
		LD R3,Neg100 
Loop100 	ADD R0,R0,R3
		BRn End100

		ADD R2,R2,#1
		BRnzp Loop100

End100 		STR R2,R1,#1
		LD R3,Pos100
		ADD R0,R0,R3
		LD R2, ASCIIoffset

Begin10 	LD R3,Neg10
Loop10 		ADD R0,R0,R3
		BRn End10

		ADD R2,R2,#1
		BRnzp Loop10
End10 		STR R2,R1,#2
		ADD R0,R0,#10
Beginl 		LD R2,ASCIIoffset
		ADD R2,R2,R0
		STR R2,R1,#3
		RET

StackMax 	.BLKW 10
StackBase 	.FILL x0000
StackClose 	.FILL #0

NegASCIIOffest .FILL Xffd0
ASCIIBUFF      .BLKW 4
LookUp10       .FILL #0
               .FILL #10
               .FILL #20
	       .FILL #30
               .FILL #40
               .FILL #50
               .FILL #60
               .FILL #70
               .FILL #80
               .FILL #90
LookUp100      .FILL #0
	       .FILL #100
               .FILL #200
               .FILL #300
               .FILL #400
               .FILL #500
               .FILL #600
               .FILL #700
               .FILL #800
               .FILL #900

RangeCheck     	ST R3, SAVER3
		ADD R1, R7, #0
		JSR POP
	       	LD R5, Neg999
		ADD R4, R0, R5
		BRp BadRange
		LD R5, Pos999
		ADD R4, R0, R5
		BRn BadRange
		JSR PUSH
		AND R0, R0, #0 
		ADD R7, R1, #0
		LD R3, SAVER3
		RET
BadRange 	JSR PUSH 
		LEA R0,RangeErrorMsg
		TRAP x22 
		ADD R7, R1, #0
		AND R0,R0,#0
		ADD R0,R0,#1
		LD R3, SAVER3
		RET

UnderflowMsg    .STRINGZ " Error: Too Few Values on the Stack."
RangeSave       .FILL x0200
Neg999		.FILL #-999
Pos999		.FILL #999
RangeErrorMsg   .FILL x000A
		.STRINGZ "Error: Number is out of range." 
SAVER3 		.FILL x4000
Save 		.FILL x0300



OverflowMsg     .STRINGZ "Error: Stack is Full."
ASCIIplus       .FILL X002B
ASCIIminus      .FILL X002D
ASCIIoffset     .FILL X0030
Neg100          .FILL XFF9C
Pos100          .FILL X0064
Neg10           .FILL XFFF6

SavePush        .FILL X0000
Save1 		.FILL X0000
NewlineLocation .FILL NewlineChar
FailureValueLocation .Fill FailureValue

SavedValue .FILL #1000

HeaderInfo .STRINGZ "Commands: Number or (+,-,*,!,%,D,S,L)"

PUSH		ST R1,Save1
		LEA R1,StackMax
		NOT R1, R1
		ADD R1,R1,#1
		ADD R1,R1,R6
		BRz Overflow
		ADD R6,R6,#-1
		STR R0,R6,#0
		BRnzp Success_exit
		Overflow ST R7, SavePush 
		LEA R0,OverflowMsg
		PUTS
		LDI R0, NewlineLocation
		OUT
		LD R7, SavePush 
		LD R1, Save1
		LDI R0,  FailureValueLocation 
		AND R0,R0,#0
		RET

Success_exit    LD R1,Save1		
		RET

POP		LEA R0,StackBase
		NOT R0, R0
		ADD R0,R0,#2
		ADD R0,R0,R6
		BRz Underflow
		LDR R0,R6,#0 
		ADD R6,R6,#1
		RET
Underflow 	ST R7, Save
		LDI R0, NewlineLocation
		OUT
		LEA R0, UnderflowMsg
		PUTS
		LDI R0, NewlineLocation 
		OUT 
		LD R7, Save 
		LDI R0,  FailureValueLocation 
		RET

.END