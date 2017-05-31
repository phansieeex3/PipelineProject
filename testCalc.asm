.ORIG 		x3000
	;      
	;	Main
	;

;Load stack pointer to R6, Print Header and prompt for input

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


;Checks to see if the character that was typed was a 'X', 'C', or a '+'.
;If it was a 'X', go to Exit and leave the program, If it was 'C', Call
;the OpClear subroutine. If it was a '+', then call the OpAdd sub-routine
;Otherwise, go to NotAdd

Test 		LD R1,NegX 
		ADD R1,R1,R0
		BRz Exit		;Check if the character is a 'X'
		LD R1, NegC 
		ADD R1,R1 , R0
		BRz OpClear 		;Check if the character is a 'C'
		LD R1, NegPlus 
		ADD R1, R1,R0
		BRnp NotAdd		;Check if the character is a '+'
		JSR OpAdd		;Call the OpAdd Subroutine
		LD R2, ErrorValue
		ADD R3, R0, R2
		BRz NewCommand		;Check if the OpAdd subroutine was succussful
		JSR PUSH		;Push new addition value onto the stack
		JSR OpDisplay		;Display the new value
		BRnzp NewCommand

;Checks to see if the character that was typed was a '*'. If it was a '*',
;call the OpMult sub-routine, otherwise go to NotNeg

NotAdd		LD R1, NegMult 
		ADD R1, R1, R0
		BRnp NotMult		;Check to see if the character is a '*'
		JSR OpMult		
		LD R2, ErrorValue
		ADD R3, R0, R2
		BRz NewCommand		;Check if the sub-routine was successful
		JSR PUSH		;Push new product onto the stack
		JSR OpDisplay		;Display new product
		BRnzp NewCommand

;Checks to see if the character that was typed was a '!'. If it was a '!',
;call the OpNeg sub-routine, otherwise go to NotNeg

NotMult 	LD R1, NegExl 
		ADD R1,R1,R0
		BRnp NotNeg		;Check to see if the character is a '!'
		JSR OpNeg
		JSR PUSH		;Push Negated Value onto the stack
		JSR OpDisplay		;Display new value
		BRnzp NewCommand

;Checks to see if the character that was typed was a 'D'. If it was a 'D',
;call the OpDisplay sub-routine, otherwise go to NotDisplay

NotNeg		LD R1,NegD
		ADD R1,R1,R0
		BRnp NotDisplay		;Check if the character is a 'D'
		JSR OpDisplay		;Call the OpDisplay sub-routine
                BRnzp NewCommand

;Checks to see if the character that was typed was a '-' Or a Minus Sign.
;If it was a '-', If it was, call the OpSub sub-routine, otherwise 
;go to NotMinus

NotDisplay 	LD R1, NegMinus
		ADD R1, R0, R1
		BRnp NotMinus		;Check to see if the character is a '-'
		JSR OpSub		;Call OpSub
		LD R1, ErrorValue
		ADD R1, R1, R0
		BRz NewCommand		;Check to see if the Operation was successful
		JSR PUSH		;Push new value onto the stack
		JSR OpDisplay		;Display new Value
		BRnzp NewCommand

;Prompt for a new command and enter main loop
NewCommand 	LEA R0,PromptMsg
		PUTS
		GETC
		OUT
		BRnzp Test

;Exit the Program
Exit		 HALT

;Checks to see if the character that was typed was a '%' Or a Modulo.
;If it was a '%', If it was, call the Opmod sub-routine, otherwise 
;go to NotMod

NotMinus	LD R1, NegMod
		ADD R1, R1, R0
		BRnp NotMod		;Checks to see if '%' was pressed
		JSR OpMod		;Call OpMod
		LD R2, ErrorValue
		ADD R3, R0, R2
		BRz NewCommand		;Check to see if it was successful
		JSR PUSH		;Push new value onto the stack
		JSR OpDisplay		;Display new Value
		BRnzp NewCommand

;Checks to see if the character that was typed was a 'S' Or a save.
;If it was a 'S', then save the current top of the stack to our variable
;Else go to NotSave

NotMod		LD R1, NegS
		ADD R1, R1, R0
		BRnp NotSave		  ;Check if 'S' Was pressed
		JSR POP
		ADD R4, R0, #0
		LD R2, ErrorValue	
		ADD R3, R0, R2
		BRz NewCommand		  ;Check if pop off the stack was successful
		STI R4, SavedValuePointer ;Save the Number into the Variable
		LD R0, NewlineChar
		OUT
		BRnzp NewCommand



