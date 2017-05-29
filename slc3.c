/*
 TCSS372 - Computer Architecture
 Project - Simulated LC3 
 Group Members: 
 Shaun Coleman
 Phansa Chaonpoj
 Joshua Meigs
 */
#include "slc3.h"
#include "slc3_ui.h"

// TODO: FIX MAXMEM
// you can define a simple memory module here for this program
unsigned short memory[MAXMEM];

// Update Opcodes in cpu based on the sign (Negative/Zero/Positive) of val
void updateConCodes(CPU_p cpu, short val) {
    if (val < 0) {
        cpu->conCodes.n = true;
        cpu->conCodes.z = false;
        cpu->conCodes.p = false;
    } else if (val == 0) {
        cpu->conCodes.n = false;
        cpu->conCodes.z = true;
        cpu->conCodes.p = false;
    } else {
        cpu->conCodes.n = false;
        cpu->conCodes.z = false;
        cpu->conCodes.p = true;
    }
}

// Evaluates the trapVector and performs the appropriate action
// Returns 1 for a halt command, 0 otherwise
bool trap(CPU_p cpu, DEBUG_WIN_p win, Register vector) {
    int i = 0;
    switch (vector) {
		// Gets a single char and stores the value in R0
		case GETCH:
			cpu->reg_file[IO_REG] = mvwgetch(win->ioWin, win->ioY, win->ioX);
			break;
		// Writes a single char from R0 to the I/O Window
		case OUT:
			writeCharToIOWin(win, cpu->reg_file[IO_REG]);
			break;
		// Writes a string for the I/O Window (OUT until end of string is found)
		case PUTS:
			while (memory[cpu->reg_file[IO_REG] + i]) {
				writeCharToIOWin(win, memory[cpu->reg_file[IO_REG] + i]);
				i++;
			}
			break;
		// Displays a halt message and stops the program
		case HALT:
			displayBoldMessage(win, "Program Halted! Press any key...");
			return true;
		default:
			break;
    }

    wrefresh(win->ioWin);
    return false;
}

// Loads a hex file into the memory of the controller
char loadFileIntoMemory(FILE * theInFile, CPU_p cpu) {
    char * t;
    int temp;
    char line[MAX_INPUT_SIZE];
    fscanf(theInFile, "%s", &line);
    if (strlen(line) > EXPECTED_HEX_DIGITS
            || line[strspn(line, "0123456789abcdefABCDEF")] != 0) {
        return false;
    }
    cpu->pc = strtol(line, &t, HEX_MODE);
   
    cpu->prefetch.nextPC = cpu->pc;
    temp = cpu->pc;
    while (!feof(theInFile)) {
        fscanf(theInFile, "%s", &line);
        if (strlen(line) > EXPECTED_HEX_DIGITS
                || line[strspn(line, "0123456789abcdefABCDEF")] != 0) {
            return false;
        }
        memory[temp] = strtol(line, &t, HEX_MODE);
        temp++;
    }

    return true;
}

char loadFile(char * theInFile, CPU_p cpu) {
    FILE * inFile = fopen(theInFile, "r");

    if (!inFile) {      
        return false;
    }

    return loadFileIntoMemory(inFile, cpu);
}

// Outputs the selected section of memory to the specified file
void saveToFile(char * fileName, char * start, char * end) {

    FILE * file = fopen(fileName, "w+");

    char * temp1;
    char * temp2;
    int i = strtol(start, &temp1, HEX_MODE);

    for (; i <= strtol(end, &temp2, HEX_MODE); i++) {
        fprintf(file, "%04X\r\n", memory[i]);

    }

    fclose(file);

}

// Prompts user to override a file if it exists
// Cancels save if any character other than Y or y is entered
void promptSaveToFile(CPU_p cpu, char *input, char * start, char * end,
        DEBUG_WIN_p win) {

    char * filename = input; //saving input.

    //checking if file is empty.
    FILE * file;
    file = fopen(input, "r"); //file must exist or else it's NULL.

    char prompt[INPUT_LIMIT];

    do {
        fclose(file);
        clearPrompt(win);
        if (file != NULL) {
            promptUser(win,
                    "File exists, type Y to overwrite: ",
                    prompt);
            if (prompt[0] == 'Y' || prompt[0] == 'y') //ignore case
            {
                saveToFile(input, start, end);
                break;
            } else {
                displayBoldMessage(win, "Cancelling! Press any key...");
                return;
            }
        }

    } while (file != NULL);

    if (file == NULL) { //save to file.
        saveToFile(input, start, end);
    }

    fclose(file);
}

// Prompts a user for a file to load
// if the file does not exists reports an error
char load(CPU_p cpu, unsigned short * memory, DEBUG_WIN_p win) {
    char input[INPUT_LIMIT];
    char programLoaded;
    short orig = DEFAULT_MEM_ADDRESS;

    promptUser(win, "File Name: ", input);
    programLoaded = loadFile(input, cpu);
    if (programLoaded) {
        win->memAddress = cpu->pc;
        orig = cpu->pc;
        
        updateScreen(win, cpu, memory, programLoaded);
        printIoLabels(win);
        clearPrompt(win);
        clearIOWin(win);
        displayBoldMessage(win, "Load Successful!");
    } else {
        clearPrompt(win);
        displayBoldMessage(win,
                "Invalid File. Press any key...");
    }

    return programLoaded;
}

