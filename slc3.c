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
int trap(CPU_p cpu, DEBUG_WIN_p win) {
    int i = 0;
    switch (cpu->mar) {
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
                    "This file already exists, do you want to overwrite? \n Y/N \n",
                    prompt);
            if (prompt[0] == 'Y' || prompt[0] == 'y') //ignore case
                    {
                saveToFile(input, start, end);
                break;
            } else if (prompt[0] == 'N' || prompt[0] == 'n') {
                clearPrompt(win);
                promptUser(win, "Enter new file name: ", input);
                file = fopen(input, "r");
                fclose(file);
                //saveToFile(input, memory,  start, end); 

            } else {
                displayBoldMessage(win, "invalid response! Cancelling.");
                return;
            }
        }

    } while (file != NULL);

    if (file == NULL) { //save to file.
        saveToFile(input, start, end);
    }

    fclose(file);

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

short breakpointsContains(DEBUG_WIN_p win, unsigned short inputAddress) {
    int i;
    for(i = 0; i < MAXBREAK; i++) {
        if(win->breakpoints->breakpointArr[i] == inputAddress) {
            return i;
        }
    }
    return NULL_BREAKPOINT;
}

void modifyBreakPoint(DEBUG_WIN_p win ,BREAKPOINT_p breakpoints, char* inputAddress) {
    char* temp;
    unsigned short breakpointToAdd = strtol(inputAddress, &temp, HEX_MODE);
    short breakpointLocation = breakpointsContains(win, breakpointToAdd);
    if(breakpointLocation > NULL_BREAKPOINT) {
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
    displayBoldMessage(win, "Error: Breakpoints ARR Full.");
}



void initBreakPoints(BREAKPOINT_p breakpoints) {
    int i;
    for(i = 0; i < MAXBREAK; i++) {
        breakpoints->breakpointArr[i] = NULL_BREAKPOINT;
    }
    breakpoints->emptySpaces = MAXBREAK;
}

int controller(CPU_p cpu, DEBUG_WIN_p win) { //, FILE * file
    // check to make sure both pointers are not NULLS
    if (!cpu)
        return NULL_CPU_POINTER;
    if (!memory)
        return NULL_MEMORY_POINTER;

    unsigned short opcode, Rd, Rs1, Rs2, immed9;    // fields for the IR
    unsigned short ben;
    char *temp;
    short state = FETCH;
    char* message;
    char readyToLeave = false;
    char programLoaded = false;
    char programRunning = false;
    int displayMemAddress = DEFAULT_MEM_ADDRESS;
    short orig = DEFAULT_MEM_ADDRESS;
    
    BREAKPOINT_p breakpoints = (BREAKPOINT_p) malloc(sizeof(BREAKPOINT_p));
    initBreakPoints(breakpoints);
    win->breakpoints = breakpoints;
    for (;;) {   // efficient endless loop
        char input[INPUT_LIMIT];
        char inputAddress[INPUT_LIMIT];

        //for save option
        char startAddress[INPUT_LIMIT];
        char endAddress[INPUT_LIMIT];

        switch (state) {
        case FETCH: // microstates 18, 33, 35 in the book    
            do {
                if (programRunning)
                    break;

                readyToLeave = false;
                updateScreen(win, cpu, memory, programLoaded);
                promptUser(win, "", input);

                // Evaluate User Input
                if (strlen(input) == SINGLE_CHAR) {
                    switch (input[MENU_SELECTION]) {
                    case LOAD:
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
                        break;

                    case SAVE:
                        // Prompt for start address

                        clearPrompt(win);
                        promptUser(win, "Start Address: ", startAddress);
                        clearPrompt(win);

                        // Validate address
                        if (strlen(startAddress) > EXPECTED_HEX_DIGITS
                                || startAddress[strspn(startAddress,
                                        "0123456789abcdefABCDEF")] != 0) {
                            displayBoldMessage(win,
                                    "Error: Invalid address. Press any key to continue.");
                            continue;
                        }

                        char *tempStart = inputAddress;

                        // Prompt for end address (inclusive)
                        clearPrompt(win);
                        promptUser(win, "End Address: ", endAddress);
                        clearPrompt(win);

                        //Validate value
                        if (strlen(endAddress) > EXPECTED_HEX_DIGITS
                                || endAddress[strspn(endAddress,
                                        "0123456789abcdefABCDEF")] != 0) {
                            displayBoldMessage(win,
                                    "Error: Invalid address. Press any key to continue.");
                            continue;
                        }

                        char *tempEnd = input;

                        promptUser(win, "File name: ", input);

                        promptSaveToFile(cpu, input, startAddress, endAddress,
                                win);

                        displayBoldMessage(win,
                                "Succesfull, New data saved to file.");

                        break;

                    case STEP:
                        if (!programLoaded) {
                            displayBoldMessage(win,
                                    "No program loaded! Press any key to continue.");
                        } else {
                            readyToLeave = true;
                            continue;
                        }

                        break;
                    case RUN:
                        if (!programLoaded) {
                            displayBoldMessage(win,
                                    "No program loaded! Press any key to continue.");
                        } else {
                            readyToLeave = true;
                            programRunning = true;
                            continue;
                        }

                        break;
                    case DISPLAY_MEM:
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
                            continue;
                        }
                        break;
                    case EDIT:
                        // Prompt for Address to edit
                        clearPrompt(win);
                        promptUser(win, "Address to Edit: ", inputAddress);
                        clearPrompt(win);

                        // Validate address
                        if (strlen(inputAddress) > EXPECTED_HEX_DIGITS
                                || inputAddress[strspn(inputAddress,
                                        "0123456789abcdefABCDEF")] != 0) {
                            displayBoldMessage(win,
                                    "Error: Invalid address. Press any key to continue.");
                            continue;
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
                                    "Error: Invalid Value. Press any key to continue.");
                            continue;
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

                        break;
                    case BREAKPOINT:
                        if(programLoaded) {
                           // Prompt for Address to edit
                            clearPrompt(win);
                            promptUser(win, "Address to Add A Breakpoint To: ", inputAddress);
                            clearPrompt(win);

                            // Validate address
                            if (strlen(inputAddress) > EXPECTED_HEX_DIGITS
                                    || inputAddress[strspn(inputAddress,
                                            "0123456789abcdefABCDEF")] != 0) {
                                displayBoldMessage(win,
                                        "Error: Invalid address. Press any key to continue.");
                                continue;
                            }
                            
                            modifyBreakPoint(win, breakpoints, inputAddress);
                            

                            // Update screen to reflect changes
                            updateScreen(win, cpu, memory, programLoaded); 
                        } 
                        displayBoldMessage(win, "No program loaded! Press any key to continue.");
                        break;
                    
                    case EXIT:
                        mvwprintw(win->mainWin, PROMPT_DISPLAY_Y,
                                PROMPT_DISPLAY_X,
                                "Exit Selected! Press any key to continue.");
                        wgetch(win->mainWin);
                        return 0;
                    default:
                        clearPrompt(win);
                        displayBoldMessage(win, "Invalid Menu option!");
                    }
                } else {
                    clearPrompt(win);
                    displayBoldMessage(win, "Invalid Menu option!");
                }
                // Refresh Screen
                updateScreen(win, cpu, memory, programLoaded);

                // Pause
                mvwgetch(win->mainWin, PROMPT_DISPLAY_Y, PROMPT_DISPLAY_X);
                input[MENU_SELECTION] = '-';

                clearPrompt(win);
            } while (!readyToLeave || !programLoaded);

            // get memory[PC] into IR - memory is a global array
            cpu->mar = cpu->pc;
            // increment PC
            cpu->pc++;
            cpu->mdr = memory[cpu->mar];
            cpu->ir = cpu->mdr;
            state = DECODE;
            break;
        case DECODE: // microstate 32

            opcode = (unsigned short) OPCODE(cpu->ir);
            Rd = DSTREG(cpu->ir);
            Rs1 = SRCREG(cpu->ir);
            Rs2 = SRCREG2(cpu->ir);

            //sext immediate 9
            immed9 = SEXTPCOFFSET9(cpu->ir);

            ben = (cpu->conCodes.n && NBIT(cpu->ir))
                    + (cpu->conCodes.z && ZBIT(cpu->ir))
                    + (cpu->conCodes.p && PBIT(cpu->ir));

            state = EVAL_ADDR;
            break;
        case EVAL_ADDR: // Look at the LD instruction to see microstate 2 example
            switch (opcode) {
            case ADD:
            case AND:
            case NOT:
                break;

            case TRAP:
                // zext trapVector
                cpu->mar = ZEXTTRAPVECT(cpu->ir);
                break;
            case LD:
                cpu->mar = cpu->pc + immed9;
                break;
            case LDR:
                cpu->mar = cpu->reg_file[Rs1] + SEXTPCOFFSET6(cpu->ir);
                break;
            case ST:
                cpu->mar = cpu->pc + immed9;
                break;
            case STR:
                cpu->mar = cpu->reg_file[Rs1] + SEXTPCOFFSET6(cpu->ir);
                break;
            case JMP: // and RET
                //if BaseR == 111
                if (cpu->reg_file[Rs1] == RETURN_REG) //ret case
                        {
                    cpu->pc = cpu->reg_file[RETURN_REG]; //go to register 7?
                } else {
                    cpu->pc = cpu->reg_file[Rs1];
                }
                break;
            case LEA:
                //loads DR = PC + SEXT(PCOffset9)
                cpu->reg_file[Rd] = cpu->pc + immed9;
                //setCC
                updateConCodes(cpu, immed9);
                break;
            case JSR:
                cpu->reg_file[RETURN_REG] = cpu->pc;
                if (!NBIT(cpu->ir)) //check if JSRR incase if we implement JSR
                        {
                    //pc = baseR
                    cpu->pc = cpu->reg_file[Rs1];
                }
                //implementing JSR here{
                //cpu->pc = cpu->pc + SEXT(PCOffset11);

                break;
                //case RET: same case as JMP

            case BR:
                if (ben) {
                    cpu->pc = cpu->pc + immed9;
                }
                break;
            case RSV:
                //using base register. 
                cpu->mar = cpu->reg_file[Rs1];
                //destination register is always r6

                break;
            default:
                break;
            }
            state = FETCH_OP;
            break;
        case FETCH_OP: // Look at ST. Microstate 23 example of getting a value out of a register
            switch (opcode) {
            case ADD:
                cpu->alu_a = cpu->reg_file[Rs1];
                // Check bit 5
                if (IMMBIT(cpu->ir)) {
                    // sext immediate value
                    cpu->alu_b = SEXTIMMVAL(cpu->ir);
                } else {
                    cpu->alu_b = cpu->reg_file[Rs2];
                }
                break;
            case AND:
                cpu->alu_a = cpu->reg_file[Rs1];
                // Check bit 5
                if (IMMBIT(cpu->ir)) {
                    // sext immediate value
                    cpu->alu_b = SEXTIMMVAL(cpu->ir);
                } else {
                    cpu->alu_b = cpu->reg_file[Rs2];
                }
                break;
            case NOT:
                cpu->alu_a = cpu->reg_file[Rs1];
                break;
            case TRAP:
                cpu->mdr = memory[cpu->mar];
                cpu->reg_file[RETURN_REG] = cpu->pc;
                break;
            case LD:
            case LDR:
                cpu->mdr = memory[cpu->mar];
                break;
            case ST:
            case STR:
                cpu->mdr = cpu->reg_file[Rd];
                break;

            case RSV: //not sure where this would be
                break;

            default:
                break;
            }
            state = EXECUTE;
            break;
        case EXECUTE: // Note that ST does not have an execute microstate
            switch (opcode) {
            // do what the opcode is for, e.g. ADD
            // in case of TRAP: call trap(int trap_vector) routine, see below for TRAP x25 (HALT)
            case ADD:
                cpu->alu_r = cpu->alu_a + cpu->alu_b;
                break;
            case AND:
                cpu->alu_r = cpu->alu_a & cpu->alu_b;
                break;
            case NOT:
                cpu->alu_r = ~cpu->alu_a;
                break;

            case TRAP:
                // Halt if trap function returns 1
                if (trap(cpu, win)) {
                    // end current program
                    cpu->pc = orig;
                    state = FETCH;
                    programLoaded = false;
                    programRunning = false;
                    displayBoldMessage(win,
                            "Program HALT. Press any key to continue.");
                    mvwgetch(win->mainWin, PROMPT_DISPLAY_Y, PROMPT_DISPLAY_X);
                    clearPrompt(win);
                }
                break;

            case RSV:

                break;
            default:
                break;
            }
            state = STORE;
            break;
        case STORE: // Look at ST. Microstate 16 is the store to memory
            switch (opcode) {
            // write back to register or store MDR into memory
            case ADD:
            case AND:
            case NOT:
                cpu->reg_file[Rd] = cpu->alu_r;
                updateConCodes(cpu, cpu->reg_file[Rd]);
                break;
            case LD:
            case LDR:
                cpu->reg_file[Rd] = cpu->mdr;
                updateConCodes(cpu, cpu->reg_file[Rd]);
                break;
            case ST:
                memory[cpu->mar] = cpu->mdr;
                break;
            case STR:
                memory[cpu->mar] = cpu->mdr;
                break;

            case RSV:
                if (IMMBIT(cpu->ir)) //if 1, push case
                        {
                    cpu->mdr = memory[cpu->reg_file[Rd]]--; //make room on the stack R6
                    //memory[r6] <- Br
                    memory[cpu->reg_file[Rd]] = cpu->reg_file[Rd]; //push item on stack

                } else //pop case
                {
                    //Br <- memory[r6]
                    cpu->reg_file[Rs1] = memory[cpu->reg_file[Rd]];
                    cpu->mdr = memory[cpu->reg_file[Rd]]++; //pop off stack.     

                }
                break;
            }

            // do any clean up here in prep for the next complete cycle
            state = FETCH;
            break;
        }
    }
}

