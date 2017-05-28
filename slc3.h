/*
TCSS372 - Computer Architecture
Project LC3 
Group Members: 
Shaun Coleman
Phansa Chaonpoj
Joshua Meigs
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// States
#define FETCH 0
#define DECODE 1
#define EVAL_ADDR 2
#define FETCH_OP 3
#define EXECUTE 4
#define STORE 5

#define REG_SIZE 8
#define IO_REG 0
#define RETURN_REG 7
#define SP_REG 6

// Opcodes
#define ADD 1
#define AND 5
#define NOT 9
#define TRAP 15
#define LD 2
#define ST 3
#define JMP 12 // and RET
#define BR 0
#define LEA 14
#define JSR 4 // and JSRR
#define STR 7
#define LDR 6
#define RSV 13 //reserve :D
#define LDI 10
#define STI 11

// Macros to get IR bit fields
#define OPCODE(instr)  (instr >> 12 & 0x000F)
#define DSTREG(instr)  (instr >> 9 & 0x0007)
#define SRCREG(instr)  (instr >> 6 & 0x0007)
#define SRCREG2(instr)  (instr & 0x0007)
#define IMMBIT(instr)  ((instr & 0x0020) >> 5)
#define JSRBIT11(instr) ((instr & 0x0800) >> 11)
#define NBIT(instr)  (instr & 0x0004) 
#define ZBIT(instr)  (instr & 0x0002)
#define PBIT(instr)  (instr & 0x0001)
#define NZPBITS(instr) ((instr & 0x0E00) >> 9)
 
// ZEXT trap
#define ZEXTTRAPVECT(instr) (instr & 0x00FF)

// SEXT
#define SEXTPCOFFSET9(instr) ((instr << 23 ) >> 23)
#define SEXTPCOFFSET11(instr) ((instr << 21 ) >> 21)
#define SEXTPCOFFSET6(instr)  ((instr << 26) >> 26) 
#define SEXTIMMVAL(instr)  ((instr << 27) >> 27) 

// Trap Vectors
#define GETCH 0x0020
#define OUT 0x0021
#define PUTS 0x0022
#define HALT 0x0025

// Constants
#define MAXMEM 65536 // TODO
#define HEX_MODE 16
#define EXPECTED_HEX_DIGITS 4
#define MENU_SELECTION 0
#define SINGLE_CHAR 1
#define NULL_CPU_POINTER -1
#define NULL_MEMORY_POINTER -2
#define MEM_CENTERED_OFFSET 7
#define MAXBREAK 4
#define NULL_BREAKPOINT 99999
#define BREAKPOINT_NOT_FOUND -1
#define FLUSH_PIPELINE 1
#define HALT_PROGRAM 2
#define N_FLAG 4
#define Z_FLAG 2
#define P_FLAG 1

// Menu Options
#define LOAD '1'
#define SAVE '2'
#define STEP '3'
#define RUN  '4'
#define DISPLAY_MEM '5'
#define EDIT '6'
#define BREAKPOINT '8'
#define EXIT '9'


#define PIPELINE_PHASES 5
#define P_STORE 4
#define P_MEM 3
#define P_EX 2
#define P_ID 1
#define P_IF 0

#define MEMORY_ACCESS_STALL_TIME 10

#define STEP_MODE 0
#define RUN_MODE 1
#define MICRO_STEP_MODE 2
#define MAX_PREFETCH 8

#define NOP 0x0000
#define NOP_IN_STORE 0xFFFF

typedef unsigned short Register;

typedef struct {
    int emptySpaces : MAXBREAK;

    int breakpointArr[MAXBREAK];
    //Register breakPointArr[MAXBREAK];
    
} BREAKPOINT_s;

typedef BREAKPOINT_s* BREAKPOINT_p;

typedef unsigned short Register;

// Condition Codes
typedef struct {
  Register n : 1;
  Register z : 1;
  Register p: 1;
} CC_s;

typedef struct {
    Register ir;
    Register pc;
} FBUFF_s;

typedef struct {
    Register op : 4;
    Register dr;
	Register imb: 1;
    Register opn1;
    Register opn2;
    Register pc;
} DBUFF_s;

typedef struct {
    Register op : 4;
    Register dr;
    Register imb: 1;
    Register result;
    Register pc;
} EMBUFF_s;

typedef struct {
    short index;
    Register instructs[MAX_PREFETCH];
    Register nextPC;
} PREFETCH_s;

typedef struct {
    // Array containing all data registers
	Register reg_file[REG_SIZE];
    // Special registers
	Register mar, mdr, ir, pc;
	// ALU registers
    Register alu_a, alu_b, alu_r;
    // Condition codes (part of the psr)
	CC_s conCodes;
    // dr used in the store step
	Register dr_store;
	// current op in the store step
    Register opInStore;
	// current value being stored
	Register valueInStore;
	// Structs for each pipeline buffer
	FBUFF_s fbuff;
    DBUFF_s dbuff;
    EMBUFF_s ebuff;
    EMBUFF_s mbuff;
	// Struct to handle the prefetch queue
    PREFETCH_s prefetch;
	// Stall counters for each pipeline phase
    short stalls[PIPELINE_PHASES];
	// flag used to signal indirect access mode for LDI/STI
    bool indirectFlag;
} CPU_s;

typedef CPU_s* CPU_p;




