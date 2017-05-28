/*
TCSS372 - Computer Architecture
Project LC3 
Group Members: 
Shaun Coleman
Phansa Chaonpoj
Joshua Meigs
*/

#include "slc3_ui.h"

void printLabels(DEBUG_WIN_p win) {
     //Titles
     box(win->mainWin, 0, 0);
     mvwprintw(win->mainWin, TITLE_Y_X, "Welcome to the LC-3 Simulator Simulator");
     mvwprintw(win->mainWin, REG_TITLE_Y_X, "Registers");
     mvwprintw(win->mainWin, MEM_TITLE_Y_X, "Memory");
     
     for (int i = 0; i < MAX_REG; i++) {
        mvwprintw(win->mainWin, REG_MEM_START_Y + i, REG_LABEL_X, REG_OUT_FORMAT, i); 
     }

	 mvwprintw(win->mainWin, FBUFF_LABEL_Y_X, "FBUFF");
     mvwprintw(win->mainWin, DBUFF_LABEL_Y_X, "DBUFF");
     mvwprintw(win->mainWin, EBUFF_LABEL_Y_X, "EBUFF");
     mvwprintw(win->mainWin, MBUFF_LABEL_Y_X, "MBUFF");
     mvwprintw(win->mainWin, STORE_LABEL_Y_X, "STORE:");
       
     mvwprintw(win->mainWin, CC_LABEL_Y_X, "CC:");
     mvwprintw(win->mainWin, N_LABEL_Y_X, "N:");
     mvwprintw(win->mainWin, Z_LABEL_Y_X, "Z:");
     mvwprintw(win->mainWin, P_LABEL_Y_X, "P:");
     
     // Menu
     mvwprintw(win->mainWin, MENU1_Y_X, "Select: 1)Load, 2)Save, 3)Step, 4)Run, 5)Display, 6)Edit, 7)");
     mvwprintw(win->mainWin, MENU2_Y_X, "8)Un/Set Brkpt, 9)Exit > ");

}      

void printIoLabels(DEBUG_WIN_p win) {
     box(win->ioWin, 0, 0);
     mvwprintw(win->ioWin, IO_TITLE_Y_X, "I/O Window");
}

void updateMemory(DEBUG_WIN_p win, unsigned short* memory, unsigned short mem_index) {
     win->memAddress = mem_index;
     for (int i = 0; i < MAX_MEM; i++) {
        mvwprintw(win->mainWin, REG_MEM_START_Y + i, MEM_LABEL_X, HEX_OUT_LABEL, win->memAddress + i);
        mvwprintw(win->mainWin, REG_MEM_START_Y + i, MEM_VAL_X, HEX_OUT_FORMAT, memory[win->memAddress + i]);
     }
     
}

void clearBufferBox(DEBUG_WIN_p win, int startY, int startX) {
	for (int i = startY; i < startY+4; i++) {
		for (int j = startX; j < startX+33; j++) {
			mvwaddch(win->mainWin, i, j, ' ');
		}
	}
}

void clearArrow(DEBUG_WIN_p win) {
    for(int i = 0; i<MAX_MEM; i++) {    
        mvwprintw(win->mainWin, REG_MEM_START_Y + i, ARROW_X, "  ");
    }
}

void printArrow(DEBUG_WIN_p win, CPU_p cpu) {
    int offset =  cpu->pc - win->memAddress;

    clearArrow(win);
    
    if(offset >= 0 && offset < MAX_MEM) {
        wattron(win->mainWin, A_STANDOUT); 
        mvwprintw(win->mainWin, REG_MEM_START_Y + offset, ARROW_X, "->");
        wattroff(win->mainWin, A_STANDOUT);
    } 
}

void printBreakPoint(DEBUG_WIN_p win, CPU_p cpu) {
    int i = 0;
    int offset;
    
    for(i = 0; i < (MAXBREAK - win->breakpoints->emptySpaces); i++) {
        offset =  win->breakpoints->breakpointArr[i] - win->memAddress;
        if(offset >= 0 && offset < MAX_MEM) {
            wattron(win->mainWin, A_STANDOUT); 
            mvwprintw(win->mainWin, REG_MEM_START_Y + offset, ARROW_X+1, "O");//such magic
            wattroff(win->mainWin, A_STANDOUT);          
        } 
    }
}