;Check to see if the character that was typed was a 'L', for load.
;If it is not, then branch to NotLoad. If it is a load command, then
;we check if any value has been saved into the variable yet. We do this
;by checking against its default value, which is 1000(outside the valid range),
;and if it is its default, print the error and prompt for another input.
;Otherwise load the saved value.

NotSave		LD R1, NegL
		ADD R1, R1, R0
		BRnp NotLoad			;Check to see if it is a load command 
		LDI R0, SavedValuePointer 
		LD R3, ErrorValue
		ADD R4, R3, R0
		BRz DisplayLoadError		;Check if anything has been saved yet
		JSR PUSH			;Push the saved number onto the stack
		ADD R4, R3, R0
		LD R0, NewlineChar
		OUT
		ADD R4, R4, #0
		BRnp NewCommand			;Check if there was room on the stack			
		JSR POP				;Stack Full, so undo last push
		BRnzp NewCommand		

;If Nothing has been saved to our saved value, then display this error, then prompt
;for new input.

DisplayLoadError LEA R0, LoadValueErrorMessage
		 PUTS
		 LD R0, NewlineChar
		 OUT
		 BRnzp NewCommand
		
;	If the Value was none of the preset options, then it is to be treated as input
;	This will loop until and enter is pressed, or it detects that the input is too large.

NotLoad		LD R1, ASCIIBUFFLocation ;Load the Buffer location into R1
		LD R2,MaxDigits		 ;Load the maximum number of digits into R2
ValueLoop       ADD R3,R0,xFFF6	
		BRz Goodlnput		 ;Check if a return character(enter) was pressed
		ADD R2,R2,#0		
		BRz TooLargeInput	 ;Check if the input is too large at this point
		ADD R2,R2,#-1		 ;Decrement how many digits they have left to use
		STR R0,R1,#0 		 ;Store the character pressed into the buffer
		ADD R1,R1,#1		 ;Increment the Asciibuffer
		GETC			 ;Get next input
		OUT 	
		BRnzp ValueLoop

;	If the input was valid, then do the conversions and push onto the buffer. 
;	After pushes have been done, prompt for a new command
Goodlnput     	LD R2, ASCIIBUFFLocation ;Load the Bufferlocation into R2
        	NOT R2,R2
        	ADD R2,R2,#1
        	ADD R0,R1,R2
                JSR PUSH	
		LD R3, ErrorValue	 
		ADD R3, R3, R0
		BRz NewCommand			;Check if psuh was succesful
                LD R0, ASCIIBUFFLocation	;Load the Bufferlocation into R0
                JSR PUSH			;Push it onto the stack
		LD R3, ErrorValue
		ADD R3, R3, R0
		BRz NewCommand			;Check if push was succesful
        	JSR ASCIItoBinary		;Convert the Ascii to Binary
        	JSR PUSH			;Push the Conversion
        	BRnzp NewCommand		;Prompt a new command

;	If the input that was entered by the user was too large, for example '4444', we then display 
;	this error to the user then prompt for input

TooLargeInput 	GETC 
		OUT
		ADD R3,R0,xFFF6
		BRnp TooLargeInput
		LEA R0, TooManyDigits 
		PUTS			;Display Error
		LD R0, NewlineChar	
		OUT			;Display Newline
		BRnzp NewCommand	;Get a new command

;
;		End Of Main
;

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



; Calculates ADD(a,m)
; Returns a+m or an error code in R0
; Consumes 2 operands from the stack

OpAdd		AND R4, R4, #0
		ADD R4, R7, R4 		;Save R7 into R4
		JSR POP
		LD R3, ErrorValue
		ADD R5, R0, R3
		BRz AddRestoreR7	;Check if the Pop was succussful	
		ADD R1, R0, #0
		JSR POP
		ADD R5, R0, R3
		BRz AddRestoreR7	;Check if the Pop was succussful
		ADD R3, R0, R1		;Add the Two values
		AND R0, R0, #0
		ADD R0, R4, R0
		JSR PUSH
		ADD R0, R3, #0
		JSR PUSH
		JSR RangeCheck		;Check if the new addition is within acceptable range
		ADD R5, R0, #0
		JSR POP
		ADD R7, R0, #0		;Restore R7
		ADD R0, R5, #0		;Put the new Addition into R0 to return
		RET
