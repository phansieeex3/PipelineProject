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
    if(val < 0) {
       cpu->conCodes.n = true;
       cpu->conCodes.z = false;
       cpu->conCodes.p = false;          
    } else if(val == 0) {
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
	 switch(cpu->mar) {
        case GETCH:
			cpu->reg_file[IO_REG] = mvwgetch(win->ioWin, win->ioY, win->ioX);
			break;
		case OUT:
		    writeCharToIOWin(win, cpu->reg_file[IO_REG]);
			break;
		case PUTS:
		    while(memory[cpu->reg_file[IO_REG]+i]) {
				writeCharToIOWin(win, memory[cpu->reg_file[IO_REG]+i]);
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
	if (strlen(line) > EXPECTED_HEX_DIGITS || line[strspn(line, "0123456789abcdefABCDEF")] != 0) {
	    return false;
	}
	cpu->pc = strtol(line, &t, HEX_MODE);
	temp = cpu->pc;
	while(!feof(theInFile)) {
		fscanf(theInFile, "%s", &line);
	    if (strlen(line) > EXPECTED_HEX_DIGITS || line[strspn(line, "0123456789abcdefABCDEF")] != 0) {
	        return false;
	    }  
		memory[temp] = strtol(line, &t, HEX_MODE);
		temp++;
	}
	
	return true;
}

char loadFile(char * theInFile, CPU_p cpu) {
	FILE * inFile =  fopen(theInFile,"r");
	
	if(!inFile) {
		return false;
	}	

	return loadFileIntoMemory(inFile, cpu);
}

int controller (CPU_p cpu, DEBUG_WIN_p win) { //, FILE * file
    // check to make sure both pointers are not NULLS
    if (!cpu) return NULL_CPU_POINTER;
    if (!memory) return NULL_MEMORY_POINTER;
    
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
	
    for (;;) {   // efficient endless loop
	 char input[INPUT_LIMIT];    
	 			
		switch (state) {
            case FETCH: // microstates 18, 33, 35 in the book    
				 do {
				    if (programRunning) break;
					
					readyToLeave = false;
					updateScreen(win, cpu, memory, programLoaded);
					promptUser(win, "",input); 
					
					 // Evaluate User Input
					 if (strlen(input) == SINGLE_CHAR) {
						 switch (input[MENU_SELECTION]) {
							 case '1':
								promptUser(win, "File Name: ",input); 	
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
										  displayBoldMessage(win, "Error: Invalid File. Press any key to continue.");
									}								
								 break;
							 case '3':
								 if(!programLoaded) {
									displayBoldMessage(win, "No program loaded! Press any key to continue."); 
								 } else {								  
									readyToLeave = true;
									continue;
								 }
								 
								 break;
							 case '4':
						         if(!programLoaded) {
									displayBoldMessage(win, "No program loaded! Press any key to continue."); 
								 } else {								  
									readyToLeave = true;
									programRunning = true;
									continue;
								 }
								 
								 break;
							 case '5':
							    clearPrompt(win);
								promptUser(win, "Starting Address: ",input);
                                clearPrompt(win);
								
								if (strlen(input) > EXPECTED_HEX_DIGITS || input[strspn(input, "0123456789abcdefABCDEF")] != 0) {
									displayBoldMessage(win, "Error: Invalid address. Press any key to continue.");
								}  else {
								    displayMemAddress = strtol(input, &temp, HEX_MODE);
									win->memAddress = displayMemAddress;
									updateScreen(win, cpu, memory, programLoaded);
									continue;
								}								
								break;
							 case '9':
								 mvwprintw(win->mainWin, PROMPT_DISPLAY_Y, PROMPT_DISPLAY_X, "Exit Selected! Press any key to continue.");
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
                cpu->ir =cpu->mdr;               				
               state = DECODE;
                break;
            case DECODE: // microstate 32
                
                opcode = (unsigned short) OPCODE(cpu->ir);
                Rd = DSTREG(cpu->ir);
                Rs1 = SRCREG(cpu->ir);
                Rs2 = SRCREG2(cpu->ir);
               
                //sext immediate 9
                immed9 = SEXTPCOFFSET9(cpu->ir);
                
                ben = (cpu->conCodes.n && NBIT(cpu->ir)) + (cpu->conCodes.z && ZBIT(cpu->ir)) + (cpu->conCodes.p && PBIT(cpu->ir));
                
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
                        if(cpu->reg_file[Rs1] == RETURN_REG) //ret case
                        {
                            cpu->pc = cpu->reg_file[RETURN_REG]; //go to register 7?
                        }
                        else
                        {
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
						if(!NBIT(cpu->ir)) //check if JSRR incase if we implement JSR
                        {
                            //pc = baseR
                            cpu->pc = cpu->reg_file[Rs1]; 
                        }
                     //implementing JSR here{
                            //cpu->pc = cpu->pc + SEXT(PCOffset11);
                        
                        break;
                    //case RET: same case as JMP


                    case BR:
                        if(ben) {
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
                        if(IMMBIT(cpu->ir)) {
                           // sext immediate value
                           cpu->alu_b = SEXTIMMVAL(cpu->ir);
                        } else {
                           cpu->alu_b = cpu->reg_file[Rs2];
                        }
                        break;
                    case AND:
                        cpu->alu_a = cpu->reg_file[Rs1];
                        // Check bit 5
                        if(IMMBIT(cpu->ir)) {
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
                            if(trap(cpu, win)) {
							   // end current program
							   cpu->pc = orig;
                               state = FETCH;
							   programLoaded = false;
							   programRunning = false;
							   displayBoldMessage(win, "Program HALT. Press any key to continue.");
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
                        if(IMMBIT(cpu->ir))//if 1, push case
                        {
                            cpu->mdr = memory[cpu->reg_file[Rd]]--; //make room on the stack R6
                            //memory[r6] <- Br
                            memory[cpu->reg_file[Rd]] = cpu->reg_file[Rd];//push item on stack
                            

                        }
                        else //pop case
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

int main(int argc, char* argv[]) {
    char *temp;

	// initialize resources
    CPU_p cpu = (CPU_p)malloc(sizeof(CPU_s));
	DEBUG_WIN_p win = (DEBUG_WIN_p)malloc(sizeof(DEBUG_WIN_s));
    initializeWindows(win);
	
	// print
	reprintScreen(win, cpu ,memory, false);
	
	controller(cpu, win);

	// free resources
    endWindows(win);
	free(win);
    free(cpu);
}