// Initializes all stall counters to zero for the pipeline
void initStall(CPU_p cpu) {
    cpu->stalls[P_IF] = 0;
    cpu->stalls[P_ID] = 0;
    cpu->stalls[P_EX] = 0;
    cpu->stalls[P_MEM] = 0;
    cpu->stalls[P_STORE] = 0;
}

// Pushes a removed breakpoint to the end of the array
void updateBreakpoints(DEBUG_WIN_p win) {
    int i;
    for(i = 0; i < MAXBREAK-1; i++) {
        if(win->breakpoints->breakpointArr[i] == NULL_BREAKPOINT) {
            win->breakpoints->breakpointArr[i] = win->breakpoints->breakpointArr[i+1];
            win->breakpoints->breakpointArr[i+1] = NULL_BREAKPOINT;
        }
    }
}

// Checks to see if a specified address has a breakpoint and returns index
// or BREAKPOINT_NOT_FOUND
int breakpointsContains(DEBUG_WIN_p win, int inputAddress) {
    int i;
    for(i = 0; i < MAXBREAK; i++) {
        if(win->breakpoints->breakpointArr[i] == inputAddress) {
            return i;
        }
    }
    return BREAKPOINT_NOT_FOUND;
}

// Checks if the specified PC is a breakpoint and returns a boolean true if it is and false otherwise
bool breakpointsReached(DEBUG_WIN_p win, Register pc) {
    int i;
    for(i = 0; i < MAXBREAK; i++) {
        if(win->breakpoints->breakpointArr[i] == pc) {
            return true;
        }
    }
    return false;
}

// Adds specified breakpoint if not present, or remove specified breakpoint if present.
void modifyBreakPoint(DEBUG_WIN_p win ,BREAKPOINT_p breakpoints, char* inputAddress) {
    char* temp;
    unsigned short breakpointToAdd = strtol(inputAddress, &temp, HEX_MODE);
    short breakpointLocation = breakpointsContains(win, breakpointToAdd);
    if(breakpointLocation != BREAKPOINT_NOT_FOUND) {
        win->breakpoints->breakpointArr[breakpointLocation] = NULL_BREAKPOINT; 
        displayBoldMessage(win, "Breakpoint Removed");
        updateBreakpoints(win);
        win->breakpoints->emptySpaces++;
        return;
    } else if(breakpoints->emptySpaces > 0) {
        win->breakpoints->breakpointArr[(MAXBREAK - breakpoints->emptySpaces)] = breakpointToAdd;
        displayBoldMessage(win, "Breakpoint Added");
        win->breakpoints->emptySpaces--;
        return;
    }

    displayBoldMessage(win, "Breakpoints full; cannot add more.");
}

// Initializes all breakpoints as empty
void initBreakPoints(BREAKPOINT_p breakpoints) {
    int i;
    for(i = 0; i < MAXBREAK; i++) {
        breakpoints->breakpointArr[i] = NULL_BREAKPOINT;
    }
    breakpoints->emptySpaces = MAXBREAK;
}

// Prompts user for the starting/ending address in memory and
// a file to save the selected range of memory values in
void save(CPU_p cpu, DEBUG_WIN_p win) {
    char startAddress[INPUT_LIMIT];
    char endAddress[INPUT_LIMIT];
    char input[INPUT_LIMIT];

    clearPrompt(win);
    promptUser(win, "Start Address: ", startAddress);
    clearPrompt(win);

    // Validate address
    if (strlen(startAddress) > EXPECTED_HEX_DIGITS
	        || strlen(startAddress) == 0
            || startAddress[strspn(startAddress,
                    "0123456789abcdefABCDEF")] != 0) {
        displayBoldMessage(win,
                "Invalid address. Press any key...");
		return;
    }


    // Prompt for end address (inclusive)
    clearPrompt(win);
    promptUser(win, "End Address: ", endAddress);
    clearPrompt(win);

    //Validate value
    if (strlen(endAddress) > EXPECTED_HEX_DIGITS
	        || strlen(endAddress) == 0
            || endAddress[strspn(endAddress,
                    "0123456789abcdefABCDEF")] != 0) {
        displayBoldMessage(win,
                "Invalid address. Press any key...");
		return;
    }

    promptUser(win, "File name: ", input);
	
	if (strlen(input) == 0) {
		displayBoldMessage(win, "Invalid file. Press any key...");
		return;
	}

    promptSaveToFile(cpu, input, startAddress, endAddress,
            win);

    displayBoldMessage(win,
            "Save Complete. Press any key...");
}