AddRestoreR7	ADD R7, R4, #0		;Restore R7 from R4
ExitAdd 	RET			;Exit Add

NewlineChar	.FILL x000A


; Calculates Sub(a,m)
; Returns a-m or an error code in R0
; Consumes 2 operands from the stack

OpSub		AND R4, R4, #0
		ADD R4, R7, R4 		;Save R7 into R4
		JSR POP
		LD R3, ErrorValue
		ADD R5, R0, R3		
		BRz SubRestoreR7	;Check if pop was succusful
		ADD R1, R0, #0		;Save m into R1
		JSR POP			;Pop Value into a
		ADD R5, R0, R3
		BRz SubRestoreR7	;Check if pop was succusful	
		NOT R1, R1	
		ADD R1, R1, #1
		ADD R3, R0, R1		;Subtract m from a
		AND R0, R0, #0
		ADD R0, R4, R0
		JSR PUSH		;Push R7 into the stack
		ADD R0, R3, #0
		JSR PUSH		;Push the subtracted value into stack
		JSR RangeCheck		;Check if the Subtraction is within the acceptable range
		ADD R3, R0, #0		;load return value into R3
		JSR POP
		ADD R7, R0, #0  	;Restore R7
		ADD R0, R3, #0  	;Load Subtracted value into R0 to return
		RET
SubRestoreR7	ADD R7, R4, #0  	;Restore R7 from R4
ExitSub		RET

ErrorValue      .FILL #-1000

; Clears The stack

OpClear	        LEA R6,StackBase 	;Change stack pointer to be at its base
		ADD R6,R6,#-1
		LD R0, NewlineChar
		OUT
		BRnzp NewCommand

; Displays The top of the stack

OpDisplay 	JSR POP
		LD R3, ErrorValue
		ADD R5, R0, R3
		BRz NewCommand		;Check to see if the pop was successful
		JSR BinarytoASCII	
		LD R0, NewlineChar
		OUT
		LEA R0, ASCIIBUFF
		PUTS			;Print the Ascii version of the stack top
		ADD R6,R6,#-1		;Decrement back to the top of the stack
		LD R0, NewlineChar
		OUT
		BRnzp NewCommand


; Calculates Mult(a,m)
; Returns a*m or an error code in R0
; Consumes 2 operands from the stack

OpMult		AND R3,R3,#0
		ADD R2, R7, #0 		;R7 saved in R2
		JSR POP 
		LD R4, ErrorValue
		ADD R5, R0, R4 
		BRz MultRestoreR7
		ADD R1,R0,#0 
		JSR POP
		ADD R5,R0, R4 
		BRz MultRestoreR7
                ADD R4, R2, #0 		;R7 Saved in R4
		ADD R2, R0, #0 
		BRzp PosMultiplier
		ADD R3, R3, #1 
		NOT R2, R2
		ADD R2, R2, #1 

PosMultiplier   AND R0,R0,#0 
		ADD R2,R2,#0
		BRz ExitMult

MultLoop        ADD R0,R0,R1 		;The Actual Multiplication
		ADD R2,R2,#-1 
		BRp MultLoop

		ADD R1, R0, #0 		;Save Result
		ADD R0, R4, #0 		;Push Return(R7) onto stack
		JSR PUSH
		ADD R0, R1, #0
		JSR PUSH
		JSR RangeCheck		;Check to see if the production is within the acceptable range
		ADD R5, R0, #0 		;Store result of Range Check
		JSR POP
		ADD R7, R0, #0
		ADD R0, R5, #0
                LD R4, ErrorValue
		ADD R5, R0, R4
		BRz ExitMult

		ADD R3,R3,#0
		BRz ExitMult		;Checks to see if the sign was negitive

		NOT R0, R0 		;Is a Negitive sign
		ADD R0,R0,#1
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

; Calculates Negation(a)
; Returns !a or an error code in R0
; Consumes 1 operands from the stack

OpNeg 		ADD R4, R7, #0 		;R7 saved in R4
		JSR POP
		LD R3, ErrorValue
		ADD R5,R0, R3
		BRz NegExit		;Check to see if pop was successful
		NOT R0, R0
		ADD R0 ,R0, #1		;Negate R0 and return
		ADD R7, R4, #0 		;R7 is Restored
		RET

NegExit 	ADD R7, R4, #0 		;R7 is Restored
		RET 




ASCIIBUFFLocation .FILL ASCIIBUFF

FailureValue    .FILL #1000
	