void updateRegisterValues(DEBUG_WIN_p win, CPU_p cpu) {
     for (int i = 0; i < MAX_REG; i++) {
        mvwprintw(win->mainWin, REG_MEM_START_Y + i, REG_VAL_X, HEX_OUT_FORMAT, cpu->reg_file[i]); 
     }
	 
     mvwprintw(win->mainWin, N_VAL_Y_X, "%d", cpu->conCodes.n);
     mvwprintw(win->mainWin, Z_VAL_Y_X, "%d", cpu->conCodes.z);
     mvwprintw(win->mainWin, P_VAL_Y_X, "%d", cpu->conCodes.p);
}

void updateFBuffer(DEBUG_WIN_p win, CPU_p cpu){
	if (!cpu->fbuff.ir) {
        mvwprintw(win->mainWin, FBUFF_PC_LBL_Y_X, "IR:");
		mvwprintw(win->mainWin, FBUFF_PC_VAL_Y_X+2, "NOP");
	} else {
	    mvwprintw(win->mainWin, FBUFF_PC_LBL_Y_X, "PC:");
        mvwprintw(win->mainWin, FBUFF_IR_LBL_Y_X, "IR:");
		mvwprintw(win->mainWin, FBUFF_PC_VAL_Y_X, HEX_OUT_FORMAT, cpu->fbuff.pc);
        mvwprintw(win->mainWin, FBUFF_IR_VAL_Y_X, HEX_OUT_FORMAT, cpu->fbuff.ir);
	}
}