// Prompts users for a starting address for memory display and updates the UI
void displayMemory(CPU_p cpu, DEBUG_WIN_p win, char programLoaded) {

    int displayMemAddress = DEFAULT_MEM_ADDRESS;
    char inputAddress[INPUT_LIMIT];
    char *temp;

    clearPrompt(win);
    promptUser(win, "Starting Address: ", inputAddress);
    clearPrompt(win);

    if (strlen(inputAddress) > EXPECTED_HEX_DIGITS
	        || strlen(inputAddress) == 0
            || inputAddress[strspn(inputAddress,
                    "0123456789abcdefABCDEF")] != 0) {
        displayBoldMessage(win,
                "Invalid address. Press any key...");
    } else {
        displayMemAddress = strtol(inputAddress, &temp,
                HEX_MODE);
        win->memAddress = displayMemAddress;
        updateScreen(win, cpu, memory, programLoaded);
    }
}

// Prompts the user for an address to edit and a value to place in memory at that address
// Updates memory based on user input, or returns without editing if invalid values are
// entered.
void edit(CPU_p cpu, DEBUG_WIN_p win, char programLoaded, unsigned short * memory) {
    char *temp;
    char inputAddress[INPUT_LIMIT];
    char input[INPUT_LIMIT];
    int displayMemAddress = DEFAULT_MEM_ADDRESS;

    // Prompt for Address to edit
    clearPrompt(win);
    promptUser(win, "Address to Edit: ", inputAddress);
    clearPrompt(win);

    // Validate address
    if (strlen(inputAddress) > EXPECTED_HEX_DIGITS
	        || strlen(inputAddress) == 0
            || inputAddress[strspn(inputAddress,
                    "0123456789abcdefABCDEF")] != 0) {
        displayBoldMessage(win,
                "Invalid address. Press any key...");
		return;
    }

    // Prompt for new value to place into memory
    clearPrompt(win);
    promptUser(win, "New Value: ", input);
    clearPrompt(win);

    // Validate value
    if (strlen(input) > EXPECTED_HEX_DIGITS
	        || strlen(input) == 0
            || input[strspn(input, "0123456789abcdefABCDEF")]
                    != 0) {
        displayBoldMessage(win,
                "Invalid value. Press any key...");
		return;
    }

    // Update Memory
    displayMemAddress = strtol(inputAddress, &temp,
            HEX_MODE);
    memory[displayMemAddress] = strtol(input, &temp,
            HEX_MODE);

    // Update Memory display                                
    displayMemAddress =
            (displayMemAddress >= MEM_CENTERED_OFFSET) ?
                    displayMemAddress
                            - MEM_CENTERED_OFFSET :
                    0;
    win->memAddress = displayMemAddress;

    // Update screen to reflect changes
    updateScreen(win, cpu, memory, programLoaded);
}

// Prompts the user for and address to add or remove from breakpoints.
void breakPoint(CPU_p cpu, DEBUG_WIN_p win, BREAKPOINT_p breakpoints, char programLoaded)
{
    char inputAddress[INPUT_LIMIT];
    clearPrompt(win);
    promptUser(win, "Address: ", inputAddress);
    clearPrompt(win);

    // Validate address
    if (strlen(inputAddress) > EXPECTED_HEX_DIGITS
	        || strlen(inputAddress) == 0
            || inputAddress[strspn(inputAddress,
                    "0123456789abcdefABCDEF")] != 0) {
        displayBoldMessage(win,
                "Invalid address. Press any key...");
	    return;
    }
    
    modifyBreakPoint(win, breakpoints, inputAddress);

    // Update screen to reflect changes
    updateScreen(win, cpu, memory, programLoaded); 
}

// Checks the specified buffer for OP codes that will update a register
// returns true if found, false otherwise
// used for forwarding CC
bool containsRegisterUpdate(EMBUFF_s buffer) {
	switch (buffer.op) {
		case ADD:
		case AND:
		case NOT:
		case LD:
		case LDI:
		case LDR:
		case LEA:
		case JSR:
		case RSV:
			return true;
		default:
		    return false;
	}
}

// Returns a BEN signal based on the the OP currently in the memory step
bool forwardCCValue(CPU_p cpu) {
	int result = 0;
	short value;
	
	switch (cpu->mbuff.op) {
		case ADD:
		case AND:
		case NOT:
		case LD:
		case LDI:
		case LDR:
		case LEA:
		    value = cpu->mbuff.result;
			break;
		case JSR:
		    value = cpu->mbuff.pc+1;
			break;
		case RSV:
		    if (cpu->mbuff.imb) {
				value = cpu->reg_file[SP_REG]+1;
			} else {
				value = cpu->reg_file[SP_REG]-1;
			}
			break;
	}

    return (value < 0 && NBIT(cpu->dbuff.dr))
		   + (value == 0 && ZBIT(cpu->dbuff.dr))
           + (value > 0 && PBIT(cpu->dbuff.dr));
}

