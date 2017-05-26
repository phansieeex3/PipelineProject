/*
 TCSS372 - Computer Architecture
 Project LC3 
 Group Members: 
 Shaun Coleman
 Phansa Chaonpoj
 Joshua Meigs
 */

#include "slc3_ui.c"

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
    case GETCH:
        cpu->reg_file[IO_REG] = mvwgetch(win->ioWin, win->ioY, win->ioX);
        break;
    case OUT:
        writeCharToIOWin(win, cpu->reg_file[IO_REG]);
        break;
    case PUTS:
        while (memory[cpu->reg_file[IO_REG] + i]) {
            writeCharToIOWin(win, memory[cpu->reg_file[IO_REG] + i]);
            i++;
        }
        break;
    case HALT:
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
                    "File already exists, type Y to overwrite: ",
                    prompt);
            if (prompt[0] == 'Y' || prompt[0] == 'y') //ignore case
                    {
                saveToFile(input, start, end);
                break;
            } else {
                displayBoldMessage(win, "Cancelling! Press any key.");
                return;
            }
        }

    } while (file != NULL);

    if (file == NULL) { //save to file.
        saveToFile(input, start, end);
    }

    fclose(file);
}
    
char load(CPU_p cpu, unsigned short * memory, DEBUG_WIN_p win)
{
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
                "Error: Invalid File. Press any key to continue.");
    }

    return programLoaded;
}

void initStall(CPU_p cpu) {
    cpu->stalls[P_IF] = 0;
    cpu->stalls[P_ID] = 0;
    cpu->stalls[P_EX] = 0;
    cpu->stalls[P_MEM] = 0;
    cpu->stalls[P_STORE] = 0;
}

void updateBreakpoints(DEBUG_WIN_p win) {
    int i;
    for(i = 0; i < MAXBREAK-1; i++) {//such magic
        if(win->breakpoints->breakpointArr[i] == NULL_BREAKPOINT) {
            win->breakpoints->breakpointArr[i] = win->breakpoints->breakpointArr[i+1];
            win->breakpoints->breakpointArr[i+1] = NULL_BREAKPOINT;
        }
    }
}

int breakpointsContains(DEBUG_WIN_p win, int inputAddress) {
    int i;
    for(i = 0; i < MAXBREAK; i++) {
        if(win->breakpoints->breakpointArr[i] == inputAddress) {
            return i;
        }
    }
    return BREAKPOINT_NOT_FOUND;
}

// TODO need both?
bool breakpointsReached(DEBUG_WIN_p win, Register pc) {
    int i;
    for(i = 0; i < MAXBREAK; i++) {
        if(win->breakpoints->breakpointArr[i] == pc) {
            return true;
        }
    }
    return false;
}

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

void initBreakPoints(BREAKPOINT_p breakpoints) {
    int i;
    for(i = 0; i < MAXBREAK; i++) {
        breakpoints->breakpointArr[i] = NULL_BREAKPOINT;
    }
    breakpoints->emptySpaces = MAXBREAK;
}

void save(CPU_p cpu, DEBUG_WIN_p win)
{
    char startAddress[INPUT_LIMIT];
    char endAddress[INPUT_LIMIT];
    char input[INPUT_LIMIT];

    clearPrompt(win);
    promptUser(win, "Start Address: ", startAddress);
    clearPrompt(win);

    // Validate address
    if (strlen(startAddress) > EXPECTED_HEX_DIGITS
            || startAddress[strspn(startAddress,
                    "0123456789abcdefABCDEF")] != 0) {
        displayBoldMessage(win,
                "Invalid address. Press any key to continue.");
		return;
    }


    // Prompt for end address (inclusive)
    clearPrompt(win);
    promptUser(win, "End Address: ", endAddress);
    clearPrompt(win);

    //Validate value
    if (strlen(endAddress) > EXPECTED_HEX_DIGITS
            || endAddress[strspn(endAddress,
                    "0123456789abcdefABCDEF")] != 0) {
        displayBoldMessage(win,
                "Invalid address. Press any key to continue.");
		return;
    }

    promptUser(win, "File name: ", input);

    promptSaveToFile(cpu, input, startAddress, endAddress,
            win);

    displayBoldMessage(win,
            "Save Complete. Press any key to continue.");
}

void displayMemory(CPU_p cpu, DEBUG_WIN_p win, char programLoaded) {

    int displayMemAddress = DEFAULT_MEM_ADDRESS;
    char inputAddress[INPUT_LIMIT];
    char *temp;

    clearPrompt(win);
    promptUser(win, "Starting Address: ", inputAddress);
    clearPrompt(win);

    if (strlen(inputAddress) > EXPECTED_HEX_DIGITS
            || inputAddress[strspn(inputAddress,
                    "0123456789abcdefABCDEF")] != 0) {
        displayBoldMessage(win,
                "Error: Invalid address. Press any key to continue.");
    } else {
        displayMemAddress = strtol(inputAddress, &temp,
                HEX_MODE);
        win->memAddress = displayMemAddress;
        updateScreen(win, cpu, memory, programLoaded);
    }
}