void updateDBuffer(DEBUG_WIN_p win, CPU_p cpu){
	if (!cpu->dbuff.pc && !cpu->dbuff.op) {
		mvwprintw(win->mainWin, BUFF_LBL_START_Y, OP_LBL_X, "OP:");
		mvwprintw(win->mainWin, DBUFF_OP_VAL_Y_X, "NOP");
	} else {
		switch (cpu->dbuff.op) {
			case AND:
			case ADD:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y, OP_LBL_X, "OP: DR: OPN1: OPN2:");
				mvwprintw(win->mainWin, DBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->dbuff.op));
                mvwprintw(win->mainWin, DBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->dbuff.dr));
                mvwprintw(win->mainWin, DBUFF_OPN1_VAL_Y_X, HEX_OUT_FORMAT, cpu->dbuff.opn1);
                mvwprintw(win->mainWin, DBUFF_OPN2_VAL_Y_X, HEX_OUT_FORMAT, cpu->dbuff.opn2);
				break;
			case NOT:
			    mvwprintw(win->mainWin, BUFF_LBL_START_Y, OP_LBL_X, "OP: DR: OPN1:");
				mvwprintw(win->mainWin, DBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->dbuff.op));
                mvwprintw(win->mainWin, DBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->dbuff.dr));
                mvwprintw(win->mainWin, DBUFF_OPN1_VAL_Y_X, HEX_OUT_FORMAT, cpu->dbuff.opn1);
				break;
			case BR:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y, OP_LBL_X, "OP: NZP: OFF:");
				mvwprintw(win->mainWin, DBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->dbuff.op));
                mvwprintw(win->mainWin, DBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, cpu->dbuff.dr);
                mvwprintw(win->mainWin, DBUFF_OPN1_VAL_Y_X+1, HEX_OUT_FORMAT, cpu->dbuff.opn1);
				break;
			case LD:
			case LEA:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y, OP_LBL_X, "OP: DR: OFF:");
				mvwprintw(win->mainWin, DBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->dbuff.op));
                mvwprintw(win->mainWin, DBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->dbuff.dr));
                mvwprintw(win->mainWin, DBUFF_OPN1_VAL_Y_X, HEX_OUT_FORMAT, cpu->dbuff.opn1);
				break;
			case ST:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y, OP_LBL_X, "OP: SRV: OFF:");
				mvwprintw(win->mainWin, DBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->dbuff.op));
                mvwprintw(win->mainWin, DBUFF_DR_VAL_Y_X-1, HEX_OUT_FORMAT, cpu->dbuff.dr);
                mvwprintw(win->mainWin, DBUFF_OPN1_VAL_Y_X+2, HEX_OUT_FORMAT, cpu->dbuff.opn1);
				break;
			case STR:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y, OP_LBL_X, "OP: SRV: RVAL: OFF:");
				mvwprintw(win->mainWin, DBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->dbuff.op));
                mvwprintw(win->mainWin, DBUFF_DR_VAL_Y_X-1, HEX_OUT_FORMAT, cpu->dbuff.dr);
                mvwprintw(win->mainWin, DBUFF_OPN1_VAL_Y_X+2, HEX_OUT_FORMAT, cpu->dbuff.opn1);
				mvwprintw(win->mainWin, DBUFF_OPN2_VAL_Y_X+2, HEX_OUT_FORMAT, cpu->dbuff.opn2);;
				break;
			case LDR:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y, OP_LBL_X, "OP: DR: RVAL: OFF:");
				mvwprintw(win->mainWin, DBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->dbuff.op));
                mvwprintw(win->mainWin, DBUFF_DR_VAL_Y_X, HEX_OUT_FORMAT, MOD16(cpu->dbuff.dr));
                mvwprintw(win->mainWin, DBUFF_OPN1_VAL_Y_X, HEX_OUT_FORMAT, cpu->dbuff.opn1);
				mvwprintw(win->mainWin, DBUFF_OPN1_VAL_Y_X, HEX_OUT_FORMAT, cpu->dbuff.opn2);
				break;
			case TRAP:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y, OP_LBL_X, "OP: TRAP:");
				mvwprintw(win->mainWin, DBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->dbuff.op));
                mvwprintw(win->mainWin, DBUFF_DR_VAL_Y_X, HEX_OUT_FORMAT, cpu->dbuff.opn1);
				break;
			default:
			    mvwprintw(win->mainWin, BUFF_LBL_START_Y, OP_LBL_X, "OP: DR: OPN1: OPN2:");
			    mvwprintw(win->mainWin, DBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, cpu->dbuff.op%16);
                mvwprintw(win->mainWin, DBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, cpu->dbuff.dr%16);
                mvwprintw(win->mainWin, DBUFF_OPN1_VAL_Y_X, HEX_OUT_FORMAT, cpu->dbuff.opn1);
                mvwprintw(win->mainWin, DBUFF_OPN2_VAL_Y_X, HEX_OUT_FORMAT, cpu->dbuff.opn2);
			    break;
		}
	}
}