// Returns the BEN signal based on a forwarded value if needed
// Otherwise uses the condition codes stored in the cpu
int checkBEN(CPU_p cpu) {
    int result;
	
	if (containsRegisterUpdate(cpu->mbuff)) {
		result = forwardCCValue(cpu);
	} else {
		result = (cpu->conCodes.n && NBIT(cpu->dbuff.dr))
               + (cpu->conCodes.z && ZBIT(cpu->dbuff.dr))
               + (cpu->conCodes.p && PBIT(cpu->dbuff.dr));
	}

    return result;	
}

// Flushes the pipeline before the execute phase
// used for branch taken conditions
void flushPipeline(CPU_p cpu) {
    cpu->fbuff.pc = NOP;
    cpu->fbuff.ir = NOP;
    cpu->dbuff.op = NOP;
    cpu->dbuff.dr = NOP;
    cpu->dbuff.pc = NOP;
    cpu->dbuff.opn1 = NOP;
    cpu->dbuff.opn2 = NOP;
    cpu->prefetch.index = MAX_PREFETCH;
}

// Flushes the whole pipeline and sets all flags and counters back
// to their original values
void initPipeline(CPU_p cpu) {
    cpu->mbuff.pc = NOP;
    cpu->mbuff.dr = NOP;
    cpu->mbuff.imb = NOP;
    cpu->mbuff.result = NOP;
    cpu->mbuff.op = NOP;
    cpu->ebuff.pc = NOP;
    cpu->ebuff.dr = NOP;
    cpu->ebuff.imb = NOP;
    cpu->ebuff.result = NOP;
    cpu->ebuff.op = NOP;
	cpu->opInStore = NOP_IN_STORE;
	cpu->indirectFlag = false;
	flushPipeline(cpu);
    initStall(cpu);
}

// STORE/WRITE BACK - Writes to registers if required by the current OP
void storeStep(CPU_p cpu) {
    cpu->dr_store = cpu->mbuff.dr;
    switch(cpu->mbuff.op) {
        case ADD:
        case AND:
        case NOT:
        case LD:
        case LDR:
        case LDI:
		case LEA:
            cpu->reg_file[cpu->mbuff.dr] = cpu->mbuff.result;
			cpu->valueInStore = cpu->reg_file[cpu->mbuff.dr];
            updateConCodes(cpu, cpu->reg_file[cpu->mbuff.dr]);
            break;
		case JSR:
			cpu->reg_file[RETURN_REG] = cpu->mbuff.pc + 1;
			cpu->valueInStore = cpu->reg_file[RETURN_REG];
			updateConCodes(cpu, cpu->reg_file[RETURN_REG]);
			break;
        case RSV:
            if (!cpu->mbuff.imb) {
                cpu->reg_file[SP_REG]--;
            } else {
                cpu->reg_file[cpu->mbuff.dr] = cpu->mbuff.result;
                cpu->reg_file[SP_REG]++;
            }
			cpu->dr_store = SP_REG;
			cpu->valueInStore = cpu->reg_file[SP_REG];
			updateConCodes(cpu, cpu->reg_file[cpu->reg_file[SP_REG]]);
			break;
    }
	
	// set opInStore if store is processing an op or not
	cpu->opInStore = (cpu->mbuff.pc) ? cpu->mbuff.op : NOP_IN_STORE;
}

// MEMORY - Implements the memory step and simulated memory access times
// Memory access times simulated by stalling for 10 cycles before resuming
void memoryStep(CPU_p cpu, bool finish) {
    cpu->mbuff.op = cpu->ebuff.op;
    cpu->mbuff.dr = cpu->ebuff.dr;
    cpu->mbuff.pc = cpu->ebuff.pc;
    cpu->mbuff.result = cpu->ebuff.result;
    switch(cpu->mbuff.op) {
        case LD:
        case LDR:
			if(finish) {
				cpu->mbuff.result = memory[cpu->ebuff.result];
			} else {           
				cpu->stalls[P_MEM] = MEMORY_ACCESS_STALL_TIME;
			}
			break;      
        case LDI:
			if(finish) {
				if(cpu->indirectFlag) {
					cpu->mbuff.result = memory[cpu->ebuff.result];
					cpu->indirectFlag = false;
				} else {
					cpu->ebuff.result = memory[cpu->ebuff.result];
					cpu->indirectFlag = true;
					cpu->stalls[P_MEM] = MEMORY_ACCESS_STALL_TIME;
				}
			} else {          
				cpu->stalls[P_MEM] = MEMORY_ACCESS_STALL_TIME;
			} 
			break;
        case ST:
        case STR:
			if(finish) {
				memory[cpu->ebuff.result] = cpu->ebuff.dr;
			} else {
				cpu->stalls[P_MEM] = MEMORY_ACCESS_STALL_TIME;
			} 
			break;
        case STI:
			if(finish) {
				if(cpu->indirectFlag) {
					cpu->mbuff.result = memory[cpu->ebuff.result];
					memory[cpu->ebuff.result] = cpu->ebuff.dr;
					cpu->indirectFlag = false;
				} else {
			        cpu->ebuff.result = memory[cpu->ebuff.result];
					cpu->indirectFlag = true;
					cpu->stalls[P_MEM] = MEMORY_ACCESS_STALL_TIME;
				}
			} else {
				cpu->stalls[P_MEM] = MEMORY_ACCESS_STALL_TIME;
			} 
			break;
        case RSV:
            if (!cpu->ebuff.imb) {
                memory[cpu->reg_file[SP_REG]-1] = cpu->ebuff.result; 
            } else {
                cpu->mbuff.result = memory[cpu->reg_file[SP_REG]];
            }
            cpu->mbuff.imb = cpu->ebuff.imb;
        break; 
    } 

	// Push NOP forward if forced to stall due to memory access time
    if (cpu->stalls[P_MEM])	{
		cpu->mbuff.op = NOP;
        cpu->mbuff.dr = NOP;
        cpu->mbuff.result = NOP;
        cpu->mbuff.pc = NOP;
	}
}