int checkBEN(CPU_p cpu) {
    // much magic, such bad, wow!
	Register nzp;
	nzp = cpu->dbuff.opn1 << 9;
	return (cpu->conCodes.n && NBIT(nzp))
              + (cpu->conCodes.z && ZBIT(nzp))
              + (cpu->conCodes.p && PBIT(nzp));	
}

void flushPipeline(CPU_p cpu) {
	cpu->fbuff.pc = NOP;
	cpu->fbuff.ir = NOP;
	cpu->fbuff.pc = NOP;
	cpu->dbuff.op = NOP;
	cpu->dbuff.dr = NOP;
	cpu->dbuff.pc = NOP;
	cpu->dbuff.opn1 = NOP;
	cpu->dbuff.opn2 = NOP;
	cpu->prefetch.index = MAX_PREFETCH;
}

void monitor(CPU_p cpu, DEBUG_WIN_p win) {
    // prep
    // main do/while loop
    // prompt user
    // Menu Switch
    // Step/run call controller_pipeline(cpu, mode, breakpoints) mode: STEP, RUN
    // load/edit/save/display/breakpoint calls appropriate funtion

}

// Write results to register
void storeStep() {
	
}

// Memory access (LD/ST like commands)
void memoryStep() {
	
}

// Execute + Eval Address
void executeStep(CPU_p cpu, DEBUG_WIN_p win) {
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
		case BR:
		    // Stall until next OP doesnt write to reg
		    if (checkBEN(cpu)) {
                    cpu->ebuff.result = cpu->dbuff.pc + cpu->dbuff.opn2;
					cpu->pc = cpu->ebuff.result;
					
					// flush pipeline and prefetch
					flushPipeline(cpu);	
            }
			break;
		case JSR:
		case JMP:
		    cpu->ebuff.result = cpu->dbuff.pc + cpu->dbuff.opn2;
		    cpu->pc = cpu->ebuff.result;
			flushPipeline(cpu);
		case ST:
		case LD:
		case LEA:
		    cpu->ebuff.result = cpu->dbuff.pc + cpu->dbuff.opn1;
			break;
		case LDR:
		case STR:
		    cpu->ebuff.result = cpu->dbuff.opn1 + cpu->dbuff.opn2;
		case TRAP:
		    // Test for correctness
			if(trap(cpu, win)) {
			    return;
			}
		case RSV:
		    
		    break;
	}
}