void edit(CPU_p cpu, DEBUG_WIN_p win, char programLoaded, unsigned short * memory)
{
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
            || inputAddress[strspn(inputAddress,
                    "0123456789abcdefABCDEF")] != 0) {
        displayBoldMessage(win,
                "Invalid address. Press any key to continue.");
		return;
    }

    // Prompt for new value to place into memory
    clearPrompt(win);
    promptUser(win, "New Value: ", input);
    clearPrompt(win);

    // Validate value
    if (strlen(input) > EXPECTED_HEX_DIGITS
            || input[strspn(input, "0123456789abcdefABCDEF")]
                    != 0) {
        displayBoldMessage(win,
                "Invalid value. Press any key to continue.");
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

void breakPoint(CPU_p cpu, DEBUG_WIN_p win, BREAKPOINT_p breakpoints, char programLoaded)
{
    char inputAddress[INPUT_LIMIT];
    clearPrompt(win);
    promptUser(win, "Address: ", inputAddress);
    clearPrompt(win);

    // Validate address
    if (strlen(inputAddress) > EXPECTED_HEX_DIGITS
            || inputAddress[strspn(inputAddress,
                    "0123456789abcdefABCDEF")] != 0) {
        displayBoldMessage(win,
                "Invalid address. Press any key to continue.");
	    return;
    }
    
    modifyBreakPoint(win, breakpoints, inputAddress);

    // Update screen to reflect changes
    updateScreen(win, cpu, memory, programLoaded); 
}

int checkBEN(CPU_p cpu) {
    return (cpu->conCodes.n && NBIT(cpu->dbuff.dr))
              + (cpu->conCodes.z && ZBIT(cpu->dbuff.dr))
              + (cpu->conCodes.p && PBIT(cpu->dbuff.dr));    
}

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
	flushPipeline(cpu);
    initStall(cpu);
}

// Write results to register
void storeStep(CPU_p cpu) {
    cpu->mdr = cpu->mbuff.result;
    cpu->dr_store = cpu->mbuff.dr;
    switch(cpu->mbuff.op) {
        case ADD:
        case AND:
        case NOT:
        case LD:
        case LDR:
        case LDI:
            cpu->reg_file[cpu->mbuff.dr] = cpu->mbuff.result;
            updateConCodes(cpu, cpu->mbuff.result);
            break;
        case RSV:
            if (!cpu->mbuff.imb) {
                cpu->reg_file[SP_REG]--;
            } else {
                cpu->reg_file[cpu->mbuff.dr] = cpu->mbuff.result;
                cpu->reg_file[SP_REG]++;
            }
            break;
    }
}

// Memory access (LD/ST like commands)
void memoryStep(CPU_p cpu, bool finish) {
    cpu->mbuff.op = cpu->ebuff.op;
    cpu->mbuff.dr = cpu->ebuff.dr;
    cpu->mbuff.pc = cpu->ebuff.pc;
    cpu->mbuff.result = cpu->ebuff.result;
    // implement stall for ten cycles
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
				if(cpu->indirectFlag == true) {
					cpu->mbuff.result = memory[cpu->ebuff.result];
					cpu->indirectFlag = false;
				} else {
					cpu->ebuff.result = memory[cpu->ebuff.result];// stall for 20 cycles
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
				if(cpu->indirectFlag == true) {
					cpu->mbuff.result = memory[cpu->ebuff.result];
					cpu->indirectFlag = false;
				} else {
					memory[cpu->ebuff.result] = cpu->ebuff.dr;// stall for 20 cycles
					cpu->indirectFlag = true;
					cpu->stalls[P_MEM] = MEMORY_ACCESS_STALL_TIME;
				}
			} else {
				memory[cpu->ebuff.result] = cpu->ebuff.dr;
				cpu->stalls[P_MEM] = MEMORY_ACCESS_STALL_TIME;
			} 
			
			if(cpu->indirectFlag == true) {
				memory[cpu->ebuff.result] = cpu->ebuff.dr; // stall for 20 cycles
				cpu->indirectFlag = false;
			}
			memory[cpu->ebuff.result] = cpu->ebuff.dr; // stall for 20 cycles
			cpu->indirectFlag = true;
			cpu->stalls[P_MEM] = MEMORY_ACCESS_STALL_TIME;
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

    if (cpu->stalls[P_MEM])	{
		cpu->mbuff.op = NOP;
        cpu->mbuff.dr = NOP;
        cpu->mbuff.result = NOP;
        cpu->mbuff.pc = NOP;
	}
}

