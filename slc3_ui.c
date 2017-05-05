/*
TCSS372 - Computer Architecture
Problem #4
Group Members: 
Shaun Coleman
Phansa Chaonpoj
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
	 
	 mvwprintw(win->mainWin, PC_LABEL_Y_X, "PC:");
	 mvwprintw(win->mainWin, IR_LABEL_Y_X, "IR:");
	 mvwprintw(win->mainWin, A_LABEL_Y_X, "A:");
	 mvwprintw(win->mainWin, B_LABEL_Y_X, "B:");
	 mvwprintw(win->mainWin, MAR_LABEL_Y_X, "MAR:");
	 mvwprintw(win->mainWin, MDR_LABEL_Y_X, "MDR:");
	 mvwprintw(win->mainWin, CC_LABEL_Y_X, "CC:");
	 mvwprintw(win->mainWin, N_LABEL_Y_X, "N:");
	 mvwprintw(win->mainWin, Z_LABEL_Y_X, "Z:");
	 mvwprintw(win->mainWin, P_LABEL_Y_X, "P:");
	 
	 // Menu
	 mvwprintw(win->mainWin, MENU_Y_X, "Select: 1) Load, 3) Step, 4) Run, 5) Display Mem, 9) Exit");

	 // Prompt
	 mvwprintw(win->mainWin, PROMPT_Y_X, "> ");
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

void updateRegisterValues(DEBUG_WIN_p win, CPU_p cpu) {
	 for (int i = 0; i < MAX_REG; i++) {
	    mvwprintw(win->mainWin, REG_MEM_START_Y + i, REG_VAL_X, HEX_OUT_FORMAT, cpu->reg_file[i]); 
	 }
	 
	 mvwprintw(win->mainWin, PC_VAL_Y_X, HEX_OUT_FORMAT, cpu->pc);
	 mvwprintw(win->mainWin, IR_VAL_Y_X, HEX_OUT_FORMAT, cpu->ir);
	 mvwprintw(win->mainWin, A_VAL_Y_X, HEX_OUT_FORMAT, cpu->alu_a);
	 mvwprintw(win->mainWin, B_VAL_Y_X, HEX_OUT_FORMAT, cpu->alu_b);
	 mvwprintw(win->mainWin, MAR_VAL_Y_X, HEX_OUT_FORMAT, cpu->mar);
	 mvwprintw(win->mainWin, MDR_VAL_Y_X, HEX_OUT_FORMAT, cpu->mdr);
	 mvwprintw(win->mainWin, N_VAL_Y_X, "%d", cpu->conCodes.n);
	 mvwprintw(win->mainWin, Z_VAL_Y_X, "%d", cpu->conCodes.z);
	 mvwprintw(win->mainWin, P_VAL_Y_X, "%d", cpu->conCodes.p);
}

void reprintBoarder(DEBUG_WIN_p win) {
    box(win->mainWin, 0, 0);
	mvwprintw(win->mainWin, TITLE_Y_X, "Welcome to the LC-3 Simulator Simulator");
	move(PROMPT_DISPLAY_Y, PROMPT_DISPLAY_X);
}

void reprintScreen(DEBUG_WIN_p win, CPU_p cpu, unsigned short *memory, char programLoaded) {
	updateRegisterValues(win, cpu);
	updateMemory(win, memory, win->memAddress);
	if (programLoaded) {
	    printArrow(win, cpu);
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
	initscr();				/* start the curses mode */
	raw();
	noecho();
	getmaxyx(stdscr,max_y,max_x);
	x_pos = (HALF(max_x) - HALF(WIN_WIDTH)) < 0 ? 0 : (HALF(max_x) - HALF(WIN_WIDTH));
	y_pos = (HALF(max_y) - HALF(TOTAL_WIN_HEIGHT)) < 0 ? 0 : (HALF(max_y) - HALF(TOTAL_WIN_HEIGHT)) - 1;
	win->mainWin = newwin(MAIN_WIN_HEIGHT, WIN_WIDTH, y_pos, x_pos);
	win->ioWin = newwin(IO_WIN_HEIGHT, WIN_WIDTH, y_pos+MAIN_WIN_HEIGHT, x_pos);
	win->memAddress = DEFAULT_MEM_ADDRESS;
	refresh();
	 
	start_color();			/* Start color 			*/
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
	    } else {
		    clearArrow(win);
		}
		updateRegisterValues(win, cpu);
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
	wattron(win->mainWin, A_STANDOUT);
	mvwprintw(win->mainWin, PROMPT_DISPLAY_Y, PROMPT_DISPLAY_X, message);
	wattroff(win->mainWin, A_STANDOUT);
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