// decode IR and get values out of registers
void decodeStep(CPU_p cpu) {
	cpu->dbuff.op = (Register)OPCODE(cpu->ir);
    cpu->dbuff.dr = DSTREG(cpu->ir);
    cpu->dbuff.pc = cpu->fbuff.pc;
	switch(cpu->dbuff.op) {
	    case ADD:
		case AND:
		    cpu->dbuff.opn1 = cpu->reg_file[SRCREG(cpu->ir)];
			if (IMMBIT(cpu->ir)) {
                    cpu->dbuff.opn2 = SEXTIMMVAL(cpu->ir);
            } else {
                    cpu->dbuff.opn2 = cpu->reg_file[SRCREG2(cpu->ir)];
            }
			break;
		case NOT:
		    cpu->dbuff.opn1 = cpu->reg_file[SRCREG(cpu->ir)];
			cpu->dbuff.opn2 = SEXTIMMVAL(cpu->ir);
			break;
		case BR:
		    cpu->dbuff.opn1 = NZPBITS(cpu->ir);
			cpu->dbuff.opn2 = SEXTPCOFFSET9(cpu->ir);
			break;
		case ST:
		case LD:
		case LEA:
		    cpu->dbuff.opn1 = SEXTPCOFFSET9(cpu->ir);
			cpu->dbuff.opn2 = NOP;
			break;
		case STR:
		case LDR:
		    cpu->dbuff.opn1 = cpu->reg_file[SRCREG(cpu->ir)];
			cpu->dbuff.opn2 = SEXTPCOFFSET9(cpu->ir);
			break;
		case JSR:
		    // only does JSRR
		    cpu->dbuff.opn1 = SEXTPCOFFSET9(cpu->ir);
			cpu->dbuff.opn2 = NOP;
			break;
		case JMP:
		    cpu->dbuff.opn1 = cpu->reg_file[SRCREG(cpu->ir)];
			cpu->dbuff.opn2 = NOP;
			break;
		case TRAP:
		    cpu->dbuff.opn1 = ZEXTTRAPVECT(cpu->ir);
			cpu->dbuff.opn2 = NOP;
			break;
		case RSV:
		    cpu->dbuff.opn1 = cpu->reg_file[SRCREG(cpu->ir)];
			cpu->dbuff.opn2 = IMMBIT(cpu->ir);
	}
}