; Converts the ascii from the input and stores it
; in the asciibuffer. 

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
		BRz DoneAtoB		;Value of the 'Ones' place has been calculated
		
		ADD R2, R2, #-1
		LDR R4, R2, #0
		ADD R4, R4, R3
		LEA R5 LookUp10
		ADD R5, R5, R4
		LDR R4, R5, #0
		ADD R0, R0, R4
		ADD R1, R1, #-1
		BRz DoneAtoB		;Value of the 'Tens' place has been calculated
		
		ADD R2, R2, #-1
		LDR R4, R2, #0
		ADD R4, R4, R3
		LEA R5, LookUp100
		ADD R5, R5, R4
		LDR R4, R5, #0
		ADD R0, R0, R4		;Value of the 'Hundreds' place has been calculated

DoneAtoB 	RET


; Converts the binary from the input and stores it
; in the asciibuffer.

BinarytoASCII   LEA R1,ASCIIBUFF
		ADD R0,R0,#0 
		BRn NegSign
		LD R2,ASCIIplus
		STR R2,R1,#0
		BRnzp Begin100

NegSign		LD R2,ASCIIminus	;Finds sign of the number
		STR R2,R1,#0
		NOT R0,R0
		ADD R0,R0,#1

Begin100 	LD R2,ASCIIoffset	;Start the working with the 'hundreds' place
		LD R3,Neg100 
Loop100 	ADD R0,R0,R3
		BRn End100

		ADD R2,R2,#1
		BRnzp Loop100

End100 		STR R2,R1,#1
		LD R3,Pos100
		ADD R0,R0,R3
		LD R2, ASCIIoffset	;Put result into the buffer

Begin10 	LD R3,Neg10		;Start the working with the 'tens' place
Loop10 		ADD R0,R0,R3
		BRn End10

		ADD R2,R2,#1
		BRnzp Loop10
End10 		STR R2,R1,#2		;Put result into the buffer
		ADD R0,R0,#10
Beginl 		LD R2,ASCIIoffset	;Start the working with the tens place
		ADD R2,R2,R0
		STR R2,R1,#3		;Put result into the buffer
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

; RangeCheck(a)
; Returns 'a' or an error code in R0

RangeCheck     	ST R3, SAVER3		;Store the value of R3 since it is edited
		ADD R1, R7, #0		;Save R7 in R1
		JSR POP			;Pop Value to check range for 
	       	LD R5, Neg999
		ADD R4, R0, R5
		BRp BadRange		;Check if the value is 1000 or higher
		LD R5, Pos999
		ADD R4, R0, R5
		BRn BadRange		;Check if the value is -1000 or lower
		ADD R7, R1, #0		;Restore R7
		LD R3, SAVER3		;Restore R3 to its previous value
		RET
BadRange 	LEA R0,RangeErrorMsg	;Was a bad Range, print error and return error code
		TRAP x22
                LDI R0, NewlineLocation
		OUT 
		ADD R7, R1, #0
                LDI R0, FailureValueLocation
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


; Push(a)
; Returns 'a' or an error code in R0

PUSH		ST R1,Save1
		LEA R1,StackMax
		NOT R1, R1
		ADD R1,R1,#1
		ADD R1,R1,R6
		BRz Overflow		;Check if there is a Overflow
		ADD R6,R6,#-1
		STR R0,R6,#0		;Push value onto the stack
		BRnzp Success_exit
Overflow        ST R7, SavePush 

		LEA R0,OverflowMsg	;There was an overflow, display error
		PUTS			;And return Error code
		LDI R0, NewlineLocation
		OUT
		LD R7, SavePush 
		LD R1, Save1
		LDI R0, FailureValueLocation 
		RET

Success_exit    LD R1,Save1		
		RET
; Pop(a)
; Returns 'a' or an error code in R0

POP		LEA R0,StackBase
		NOT R0, R0
		ADD R0,R0,#2
		ADD R0,R0,R6
		BRz Underflow			;Check to see if there will be a underflow
		LDR R0,R6,#0 
		ADD R6,R6,#1			;Pop value off the stack and put into R0
		RET
Underflow 	ST R7, Save			;There was an Underflow, display error and
		LDI R0, NewlineLocation		;Return Error code
		OUT
		LEA R0, UnderflowMsg
		PUTS
		LDI R0, NewlineLocation 
		OUT 
		LD R7, Save 
		LDI R0,  FailureValueLocation 
		RET

.END