void updateEBuffer(DEBUG_WIN_p win, CPU_p cpu){
	if (!cpu->ebuff.pc && !cpu->ebuff.op) {
		mvwprintw(win->mainWin, BUFF_LBL_START_Y+4, OP_LBL_X, "OP:");
		mvwprintw(win->mainWin, EBUFF_OP_VAL_Y_X, "NOP");
	} else {
		switch (cpu->ebuff.op) {
			case AND:
			case ADD:
			case NOT:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+4, OP_LBL_X, "OP: DR: RESULT:");
				mvwprintw(win->mainWin, EBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->ebuff.op));
                mvwprintw(win->mainWin, EBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->ebuff.dr));
                mvwprintw(win->mainWin, EBUFF_RESULT_VAL_Y_X, HEX_OUT_FORMAT, cpu->ebuff.result);
				break;
			case BR:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+4, OP_LBL_X, "OP: NZP: PC+OFS:");
				mvwprintw(win->mainWin, EBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->ebuff.op));
                mvwprintw(win->mainWin, EBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, cpu->ebuff.dr);
                mvwprintw(win->mainWin, EBUFF_RESULT_VAL_Y_X, HEX_OUT_FORMAT, cpu->ebuff.result);
				break;
			case LD:
			case LEA:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+4, OP_LBL_X, "OP: DR: PC+OFS:");
				mvwprintw(win->mainWin, EBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->ebuff.op));
                mvwprintw(win->mainWin, EBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->ebuff.dr));
                mvwprintw(win->mainWin, EBUFF_RESULT_VAL_Y_X, HEX_OUT_FORMAT, cpu->ebuff.result);
				break;
			case ST:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+4, OP_LBL_X, "OP: SRV: PC+OFS:");
				mvwprintw(win->mainWin, EBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->ebuff.op));
                mvwprintw(win->mainWin, EBUFF_DR_VAL_Y_X-1, HEX_OUT_FORMAT, cpu->ebuff.dr);
                mvwprintw(win->mainWin, EBUFF_RESULT_VAL_Y_X+1, HEX_OUT_FORMAT, cpu->ebuff.result);
				break;
			case STR:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+4, OP_LBL_X, "OP: SRV: RG+OFS:");
				mvwprintw(win->mainWin, EBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->ebuff.op));
                mvwprintw(win->mainWin, EBUFF_DR_VAL_Y_X-1, HEX_OUT_FORMAT, cpu->ebuff.dr);
                mvwprintw(win->mainWin, EBUFF_RESULT_VAL_Y_X+1, HEX_OUT_FORMAT, cpu->ebuff.result);
				break;
			case LDR:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+4, OP_LBL_X, "OP: DR: RG+OFS:");
				mvwprintw(win->mainWin, EBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->ebuff.op));
                mvwprintw(win->mainWin, EBUFF_DR_VAL_Y_X, HEX_OUT_FORMAT, MOD16(cpu->ebuff.dr));
                mvwprintw(win->mainWin, EBUFF_RESULT_VAL_Y_X, HEX_OUT_FORMAT, cpu->ebuff.result);
				break;
			case TRAP:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+4, OP_LBL_X, "OP: TRAP:");
				mvwprintw(win->mainWin, EBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->ebuff.op));
                mvwprintw(win->mainWin, EBUFF_DR_VAL_Y_X-1, HEX_OUT_FORMAT, cpu->ebuff.result);
				break;
			default:
			    mvwprintw(win->mainWin, BUFF_LBL_START_Y+4, OP_LBL_X, "OP: DR: RESULT:");
				mvwprintw(win->mainWin, EBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->ebuff.op));
                mvwprintw(win->mainWin, EBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->ebuff.dr));
                mvwprintw(win->mainWin, EBUFF_RESULT_VAL_Y_X, HEX_OUT_FORMAT, cpu->ebuff.result);
			    break;
		}
	}
}

