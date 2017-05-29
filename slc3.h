/*
TCSS372 - Computer Architecture
Project LC3 
Group Members: 
Shaun Coleman
Phansa Chaonpoj
Joshua Meigs
*/
#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>			/* ncurses.h includes stdio.h */  

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

// Struct representing the condition codes
typedef struct {
  Register n : 1;
  Register z : 1;
  Register p: 1;
} CC_s;

// Struct representing the FBUFF
typedef struct {
    Register ir;
    Register pc;
} FBUFF_s;

// Struct representing the DBUFF
typedef struct {
    Register op : 4;
    Register dr;
	Register imb: 1;
    Register opn1;
    Register opn2;
    Register pc;
} DBUFF_s;

// Struct representing the EBUFF and MBUFF
typedef struct {
    Register op : 4;
    Register dr;
    Register imb: 1;
    Register result;
    Register pc;
} EMBUFF_s;


// Struct representing the prefetch queue
typedef struct {
    short index;
    Register instructs[MAX_PREFETCH];
    Register nextPC;
} PREFETCH_s;

// Struct representing the cpu
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

// Struct representing the debugger window
typedef struct {
    WINDOW *mainWin;
    WINDOW *ioWin;
    unsigned short memAddress;
    unsigned short maxY, maxX;
    unsigned short ioY, ioX;
    BREAKPOINT_p breakpoints;
} DEBUG_WIN_s;

typedef DEBUG_WIN_s* DEBUG_WIN_p;

// Update Opcodes in cpu based on the sign (Negative/Zero/Positive) of val
void updateConCodes(CPU_p, short);

// Evaluates the trapVector and performs the appropriate action
// Returns 1 for a halt command, 0 otherwise
bool trap(CPU_p, DEBUG_WIN_p, Register);

// Loads a hex file into the memory of the controller
char loadFileIntoMemory(FILE *, CPU_p);

char loadFile(char *, CPU_p);

// Outputs the selected section of memory to the specified file
void saveToFile(char *, char *, char *);

// Prompts user to override a file if it exists
// Cancels save if any character other than Y or y is entered
void promptSaveToFile(CPU_p, char *, char *, char *, DEBUG_WIN_p);

// Prompts a user for a file to load
// if the file does not exists reports an error
char load(CPU_p, unsigned short *, DEBUG_WIN_p);

// Initializes all stall counters to zero for the pipeline
void initStall(CPU_p);

// Pushes a removed breakpoint to the end of the array
void updateBreakpoints(DEBUG_WIN_p);

// Checks to see if a specified address has a breakpoint and returns index
// or BREAKPOINT_NOT_FOUND
int breakpointsContains(DEBUG_WIN_p, int);

// Checks if the specified PC is a breakpoint and returns a boolean true if it is and false otherwise
bool breakpointsReached(DEBUG_WIN_p, Register);

// Adds specified breakpoint if not present, or remove specified breakpoint if present.
void modifyBreakPoint(DEBUG_WIN_p, BREAKPOINT_p, char*);

// Initializes all breakpoints as empty
void initBreakPoints(BREAKPOINT_p);

// Prompts user for the starting/ending address in memory and
// a file to save the selected range of memory values in 
void save(CPU_p, DEBUG_WIN_p);

// Prompts users for a starting address for memory display and updates the UI
void displayMemory(CPU_p, DEBUG_WIN_p, char);

// Prompts the user for an address to edit and a value to place in memory at that address
// Updates memory based on user input, or returns without editing if invalid values are
// entered.
void edit(CPU_p, DEBUG_WIN_p, char, unsigned short *);

// Prompts the user for and address to add or remove from breakpoints.
void breakPoint(CPU_p, DEBUG_WIN_p, BREAKPOINT_p, char);

// Checks the specified buffer for OP codes that will update a register
// returns true if found, false otherwise
// used for forwarding CC
bool containsRegisterUpdate(EMBUFF_s);

// Returns a BEN signal based on the the OP currently in the memory step
bool forwardCCValue(CPU_p);

// Returns the BEN signal based on a forwarded value if needed
// Otherwise uses the condition codes stored in the cpu
int checkBEN(CPU_p);

// Flushes the pipeline before the execute phase
// used for branch taken conditions
void flushPipeline(CPU_p);

// Flushes the whole pipeline and sets all flags and counters back
// to their original values
void initPipeline(CPU_p);

// STORE/WRITE BACK - Writes to registers if required by the current OP
void storeStep(CPU_p);

// MEMORY - Implements the memory step and simulated memory access times
// Memory access times simulated by stalling for 10 cycles before resuming
void memoryStep(CPU_p, bool);

// Calculates the cycles to stall to clear instructions
// ahead of a trap before it executes in the Execute step
void calcStallForTraps(CPU_p);

// EXECUTE - implements the execute step
int executeStep(CPU_p, DEBUG_WIN_p);

// Checks the specified buffer for RAW hazards based on the passed src register
bool containsHazard(EMBUFF_s, Register);

// Returns the results from the alu for ADD/AND/NOT
// Stalls the ID step for LD/LDI/LDR to wait for the correct
// value to be read from memory
Register fowardExecuteData(CPU_p);

// Gets register value from reg while using data fowarding to deal with RAW hazards
Register getRegisterValue(CPU_p, Register);

// Checks for RAW hazards and stalls based on when a hazard is found in the pipeline
short checkRawHazards(CPU_p, Register);

// Checks for RAW hazards and stalls based on when a hazard is found in the pipeline
short checkRawHazardsTwoSrcs(CPU_p, Register, Register);

// DECODE - implements the Decode step and gets values out of registers
void decodeStep(CPU_p);

// Checks for stall signals and handles the stall counter before
// executing the memory step
void memHandler(CPU_p);

// Checks for stall signals and handles the stall counter before
// executing the execute step
int executeHandler(CPU_p, DEBUG_WIN_p);

// Checks for stall signals and handles the stall counter before
// executing the decode step
void decodeHandler(CPU_p);

// Checks for stall signals and handles the stall counter before
// executing the fetch step
void fetchHandler(CPU_p);

// Searches the pipeline and returns the pc for the next instruction that will complete
Register getNextInstrToFinish(CPU_p);

// Main controller for the cpu
bool controller_pipelined(CPU_p, DEBUG_WIN_p, int, BREAKPOINT_p);

// Main debug monitor
// continues to loop, prompting user for menu items until exit is selected
// updates the UI after returning from the controller_pipelined method
int monitor(CPU_p, DEBUG_WIN_p);