// Calculates the cycles to stall to clear instructions
// ahead of a trap before it executes in the Execute step
void calcStallForTraps(CPU_p cpu) {
	if (cpu->mbuff.pc) {
		cpu->stalls[P_EX]+=2;
	} else if (cpu->opInStore != NOP_IN_STORE) {
		cpu->stalls[P_EX]++;
	}
}

// EXECUTE - implements the execute step
int executeStep(CPU_p cpu, DEBUG_WIN_p win) {
	cpu->ebuff.op = cpu->dbuff.op;
	cpu->ebuff.dr = cpu->dbuff.dr;
	cpu->ebuff.pc = cpu->dbuff.pc;
	cpu->alu_a = cpu->dbuff.opn1;
	cpu->alu_b = cpu->dbuff.opn2;
	
	switch(cpu->ebuff.op) {
		case ADD:
			cpu->alu_r = cpu->alu_a + cpu->alu_b;
			cpu->ebuff.result = cpu->alu_r;
			break;
		case AND:
			cpu->alu_r = cpu->alu_a & cpu->alu_b;
			cpu->ebuff.result = cpu->alu_r; 
			break;
		case NOT:
		    cpu->alu_r = ~cpu->alu_a;
			cpu->ebuff.result = cpu->alu_r; 
			break;
		case BR:
            cpu->alu_b = cpu->dbuff.pc+1;
			cpu->alu_r = cpu->alu_a + cpu->alu_b;
		    if (checkBEN(cpu)) {
                cpu->ebuff.result = cpu->alu_r;
				cpu->prefetch.nextPC = cpu->alu_r;
				flushPipeline(cpu);
                return FLUSH_PIPELINE;				
            }
			break;
		case JSR:
		    cpu->alu_b = cpu->dbuff.pc+1;
			cpu->alu_r = cpu->alu_a + cpu->alu_b;
            if (cpu->dbuff.dr) { //jsr
                cpu->ebuff.result = cpu->alu_r;
				cpu->prefetch.nextPC = cpu->alu_r;
            } else { //jsrr
			    cpu->ebuff.result = cpu->dbuff.opn1;
                cpu->prefetch.nextPC = cpu->ebuff.result;
            }
			flushPipeline(cpu);
            return FLUSH_PIPELINE;
		case JMP:
		    cpu->ebuff.result = cpu->dbuff.opn1;
		    cpu->prefetch.nextPC = cpu->ebuff.result;
			flushPipeline(cpu);
			return FLUSH_PIPELINE;
		case ST:
		case LD:
		case STI:
		case LDI:
		case LEA:
		    cpu->alu_b = cpu->dbuff.pc+1;
			cpu->alu_r = cpu->alu_a + cpu->alu_b;
		    cpu->ebuff.result = cpu->alu_r;
			break;
		case LDR:
		case STR:
		    cpu->alu_r = cpu->alu_a + cpu->alu_b;
		    cpu->ebuff.result = cpu->alu_r;
			break;
		case TRAP:
			calcStallForTraps(cpu);

			if (cpu->stalls[P_EX]) {
				cpu->ebuff.op = NOP;
			    cpu->ebuff.dr = NOP;
			    cpu->ebuff.result = NOP;
			    cpu->ebuff.pc = NOP;
			} else { // Execute TRAP subroutine
				cpu->ebuff.result = cpu->dbuff.opn1;
				if(trap(cpu, win, cpu->dbuff.opn1)) {
					return HALT_PROGRAM;
				}
			}
			break;
		case RSV:
			cpu->ebuff.result = cpu->dbuff.opn1;
			cpu->ebuff.imb = cpu->dbuff.imb;
		    break;
	}
	
	return 0;
}

// Checks the specified buffer for RAW hazards based on the passed src register
bool containsHazard(EMBUFF_s buffer, Register reg) {
	switch (buffer.op) {
		case ADD:
		case AND:
		case NOT:
		case LD:
		case LDI:
		case LDR:
		case LEA:
		    if (reg == buffer.dr) {
				return true;
			}
			return false;
		default:
		    return false;
	}
}