void updateMBuffer(DEBUG_WIN_p win, CPU_p cpu){
	if (!cpu->mbuff.pc && !cpu->mbuff.op) {
		mvwprintw(win->mainWin, BUFF_LBL_START_Y+8, OP_LBL_X, "OP:");
		mvwprintw(win->mainWin, MBUFF_OP_VAL_Y_X, "NOP");
	} else {
		switch (cpu->mbuff.op) {
			case AND:
			case ADD:
			case NOT:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+8, OP_LBL_X, "OP: DR: RESULT:");
				mvwprintw(win->mainWin, MBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->mbuff.op));
                mvwprintw(win->mainWin, MBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->mbuff.dr));
                mvwprintw(win->mainWin, MBUFF_RESULT_VAL_Y_X, HEX_OUT_FORMAT, cpu->mbuff.result);
				break;
			case BR:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+8, OP_LBL_X, "OP: NZP: PC+OFS:");
				mvwprintw(win->mainWin, MBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->mbuff.op));
                mvwprintw(win->mainWin, MBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, cpu->mbuff.dr);
                mvwprintw(win->mainWin, MBUFF_RESULT_VAL_Y_X, HEX_OUT_FORMAT, cpu->mbuff.result);
				break;
			case LD:
			case LEA:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+8, OP_LBL_X, "OP: DR: LDVAL:");
				mvwprintw(win->mainWin, MBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->mbuff.op));
                mvwprintw(win->mainWin, MBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->mbuff.dr));
                mvwprintw(win->mainWin, MBUFF_RESULT_VAL_Y_X, HEX_OUT_FORMAT, cpu->mbuff.result);
				break;
			case ST:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+8, OP_LBL_X, "OP: SRV: PC+OFS:");
				mvwprintw(win->mainWin, MBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->mbuff.op));
                mvwprintw(win->mainWin, MBUFF_DR_VAL_Y_X-1, HEX_OUT_FORMAT, cpu->mbuff.dr);
                mvwprintw(win->mainWin, MBUFF_RESULT_VAL_Y_X+1, HEX_OUT_FORMAT, cpu->mbuff.result);
				break;
			case STR:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+8, OP_LBL_X, "OP: SRV: RG+OFS:");
				mvwprintw(win->mainWin, MBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->mbuff.op));
                mvwprintw(win->mainWin, MBUFF_DR_VAL_Y_X-1, HEX_OUT_FORMAT, cpu->mbuff.dr);
                mvwprintw(win->mainWin, MBUFF_RESULT_VAL_Y_X+1, HEX_OUT_FORMAT, cpu->mbuff.result);

				break;
			case LDR:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+8, OP_LBL_X, "OP: DR: LDVAL:");
				mvwprintw(win->mainWin, MBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->mbuff.op));
                mvwprintw(win->mainWin, MBUFF_DR_VAL_Y_X, HEX_OUT_FORMAT, MOD16(cpu->mbuff.dr));
                mvwprintw(win->mainWin, MBUFF_RESULT_VAL_Y_X, HEX_OUT_FORMAT, cpu->mbuff.result);
				break;
			case TRAP:
				mvwprintw(win->mainWin, BUFF_LBL_START_Y+8, OP_LBL_X, "OP: TRAP:");
				mvwprintw(win->mainWin, MBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->mbuff.op));
                mvwprintw(win->mainWin, MBUFF_DR_VAL_Y_X-1, HEX_OUT_FORMAT, cpu->mbuff.result);
				break;
			default:
			    mvwprintw(win->mainWin, BUFF_LBL_START_Y+8, OP_LBL_X, "OP: DR: RESULT:");
				mvwprintw(win->mainWin, MBUFF_OP_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->mbuff.op));
                mvwprintw(win->mainWin, MBUFF_DR_VAL_Y_X, HEX_OUT_SINGLE, MOD16(cpu->mbuff.dr));
                mvwprintw(win->mainWin, MBUFF_RESULT_VAL_Y_X, HEX_OUT_FORMAT, cpu->mbuff.result);
			    break;
        }
	}
}

void updateStore(DEBUG_WIN_p win, CPU_p cpu) {
	if (cpu->opInStore == NOP_IN_STORE) {
		mvwprintw(win->mainWin, STORE_LABEL_Y_X+7, "NOP        ");
	} else {
		switch (cpu->opInStore) {
			case ADD:
			case AND:
			case NOT:
			case LD:
			case LDI:
			case LDR:
			case LEA:
			case JSR:
			case RSV:
				mvwprintw(win->mainWin, STORE_LABEL_Y_X+7, "x%.04X in R%d", cpu->valueInStore, cpu->dr_store%10);
				break;
			default:
				mvwprintw(win->mainWin, STORE_LABEL_Y_X+7, "-----------");
				break;
		}
	}
}

void printStallSymbols(DEBUG_WIN_p win, CPU_p cpu) {
	if(cpu->stalls[P_IF]) {
		wattron(win->mainWin, A_STANDOUT); 
        mvwprintw(win->mainWin, FBUFF_STALL_Y_X, " S ");
        wattroff(win->mainWin, A_STANDOUT);   
	} else {
        mvwprintw(win->mainWin, FBUFF_STALL_Y_X, "   ");
	}
	
	if(cpu->stalls[P_ID]) {
		wattron(win->mainWin, A_STANDOUT); 
        mvwprintw(win->mainWin, DBUFF_STALL_Y_X, " S ");
        wattroff(win->mainWin, A_STANDOUT); 
	} else {
		mvwprintw(win->mainWin, DBUFF_STALL_Y_X, "   ");
	}
	
	if(cpu->stalls[P_EX]) {
		wattron(win->mainWin, A_STANDOUT); 
        mvwprintw(win->mainWin, EBUFF_STALL_Y_X, " S ");
        wattroff(win->mainWin, A_STANDOUT); 
	} else {
		mvwprintw(win->mainWin, EBUFF_STALL_Y_X, "   ");
	}
	
	if(cpu->stalls[P_MEM]) {
		wattron(win->mainWin, A_STANDOUT); 
        mvwprintw(win->mainWin, MBUFF_STALL_Y_X, " S ");
        wattroff(win->mainWin, A_STANDOUT);
	} else {
		mvwprintw(win->mainWin, MBUFF_STALL_Y_X, "   ");
	}
}

