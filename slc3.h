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

// Macros to get IR bit fields
#define OPCODE(instr)  (instr >> 12 & 0x000F)
#define DSTREG(instr)  (instr >> 9 & 0x0007)
#define SRCREG(instr)  (instr >> 6 & 0x0007)
#define SRCREG2(instr)  (instr & 0x0007)
#define IMMBIT(instr)  ((instr & 0x0020) >> 5) 
#define NBIT(instr)  ((instr & 0x0800) >> 11) 
#define ZBIT(instr)  ((instr & 0x0400) >> 10) 
#define PBIT(instr)  ((instr & 0x0200) >> 9) 
 
// ZEXT trap
#define ZEXTTRAPVECT(instr) (instr & 0x00FF)

// SEXT
#define SEXTPCOFFSET9(instr) ((instr << 23 ) >> 23)
#define SEXTPCOFFSET6(instr)  ((instr << 26) >> 26) 
#define SEXTIMMVAL(instr)  ((instr << 27) >> 27) 

// Trap Vectors
#define GETCH 0x0020
#define OUT 0x0021
#define PUTS 0x0022
#define HALT 0x0025

// Constants
#define MAXMEM 65536
#define HEX_MODE 16
#define EXPECTED_HEX_DIGITS 4
#define MENU_SELECTION 0
#define SINGLE_CHAR 1
#define NULL_CPU_POINTER -1
#define NULL_MEMORY_POINTER -2
#define MEM_CENTERED_OFFSET 7
#define MAXBREAK 4
#define NULL_BREAKPOINT -1

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

#define STEP_MODE 0
#define RUN_MODE 1
#define MAX_PREFETCH 8

#define STEP_MODE 0
#define RUN_MODE 1


typedef unsigned short Register;

typedef struct {
    short emptySpaces : MAXBREAK;
<<<<<<< HEAD
    short breakpointArr[MAXBREAK];
=======
    Register breakPointArr[MAXBREAK];
>>>>>>> origin/master
    
} BREAKPOINT_s;

typedef BREAKPOINT_s* BREAKPOINT_p;

<<<<<<< HEAD
typedef unsigned short Register;


=======
>>>>>>> origin/master
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
    Register op;
    Register dr;
    Register opn1;
    Register opn2;
    Register pc;
} DBUFF_s;

typedef struct {
    Register op;
    Register dr;
    Register result;
    Register pc;
} EMBUFF_s;

typedef struct {
    Register reg_file[REG_SIZE];
    Register mar, mdr, ir, pc;
    Register alu_a, alu_b, alu_r;
    CC_s conCodes;
    Register dr_store;
    FBUFF_s fbuff;
    DBUFF_s dbuff;
    EMBUFF_s ebuff;
    EMBUFF_s mbuff;
	short stalls[PIPELINE_PHASES];
} CPU_s;

typedef CPU_s* CPU_p;