// Returns the results from the alu for ADD/AND/NOT
// Stalls the ID step for LD/LDI/LDR to wait for the correct
// value to be read from memory
Register fowardExecuteData(CPU_p cpu) {
	switch (cpu->ebuff.op) {
		case ADD:
		case AND:
		case NOT:
		    return cpu->alu_r;
		case LD:
		case LDI:
		case LDR:
		case LEA:
		    cpu->stalls[P_ID]++;
			break;
	}
	
	return NOP;
}

// Gets register value from reg while using data fowarding to deal with RAW hazards
Register getRegisterValue(CPU_p cpu, Register reg) {
	Register regValue;
	if (containsHazard(cpu->ebuff, reg)) {
		regValue = fowardExecuteData(cpu);
	} else if (containsHazard(cpu->mbuff, reg)) {
	    // TODO use MDR instead once implemented
		regValue = cpu->mbuff.result;	
	} else {
		regValue = cpu->reg_file[reg];
	}
	
	return regValue;
}

// Checks for RAW hazards and stalls based on when a hazard is found in the pipeline
short checkRawHazards(CPU_p cpu, Register src) {
    if (cpu->dbuff.dr == src && cpu->dbuff.pc) {
        return 3;
    } else if (cpu->ebuff.dr == src && cpu->ebuff.pc) {
        return 2;
    } else if (cpu->mbuff.dr == src && cpu->mbuff.pc) {
        return 1;
    }
    
    return 0;
    
}

// Checks for RAW hazards and stalls based on when a hazard is found in the pipeline
short checkRawHazardsTwoSrcs(CPU_p cpu, Register src1, Register src2) {
    if (cpu->dbuff.dr == src1 && cpu->dbuff.dr == src1 && cpu->dbuff.pc) {
        return 3;
    } else if (cpu->ebuff.dr == src1 && cpu->ebuff.dr == src1 && cpu->ebuff.pc) {
        return 2;
    } else if (cpu->mbuff.dr == src1 && cpu->mbuff.dr == src1 && cpu->mbuff.pc) {
        return 1;
    }
    
    return 0;
}

// DECODE - implements the Decode step and gets values out of registers
void decodeStep(CPU_p cpu) {
    Register opn1;
    Register opn2;
    Register dr = DSTREG(cpu->fbuff.ir);
	Register sr = SRCREG(cpu->fbuff.ir);
	Register sr2 = SRCREG2(cpu->fbuff.ir);
	
    switch((Register)OPCODE(cpu->fbuff.ir)) {
        case ADD:
        case AND:
            opn1 = getRegisterValue(cpu, sr);
            if (IMMBIT(cpu->fbuff.ir)) {
                    opn2 = SEXTIMMVAL(cpu->fbuff.ir);         
            } else {
                    opn2 = getRegisterValue(cpu, sr2);
            }
			break;
		case NOT:
		    opn1 = getRegisterValue(cpu, sr);
			opn2 = SEXTIMMVAL(cpu->fbuff.ir);
			break;
		case BR:
		    dr = NZPBITS(cpu->fbuff.ir);
			opn1 = SEXTPCOFFSET9(cpu->fbuff.ir);
			break;
		case ST:
		case STI:
		    dr = getRegisterValue(cpu, dr);
			opn1 = SEXTPCOFFSET9(cpu->fbuff.ir);
			opn2 = NOP;
		case LD:
		case LDI:
		case LEA:
		    opn1 = SEXTPCOFFSET9(cpu->fbuff.ir);
			opn2 = NOP;
			break;
		case STR:
		    dr = getRegisterValue(cpu, dr);
		case LDR:
		    opn1 = getRegisterValue(cpu, sr);
			opn2 = SEXTPCOFFSET6(cpu->fbuff.ir);
			break;
		case JSR:
            if(JSRBIT11(cpu->fbuff.ir)) { //jsr
                dr =  JSRBIT11(cpu->fbuff.ir);
				opn1 = SEXTPCOFFSET11(cpu->fbuff.ir);
                opn2 = NOP;
            } else { //jsrr
                dr = JSRBIT11(cpu->fbuff.ir);
				opn1 = getRegisterValue(cpu, sr);
                opn2 = NOP;
            }
			break;
		case JMP:
		    opn1 = getRegisterValue(cpu, sr);
			opn2 = NOP;
			break;
		case TRAP:
		    opn1 = ZEXTTRAPVECT(cpu->fbuff.ir);
			opn2 = NOP;
			break;
		case RSV:
		    cpu->dbuff.imb = IMMBIT(cpu->fbuff.ir);
            if (!IMMBIT(cpu->fbuff.ir)) {
                opn1 = getRegisterValue(cpu, dr);
            } else {
                opn1 = NOP;
            }
            opn2 = NOP;
            break;                
    }
    // Push NOP forward if stalling
    if (cpu->stalls[P_ID]) {
            cpu->dbuff.op = NOP;
            cpu->dbuff.dr = NOP;
            cpu->dbuff.opn1 = NOP;
            cpu->dbuff.opn2 = NOP;
            cpu->dbuff.pc = NOP;
    } else { // Not Stalled
            cpu->dbuff.op = (Register)OPCODE(cpu->fbuff.ir);
            cpu->dbuff.dr = dr;
            cpu->dbuff.pc = cpu->fbuff.pc;
            cpu->dbuff.opn1 = opn1;
            cpu->dbuff.opn2 = opn2;
    }
    
}