void updateBufferValues(DEBUG_WIN_p win, CPU_p cpu) {
    clearBufferBox(win, FBUFF_CLEAR_START);
	printBox(win, FBUFF_BOARDER);
	updateFBuffer(win, cpu);
    
	clearBufferBox(win, DBUFF_CLEAR_START);
	printBox(win, DBUFF_BOARDER);
	updateDBuffer(win, cpu);
    
    clearBufferBox(win, EBUFF_CLEAR_START);
	printBox(win, EBUFF_BOARDER);
	updateEBuffer(win, cpu);
    
    clearBufferBox(win, MBUFF_CLEAR_START);
	printBox(win, MBUFF_BOARDER);
	updateMBuffer(win, cpu);
    
    updateStore(win, cpu);
	printStallSymbols(win, cpu);
}

void reprintBoarder(DEBUG_WIN_p win) {
    box(win->mainWin, 0, 0);
    mvwprintw(win->mainWin, TITLE_Y_X, "Welcome to the LC-3 Simulator Simulator");
    move(PROMPT_DISPLAY_Y, PROMPT_DISPLAY_X);
}

void reprintScreen(DEBUG_WIN_p win, CPU_p cpu, unsigned short *memory, char programLoaded) {
    updateRegisterValues(win, cpu);
    updateMemory(win, memory, win->memAddress);
    updateBufferValues(win, cpu);
    if (programLoaded) {
        printArrow(win, cpu);
        printBreakPoint(win, cpu);
    } else {
        clearArrow(win);
    }
    printLabels(win);
    printIoLabels(win);
    wrefresh(win->mainWin);
    wrefresh(win->ioWin);
}

void initializeWindows(DEBUG_WIN_p win) {
    unsigned short max_y, max_x, x_pos, y_pos;
    initscr();                /* start the curses mode */
    raw();
    noecho();
    getmaxyx(stdscr,max_y,max_x);
    x_pos = (HALF(max_x) - HALF(WIN_WIDTH)) < 0 ? 0 : (HALF(max_x) - HALF(WIN_WIDTH));
    y_pos = (HALF(max_y) - HALF(TOTAL_WIN_HEIGHT)) < 0 ? 0 : (HALF(max_y) - HALF(TOTAL_WIN_HEIGHT)) - 1;
    win->mainWin = newwin(MAIN_WIN_HEIGHT, WIN_WIDTH, y_pos, x_pos);
    win->ioWin = newwin(IO_WIN_HEIGHT, WIN_WIDTH, y_pos+MAIN_WIN_HEIGHT, x_pos);
    win->memAddress = DEFAULT_MEM_ADDRESS;
    refresh();
     
    start_color();            /* Start color */
    init_pair(CP_WHITE_BLUE, COLOR_WHITE, COLOR_BLUE);
    wbkgd(win->mainWin, COLOR_PAIR(CP_WHITE_BLUE));
    wbkgd(win->ioWin, COLOR_PAIR(CP_WHITE_BLUE)); 
    win->ioY = IO_START_Y;
    win->ioX = IO_START_X;
}

