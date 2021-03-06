/*
TCSS372 - Computer Architecture
Project LC3 
Group Members: 
Shaun Coleman
Phansa Chaonpoj
Joshua Meigs
*/
#pragma once
#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <stdlib.h>
#include <string.h> 

#define WIN_WIDTH 70
#define MAIN_WIN_HEIGHT 22
#define IO_WIN_HEIGHT 7
#define MAIN_WIN_START_POS 0, 0
#define IO_WIN_START_POS 21, 0

#define TITLE_Y_X 0, 14
#define REG_TITLE_Y_X 1, 2
#define MEM_TITLE_Y_X 1, 58

#define MAX_INPUT_SIZE 80
#define MAX_REG 8
#define MAX_MEM 16
#define REG_LABEL_X 2
#define REG_VAL_X 6
#define MEM_LABEL_X 56
#define REG_MEM_START_Y 2
#define MEM_VAL_X 63

#define CC_LABEL_Y_X 18,24

#define N_LABEL_Y_X 18,28
#define N_VAL_Y_X 18,30
#define Z_LABEL_Y_X 18,32
#define Z_VAL_Y_X 18,34
#define P_LABEL_Y_X 18,36
#define P_VAL_Y_X 18,38

#define MENU1_Y_X 19,1
#define MENU2_Y_X 20,1
#define PROMPT_DISPLAY_Y 20
#define PROMPT_DISPLAY_X 26

#define IO_TITLE_Y_X 0, 30
#define IO_START_Y 1
#define IO_START_X 1
#define NEWLINE 0x0A

#define ARROW_X 54
#define INPUT_LIMIT 43
#define DEFAULT_MEM_ADDRESS 0x3000
#define CP_WHITE_BLUE 1
#define TOTAL_WIN_HEIGHT 27

#define HEX_OUT_LABEL "x%.04X:"
#define HEX_OUT_FORMAT "x%.04X"
#define HEX_OUT_SINGLE "x%.01X"
#define REG_OUT_FORMAT "R%d:"

/******** New Constants for Pipeline display ***********/
#define FBUFF_LABEL_Y_X 2,16
#define DBUFF_LABEL_Y_X 6,16
#define EBUFF_LABEL_Y_X 10,16
#define MBUFF_LABEL_Y_X 14,16
#define STORE_LABEL_Y_X 17,23

#define FBUFF_PC_LBL_Y_X 2,26
#define FBUFF_IR_LBL_Y_X 2,32
#define FBUFF_NOP_LBL_Y_X 2,24
#define DBUFF_LBL_START_Y 6

#define EBUFF_LBL_START_Y 10

#define MBUFF_LBL_START_Y 14

#define OP_LBL_X 24
#define DR_LBL_X 28
#define OPN1_LBL_X 32
#define OPN2_LBL_X 38

#define FBUFF_BOARDER 1,22,3
#define DBUFF_BOARDER 5,22,3
#define EBUFF_BOARDER 9,22,3
#define MBUFF_BOARDER 13,22,3

#define NOP_WIDTH 6
#define FBUFF_WIDTH 15
#define TRAP_LABEL_WIDTH 13
#define THREE_LABEL_WIDTH 18
#define FOUR_LABEL_WIDTH 22

#define FBUFF_PC_VAL_Y_X     3,24
#define FBUFF_IR_VAL_Y_X     3,30

#define DBUFF_OP_VAL_Y_X     7,24
#define DBUFF_DR_VAL_Y_X     7,28
#define DBUFF_OPN1_VAL_Y_X   7,31
#define DBUFF_OPN2_VAL_Y_X   7,37
#define DBUFF_PC_VAL_Y_X     7,43
#define DBUFF_N_VAL_Y_X      7,28
#define DBUFF_Z_VAL_Y_X      7,29
#define DBUFF_P_VAL_Y_X      7,30

#define EBUFF_OP_VAL_Y_X     11,24
#define EBUFF_DR_VAL_Y_X     11,28
#define EBUFF_RESULT_VAL_Y_X 11,32
#define EBUFF_PC_VAL_Y_X     11,39
#define EBUFF_N_VAL_Y_X      11,28
#define EBUFF_Z_VAL_Y_X      11,29
#define EBUFF_P_VAL_Y_X      11,30

#define MBUFF_OP_VAL_Y_X     15,24
#define MBUFF_DR_VAL_Y_X     15,28
#define MBUFF_RESULT_VAL_Y_X 15,32
#define MBUFF_PC_VAL_Y_X     15,39
#define MBUFF_N_VAL_Y_X      15,28
#define MBUFF_Z_VAL_Y_X      15,29
#define MBUFF_P_VAL_Y_X      15,30

#define STORE_VAL_Y_X        17,30 

#define FBUFF_STALL_Y_X 3,18
#define DBUFF_STALL_Y_X 7,18
#define EBUFF_STALL_Y_X 11,18
#define MBUFF_STALL_Y_X 15,18

#define FBUFF_CLEAR_START 1,21
#define DBUFF_CLEAR_START 5,21
#define EBUFF_CLEAR_START 9,21
#define MBUFF_CLEAR_START 13,21

#define BOX_CLEAR_WIDTH 33
#define BOX_CLEAR_HEIGHT 4

#define HALF(x) ((x)/2)
#define MOD16(x) ((x)%16)
#define MOD10(x) ((x)%10)

#define DISPLAY_N(x) ((x&4)>>2)
#define DISPLAY_Z(x) ((x&2)>>1)
#define DISPLAY_P(x) (x&1)

#define STALL_STR " S "
#define STALL_ERS_STR "   "
#define ERASE_REG_DISPLAY "      "

// Print Main Window Labels
void printLabels(DEBUG_WIN_p);

// Print IO Window Labels
void printIoLabels(DEBUG_WIN_p);

// Update and reprint Memory labels and values
void updateMemory(DEBUG_WIN_p, unsigned short*, unsigned short);

// Update register values
void updateRegisterValues(DEBUG_WIN_p, CPU_p);

// Update buffer values
void updateBufferValues(DEBUG_WIN_p, CPU_p);

// Reprints only the boarder of the screen
void reprintBoarder(DEBUG_WIN_p);

// Clear and reprint both windows
void reprintScreen(DEBUG_WIN_p, CPU_p, unsigned short *, char);

// Update Screen Pos and Refresh
void updateScreen(DEBUG_WIN_p, CPU_p, unsigned short *, char);

// Initialize Struct and start ncurses
void initializeWindows(DEBUG_WIN_p);

// Free Windows and exit ncurses
void endWindows(DEBUG_WIN_p);

// Clears the prompt section of the screen
void clearPrompt(DEBUG_WIN_p);

// Prompts user with optional message
void promptUser(DEBUG_WIN_p, char*, char*);

// Displays a message in the prompt section using the standout attribute
void displayBoldMessage(DEBUG_WIN_p, char*);

// Writes a char to the IO window and updates the IO window's cursor pos
void writeCharToIOWin(DEBUG_WIN_p win, unsigned short);

// Clears the IO window and resets the IO window's cursor pos to default
void clearIOWin(DEBUG_WIN_p);

// Prints a square boarder with upper left corner being at y, x with height and width
void printBox(DEBUG_WIN_p, int, int, int, int);