int controller_pipelined(CPU_p cpu, DEBUG_WIN_p win, int mode, int* breakpoints) {
    // main controller for pipelines

    // Note: Simulate memory with 10 cycles of access time
    // Need to handle RAWs with stalls/NOPs
    // Need to handle branch hazards with not taken prediction and pipeline flushing

    // do/while not halt or not breakpoint or not step finished(still need to deal with mem cycles, so @ next PC)
    // Prefetch (handle instruction prefetch)

    // Store

    // Memory
    // check: is Stalled? push nop forward and restart loop
    // call function to contains switch to handle each OP during this step

    // Execute
    // check: is Stalled? push nop forward and restart loop
    // call function to contains switch to handle each OP during this step

    // Branch taken?
    // Flush pipeline(DBUFF, FBUFF set to NOP)
    // Update current PC for fetch
    // reset instruction prefetch queue?

    // Decode/Reg
    // check: is Stalled? push nop forward and restart loop
    // call function to contains switch to handle each OP during this step

    // RAW detected (DR = prev Sr1 or Sr2)?
    // Stall (3 NOPs) (counter that adds NOPs to DBUFF 3 cycles in a row)

    // Fetch
    // check: is Stalled? push nop forward and restart loop
    // call function to contains switch to handle each OP during this step

    // return reason for stopping (HALTED, BREAKPOINT, STEP_FINISHED)
	
	// TODO Add functions to handle stalls (set what is stalled and remove stalls)
	// for situations such as memory handling and 
	short memoryAccessed = false;
	short memCycleCounter = 0;
	short breakFlag = false;
	cpu->prefetch.index = MAX_PREFETCH;
	
	do {
		// Pre cycle work
		  // Instruction prefetch
		  if (cpu->prefetch.index == MAX_PREFETCH && !memoryAccessed) {
			  // TODO handle instruction prefetch
		  }
		  
		  // stall handlers
		  
		  // check for breakpoint/set breakpoint flag
			
		// Store 
		if (!cpu->stalls[P_STORE]) {
			// TODO add switch statement to handle each instruction
			// separate method?
		}
		
		// MEM
		if (!cpu->stalls[P_MEM]) {
			// TODO add switch statement to handle each instruction
			// separate method?
		}
		
		// EX
		if (!cpu->stalls[P_EX]) {
			executeStep(cpu, win);
		}
		
		// ID/RR
		if (!cpu->stalls[P_ID]) {
			decodeStep(cpu);
		}
		
		// IF
		if (!cpu->stalls[P_IF]) {
			// prefetch should be handled above and ready if this section is not stalled
			cpu->fbuff.ir = cpu->prefetch.instructs[cpu->prefetch.index];
			cpu->fbuff.pc = cpu->pc++;
			cpu->prefetch.index++;
		}
			
	} while (memoryAccessed ||(mode == RUN_MODE && !breakFlag));
}

int main(int argc, char* argv[]) {
    char *temp;

    // initialize resources
    CPU_p cpu = (CPU_p) malloc(sizeof(CPU_s));
    DEBUG_WIN_p win = (DEBUG_WIN_p) malloc(sizeof(DEBUG_WIN_s));
    initializeWindows(win);

    // print
    reprintScreen(win, cpu, memory, false);

    controller(cpu, win);

    // free resources
    endWindows(win);
    free(win);
    free(cpu);
}