// Execute + Eval Address
bool executeStep(CPU_p cpu, DEBUG_WIN_p win) {
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
			break;
		case BR:

			// Stall until next OP doesnt write to reg
			// Ignore this step for a NOP BR (Branch on nothing)
			if (cpu->mbuff.pc && cpu->ebuff.pc) {
				cpu->stalls[P_EX]++;
			    cpu->ebuff.op = NOP;
			    cpu->ebuff.dr = NOP;
			    cpu->ebuff.result = NOP;
			    cpu->ebuff.pc = NOP;
				break;
			}
			
		    if (checkBEN(cpu)) {
                cpu->ebuff.result = cpu->dbuff.pc + cpu->dbuff.opn1 + 1;
				cpu->prefetch.nextPC = cpu->ebuff.result;
				// flush pipeline and prefetch
				flushPipeline(cpu);	
            }
			break;
		case JSR:
		case JMP:
		    cpu->ebuff.result = cpu->dbuff.opn1;
		    cpu->prefetch.nextPC = cpu->ebuff.result;
			flushPipeline(cpu);
			break;
		case ST:
		case LD:
		case STI:
		case LDI:
		case LEA:
		    cpu->ebuff.result = cpu->dbuff.pc + cpu->dbuff.opn1 + 1;
			break;
		case LDR:
		case STR:
		    cpu->ebuff.result = cpu->dbuff.opn1 + cpu->dbuff.opn2 + 1;
			break;
		case TRAP:
		    if (cpu->mbuff.pc && cpu->mbuff.op) {
				cpu->stalls[P_EX]++;
				cpu->ebuff.op = NOP;
			    cpu->ebuff.dr = NOP;
			    cpu->ebuff.result = NOP;
			    cpu->ebuff.pc = NOP;
			} else {
				cpu->ebuff.result = cpu->dbuff.opn1;
				if(trap(cpu, win, cpu->dbuff.opn1)) {
					return true;
				}
			}
			break;
		case RSV:
			cpu->ebuff.result = cpu->dbuff.opn1;
			cpu->ebuff.imb = cpu->dbuff.imb;
		    break;
	}
	
	
	return false;
}

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