// Checks for stall signals and handles the stall counter before
// executing the memory step
void memHandler(CPU_p cpu) {
    bool finish = false;
	
	if (cpu->stalls[P_MEM]) {
		cpu->stalls[P_MEM]--;
		
		// if a stall finished indicate the step can complete
		if(!cpu->stalls[P_MEM]) {
			finish = true;
		}
	}
	
	if (!cpu->stalls[P_MEM]) {
	    memoryStep(cpu, finish);
    } else { // push NOP forward (0 for all fields)
        cpu->mbuff.op = NOP;
        cpu->mbuff.dr = NOP;
        cpu->mbuff.result = NOP;
        cpu->mbuff.pc = NOP;
    }
}

// Checks for stall signals and handles the stall counter before
// executing the execute step
int executeHandler(CPU_p cpu, DEBUG_WIN_p win) {
    int exResultSignal = 0;
	
	// If stalled, decrement counter
	if (cpu->stalls[P_EX]) cpu->stalls[P_EX]--;
	
    if (!cpu->stalls[P_EX]) {
		if (cpu->stalls[P_MEM]) {
			cpu->stalls[P_EX] = cpu->stalls[P_MEM];
		} else {
            exResultSignal = executeStep(cpu, win);
		}
    } else {
		// Update stall and do nothing if next is stalled
        if (cpu->stalls[P_MEM]) {
		   // if stall signal is found, update stall counter for this step
		   if (cpu->stalls[P_EX] < cpu->stalls[P_MEM]){
			    cpu->stalls[P_EX] = cpu->stalls[P_MEM];
		   }
		} else { // Push NOP forward otherwise
			cpu->ebuff.op = NOP;
			cpu->ebuff.dr = NOP;
			cpu->ebuff.result = NOP;
			cpu->ebuff.pc = NOP;
		}
    }
		
	return exResultSignal;
}

// Checks for stall signals and handles the stall counter before
// executing the decode step
void decodeHandler(CPU_p cpu) {
	// If stalled, decrement counter
	if (cpu->stalls[P_ID]) cpu->stalls[P_ID]--;
	
    if (!cpu->stalls[P_ID]) {
		if (cpu->stalls[P_EX]) {
			cpu->stalls[P_ID] = cpu->stalls[P_EX];
		} else {
			decodeStep(cpu);
		}
    } else {
		// Update stall and do nothing if next is stalled
        if (cpu->stalls[P_EX]) {
		    if (cpu->stalls[P_ID] < cpu->stalls[P_EX]) { 
				cpu->stalls[P_ID] = cpu->stalls[P_EX];
			}
	    } else { // Push NOP forward otherwise
		    cpu->dbuff.op = NOP;
		    cpu->dbuff.dr = NOP;
		    cpu->dbuff.opn1 = NOP;
		    cpu->dbuff.opn2 = NOP;
		    cpu->dbuff.pc = NOP;
		}
    }
}

// Checks for stall signals and handles the stall counter before
// executing the fetch step
void fetchHandler(CPU_p cpu) {
	// If stalled, decrement counter
	if (cpu->stalls[P_IF]) cpu->stalls[P_IF]--;
		
    if (!cpu->stalls[P_IF]) {
         if (cpu->stalls[P_ID]) {
			cpu->stalls[P_IF] = cpu->stalls[P_ID];
		} else {
			cpu->fbuff.ir = cpu->prefetch.instructs[cpu->prefetch.index];
			cpu->fbuff.pc = cpu->prefetch.nextPC++;
			cpu->prefetch.index++;
		}
    } else {
		// Update stall and do nothing if next is stalled
        if (cpu->stalls[P_ID]) {
			if (cpu->stalls[P_IF] < cpu->stalls[P_ID]) {
		        cpu->stalls[P_IF] = cpu->stalls[P_ID];
		    }
	    } else { // Push NOP forward otherwise
		    cpu->fbuff.ir = NOP;
            cpu->fbuff.pc = NOP;
	    }	
    }
}

// Searches the pipeline and returns the pc for the next instruction that will complete
Register getNextInstrToFinish(CPU_p cpu) {
	if (cpu->mbuff.pc) {
		return cpu->mbuff.pc;
	} else if (cpu->ebuff.pc) {
		return cpu->ebuff.pc;
	} else if (cpu->dbuff.pc) {
		return cpu->dbuff.pc;
	} else if (cpu->fbuff.pc) {
		return cpu->fbuff.pc;
	} else {
		return cpu->prefetch.nextPC;
	}
}