void updateScreen(DEBUG_WIN_p win, CPU_p cpu, unsigned short *memory, char programLoaded) {
    unsigned short cur_y, cur_x, x_pos, y_pos;// Check Screen Size
    getmaxyx(stdscr,cur_y,cur_x);
         
    // Update Window Position
    if (cur_x != win->maxX || cur_y != win->maxY) {
        clear();
        refresh();
        win->maxX = cur_x;
        win->maxY = cur_y;
        x_pos = (HALF(cur_x) - HALF(WIN_WIDTH)) < 0 ? 0 : (HALF(cur_x) - HALF(WIN_WIDTH));
        y_pos = (HALF(cur_y) - HALF(TOTAL_WIN_HEIGHT)) < 0 ? 0 : (HALF(cur_y) - HALF(TOTAL_WIN_HEIGHT)) - 1;
        wclear(win->mainWin);
        wresize(win->mainWin, MAIN_WIN_HEIGHT, WIN_WIDTH);
        wresize(win->ioWin, IO_WIN_HEIGHT, WIN_WIDTH);
        mvwin(win->mainWin, y_pos, x_pos);
        mvwin(win->ioWin, y_pos+MAIN_WIN_HEIGHT, x_pos);
        reprintScreen(win, cpu, memory, programLoaded);
        wrefresh(win->mainWin);
        wrefresh(win->ioWin);
    } else { // Keep same position & refresh
        updateMemory(win, memory, win->memAddress);
        if (programLoaded) {
            printArrow(win, cpu);
            printBreakPoint(win, cpu);
        } else {
            clearArrow(win);
        }
        updateRegisterValues(win, cpu);
        updateBufferValues(win, cpu);
        wrefresh(win->mainWin);
        wrefresh(win->ioWin);
        reprintBoarder(win);
    }
}

void endWindows(DEBUG_WIN_p win) {
     delwin(win->mainWin);
     delwin(win->ioWin);
     endwin();
}

void clearPrompt(DEBUG_WIN_p win) {
    wmove(win->mainWin, PROMPT_DISPLAY_Y, PROMPT_DISPLAY_X);
    wclrtoeol(win->mainWin);
    reprintBoarder(win);
}

void promptUser(DEBUG_WIN_p win, char* message, char* input) {
    echo();
    mvwprintw(win->mainWin, PROMPT_DISPLAY_Y, PROMPT_DISPLAY_X, message);
    
    // Prompt User
    wgetnstr(win->mainWin, input, INPUT_LIMIT);
         
    // Clear Prompt Section
    clearPrompt(win);
    noecho();
}

void displayBoldMessage(DEBUG_WIN_p win, char* message) {
    clearPrompt(win);
	wattron(win->mainWin, A_STANDOUT);
    mvwprintw(win->mainWin, PROMPT_DISPLAY_Y, PROMPT_DISPLAY_X, message);
    wattroff(win->mainWin, A_STANDOUT);
	wgetch(win->mainWin);
	clearPrompt(win);
}

void writeCharToIOWin(DEBUG_WIN_p win, unsigned short c) {
    if (c == NEWLINE) {
        win->ioY++;
        win->ioX = IO_START_X;
        
    } else {
    
        mvwaddch(win->ioWin, win->ioY, win->ioX, c);
        win->ioX++;
        
        if (win->ioX > WIN_WIDTH - 2) {
            win->ioY++;
            win->ioX = IO_START_X;
        }
    }
}
void clearIOWin(DEBUG_WIN_p win) {
    wclear(win->ioWin);
    printIoLabels(win);
    win->ioY = IO_START_Y;
    win->ioX = IO_START_X;
}

void printBox(DEBUG_WIN_p win, int y, int x, int height, int width) {
    mvwvline(win->mainWin, y ,x, 0, height);
    mvwvline(win->mainWin, y ,x+width, 0, height);
    mvwhline(win->mainWin, y ,x, 0, width);
    mvwhline(win->mainWin, y+height,x, 0, width);
    mvwaddch(win->mainWin, y, x, ACS_ULCORNER);
    mvwaddch(win->mainWin, y, x+width, ACS_URCORNER);
    mvwaddch(win->mainWin, y+height, x, ACS_LLCORNER);
    mvwaddch(win->mainWin, y+height, x+width, ACS_LRCORNER);
}