// decode IR and get values out of registers
void decodeStep(CPU_p cpu) {
    Register opn1;
    Register opn2;
    Register dr = DSTREG(cpu->fbuff.ir);
    switch((Register)OPCODE(cpu->fbuff.ir)) {
        case ADD:
        case AND:
            opn1 = cpu->reg_file[SRCREG(cpu->fbuff.ir)];
            if (IMMBIT(cpu->fbuff.ir)) {
                    opn2 = SEXTIMMVAL(cpu->fbuff.ir);
                    cpu->stalls[P_ID] = checkRawHazards(cpu, SRCREG(cpu->fbuff.ir));
                    
            } else {
                    opn2 = cpu->reg_file[SRCREG2(cpu->fbuff.ir)];
                    cpu->stalls[P_ID] = checkRawHazardsTwoSrcs(cpu, SRCREG(cpu->fbuff.ir), SRCREG2(cpu->fbuff.ir));
            }
			break;
		case NOT:
		    opn1 = cpu->reg_file[SRCREG(cpu->fbuff.ir)];
			opn2 = SEXTIMMVAL(cpu->fbuff.ir);
			cpu->stalls[P_ID] = checkRawHazards(cpu, SRCREG(cpu->fbuff.ir));
			break;
		case BR:
		    dr = NZPBITS(cpu->fbuff.ir);
			opn1 = SEXTPCOFFSET9(cpu->fbuff.ir);
			break;
		case ST:
		case STI:
		    dr = cpu->reg_file[dr];
		case LD:
		case LDI:
		case LEA:
		    opn1 = SEXTPCOFFSET9(cpu->fbuff.ir);
			opn2 = NOP;
			break;
		case STR:
		    dr = cpu->reg_file[dr];
		case LDR:
		    opn1 = cpu->reg_file[SRCREG(cpu->fbuff.ir)];
			opn2 = SEXTPCOFFSET6(cpu->fbuff.ir);
			cpu->stalls[P_ID] = checkRawHazards(cpu, SRCREG(cpu->fbuff.ir));
			break;
		case JSR:
		    // only does JSR
		    opn1 = SEXTPCOFFSET11(cpu->fbuff.ir);
			opn2 = NOP;
			break;
		case JMP:
		    opn1 = cpu->reg_file[SRCREG(cpu->fbuff.ir)];
			opn2 = NOP;
			cpu->stalls[P_ID] = checkRawHazards(cpu, SRCREG(cpu->fbuff.ir));
			break;
		case TRAP:
		    opn1 = ZEXTTRAPVECT(cpu->fbuff.ir);
			opn2 = NOP;
			break;
		case RSV:
		    cpu->dbuff.imb = IMMBIT(cpu->fbuff.ir);
            if (!IMMBIT(cpu->fbuff.ir)) {
                opn1 = cpu->reg_file[DSTREG(cpu->fbuff.ir)];
                cpu->stalls[P_ID] = checkRawHazards(cpu, DSTREG(cpu->fbuff.ir));
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

void memHandler(CPU_p cpu) {
    bool finish = false;
	
	if (cpu->stalls[P_MEM]) {
		cpu->stalls[P_MEM]--;
		if(!cpu->stalls[P_MEM]) {
			finish = true;
		}
	}
	
	if (!cpu->stalls[P_MEM]) {
	    memoryStep(cpu, finish);
    } else {
        cpu->mbuff.op = NOP;
        cpu->mbuff.dr = NOP;
        cpu->mbuff.result = NOP;
        cpu->mbuff.pc = NOP;
    }
}

bool executeHandler(CPU_p cpu, DEBUG_WIN_p win) {
    bool haltProgram = false;
	
	// If stalled, decrement counter
	if (cpu->stalls[P_EX]) cpu->stalls[P_EX]--;
	
    if (!cpu->stalls[P_EX]) {
		if (cpu->stalls[P_MEM]) {
			cpu->stalls[P_EX] = cpu->stalls[P_MEM];
		} else {
            haltProgram = executeStep(cpu, win);
		}
    } else {
		// Update stall and do nothing if next is stalled
        if (cpu->stalls[P_MEM]) {
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
		
	return haltProgram;
}

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

Register getNextInstrToFinish(CPU_p cpu) {
	if (cpu->mbuff.pc && cpu->mbuff.op) {
		return cpu->mbuff.pc;
	} else if (cpu->ebuff.pc && cpu->ebuff.op) {
		return cpu->ebuff.pc;
	} else if (cpu->dbuff.pc && cpu->dbuff.op) {
		return cpu->dbuff.pc;
	} else if (cpu->fbuff.pc && cpu->fbuff.ir) {
		return cpu->fbuff.pc;
	} else {
		return cpu->prefetch.nextPC;
	}
}

bool controller_pipelined(CPU_p cpu, DEBUG_WIN_p win, int mode, BREAKPOINT_p breakpoints) {
    bool breakFlag = false;
    bool foundNext = false;
	
    do {         
        // Store/Write Back
		storeStep(cpu);
        
		// If next pc for step was used in Store
		// flag that it was found to end step after this cycle
		if(cpu->pc == cpu->mbuff.pc) {
            foundNext = true;                  
        }

        memHandler(cpu);
		
		// Exit if Execute Step processes a HALT Trap
		if(executeHandler(cpu, win)) return false;

        decodeHandler(cpu);
        
        // Instruction prefetch
        if (cpu->prefetch.index == MAX_PREFETCH) {
            
            while(cpu->prefetch.index > 0) {
                cpu->prefetch.instructs[MAX_PREFETCH - cpu->prefetch.index] = memory[cpu->prefetch.nextPC + (MAX_PREFETCH - cpu->prefetch.index)];              
                cpu->prefetch.index--;               
                //cpu->stalls[P_IF]+= MAX_PREFETCH * MEMORY_ACCESS_STALL_TIME;
            }         
        }
		
		fetchHandler(cpu);
		
		breakFlag = breakpointsReached(win, getNextInstrToFinish(cpu));
		
		// TODO - mem accessed flag removed as not being used
		// TODO - can add an accessed flag to prefetch and/or memory if needed
    } while ((mode == RUN_MODE && !breakFlag) || (mode == STEP_MODE && !foundNext));
		
	return true;
}

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
					} else {
						displayBoldMessage(win, "No program loaded! Press any key to continue.");
					}
					break;
				case RUN:
					if (programLoaded) {
						programLoaded = controller_pipelined(cpu, win, RUN_MODE, breakpoints);
						cpu->pc = getNextInstrToFinish(cpu);
					} else {
						displayBoldMessage(win, "No program loaded! Press any key to continue.");
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
						displayBoldMessage(win, "No program loaded! Press any key to continue.");
					}
					break;
				case EXIT:
					displayBoldMessage(win, "Exit Selected! Press any key to continue.");
					return 0;
				default: 
					displayBoldMessage(win, "Invalid Menu Option");
            }
        }

    } //end for loop
}

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