// Main controller for the cpu
bool controller_pipelined(CPU_p cpu, DEBUG_WIN_p win, int mode, BREAKPOINT_p breakpoints) {
    bool breakFlag = false;
    bool foundNext = false;
	cpu->opInStore = false;
	bool haltTriggered = false;
	int exControlSignal = 0;
	
	// One loop equals one cycle of execution
    do {         
        // Store/Write Back
		storeStep(cpu);
        
		// If next pc for step was used in Store
		// flag that it was found to end step after this cycle
		if(cpu->pc == cpu->mbuff.pc) {
            foundNext = true;                  
        }

		// Memory Step
        memHandler(cpu);
		
		// Execute Step
		exControlSignal = executeHandler(cpu, win);
		
		// Exit if Execute Step processes a HALT Trap
		// Skip to next cycle if pipeline is to be flushed this cycle
		if (exControlSignal == HALT_PROGRAM) {
			haltTriggered = true;
		} else if (exControlSignal == FLUSH_PIPELINE){
			continue;
		}

		// Decode Step
        decodeHandler(cpu);
        
        // Instruction prefetch
        if (cpu->prefetch.index == MAX_PREFETCH) {
            while(cpu->prefetch.index > 0) {
                cpu->prefetch.instructs[MAX_PREFETCH - cpu->prefetch.index] = memory[cpu->prefetch.nextPC + (MAX_PREFETCH - cpu->prefetch.index)];              
                cpu->prefetch.index--;               
                // Stall for 10 cycles for each instruction that needs to be fetched
				cpu->stalls[P_IF]+= MAX_PREFETCH * MEMORY_ACCESS_STALL_TIME;
            }         
        }
		
		// Fetch step
		fetchHandler(cpu);
		
		// check if the next instruction to complete is a breakpoint
		breakFlag = breakpointsReached(win, getNextInstrToFinish(cpu));
		
    } while (!haltTriggered && ((mode == RUN_MODE && !breakFlag) || (mode == STEP_MODE && !foundNext)));
	
	// Returns if the program is currently running (true) or halted (false)
	return (haltTriggered) ? false : true;
}

// Main debug monitor
// continues to loop, prompting user for menu items until exit is selected
// updates the UI after returning from the controller_pipelined method
int monitor(CPU_p cpu, DEBUG_WIN_p win) {
    // check to make sure both pointers are not NULLS
    if (!cpu) return NULL_CPU_POINTER;
    if (!memory) return NULL_MEMORY_POINTER;


    int displayMemAddress = DEFAULT_MEM_ADDRESS;
    short orig = DEFAULT_MEM_ADDRESS;    
    char programLoaded = false;
    char input[INPUT_LIMIT];
    
    
    BREAKPOINT_p breakpoints = (BREAKPOINT_p) malloc(sizeof(BREAKPOINT_p));
    initBreakPoints(breakpoints);
    win->breakpoints = breakpoints;

    for(;;)
    {
        //showScreen
        updateScreen(win, cpu, memory, programLoaded);
        promptUser(win, "", input); 

        if (strlen(input) == SINGLE_CHAR) {
            switch(input[MENU_SELECTION]) {
				case LOAD:
					programLoaded = load(cpu, memory, win);
					initPipeline(cpu);
					break;
				case SAVE:
					save(cpu, win);
					break;
				case STEP:
					if (programLoaded) {
						programLoaded = controller_pipelined(cpu, win, STEP_MODE, breakpoints);
						cpu->pc = getNextInstrToFinish(cpu);
						if (displayMemAddress+15 < cpu->pc || cpu->pc < displayMemAddress) { 
							displayMemAddress = cpu->pc;
						    win->memAddress = displayMemAddress;
						}
					} else {
						displayBoldMessage(win, "No program loaded! Press any key...");
					}
					break;
				case RUN:
					if (programLoaded) {
						programLoaded = controller_pipelined(cpu, win, RUN_MODE, breakpoints);
						cpu->pc = getNextInstrToFinish(cpu);
						if (displayMemAddress+15 < cpu->pc || cpu->pc < displayMemAddress) {
							displayMemAddress = cpu->pc;
						    win->memAddress = displayMemAddress;
						}
					} else {
						displayBoldMessage(win, "No program loaded! Press any key...");
					}
					break;
				case DISPLAY_MEM:
					displayMemory(cpu, win, programLoaded);
					break;
				case EDIT:
					edit(cpu, win, programLoaded, memory);
					break;
				case BREAKPOINT:
					if(programLoaded) {
						breakPoint(cpu, win, breakpoints, programLoaded);
					}
					else {
						displayBoldMessage(win, "No program loaded! Press any key...");
					}
					break;
				case EXIT:
					displayBoldMessage(win, "Exit Selected! Press any key...");
					return 0;
				default: 
					displayBoldMessage(win, "Invalid Menu Option");
            }
        }

    } //end for loop
}

// Initializes the cpu and win structs and starts the debug monitor
int main(int argc, char* argv[]) {
    char *temp;

    // initialize resources
    CPU_p cpu = (CPU_p) malloc(sizeof(CPU_s));
    DEBUG_WIN_p win = (DEBUG_WIN_p) malloc(sizeof(DEBUG_WIN_s));
    initializeWindows(win);

    // print
    reprintScreen(win, cpu, memory, false);

    monitor(cpu, win);

    // free resources
    endWindows(win);
    free(win);
    free(cpu);
}
