/* config.c - MemTest-86  Version 3.0
 *
 * Released under version 2 of the Gnu Public License.
 * By Chris Brady, cbrady@sgi.com
 * ----------------------------------------------------
 * MemTest86+ V1.15 Specific code (GPL V2.0)
 * By Samuel DEMEULEMEESTER, sdemeule@memtest.org
 * http://www.x86-secret.com - http://www.memtest.org
 */

#include "test.h"
#include "screen_buffer.h"
#include "controller.h"
#define ITER 20

extern int bail, beepmode;
extern struct tseq tseq[];
/* extern struct vars *v; */
extern short e820_nr;
extern char memsz_mode;

char save[POP_H][POP_W];

void get_config()
{
	int flag = 0, sflag = 0, i, prt = 0;
        int reprint_screen = 0;
	ulong page;

	popup();
	wait_keyup();
	while(!flag) {
		cprint(POP_Y+1,  POP_X+2, "Configuration:");
		cprint(POP_Y+3,  POP_X+6, "(1) Cache Mode");
		cprint(POP_Y+4,  POP_X+6, "(2) Test Selection");
		cprint(POP_Y+5,  POP_X+6, "(3) Address Range");
		cprint(POP_Y+6,  POP_X+6, "(4) Memory Sizing");
		cprint(POP_Y+7,  POP_X+6, "(5) Error Summary");
		cprint(POP_Y+8,  POP_X+6, "(6) Error Report Mode");
		cprint(POP_Y+9,  POP_X+6, "(7) ECC Mode"); 
		cprint(POP_Y+10, POP_X+6, "(8) Restart Test");
		cprint(POP_Y+11, POP_X+6, "(9) Reprint Screen");
		cprint(POP_Y+12, POP_X+6, "(0) Exit");

		/* Wait for key release */
		/* Fooey! This nuts'es up the serial input. */
		sflag = 0;
		switch(get_key()) {
		case 2:
			/* 1 - Set cache mode */
			popclear();
			cprint(POP_Y+1, POP_X+2, "Cache Mode:");
			cprint(POP_Y+3, POP_X+6, "(1) Test Controlled");
			cprint(POP_Y+4, POP_X+6, "(2) Always On");
			cprint(POP_Y+5, POP_X+6, "(3) Always Off");
			cprint(POP_Y+6, POP_X+6, "(0) Cancel");
			cprint(POP_Y+3+v->cache_flag, POP_X+5, ">");
			wait_keyup();
			while (!sflag) {
				switch(get_key()) {
				case 2:
					/* test controled */
					v->cache_flag = 0;
					set_cache(tseq[v->test].cache);
					find_ticks();
					sflag++;
					break;
				case 3:
					/* Cache on */
					v->cache_flag = 1;
					if (tseq[v->test].cache == 0)  {
						bail++;
					}
					set_cache(1);
					find_ticks();
					sflag++;
					break;
				case 4:
					/* Cache off */
					v->cache_flag = 2;
					if (tseq[v->test].cache == 1)  {
						bail++;
					}
					find_ticks();
					sflag++;
					break;
				case 11:
				case 57:
					sflag++;
					break;
				}
			}
			popclear();
			break;
		case 3:
			/* 2 - Test Selection */
			popclear();
			cprint(POP_Y+1, POP_X+2, "Test Selection:");
			cprint(POP_Y+3, POP_X+6, "(1) Default Tests");
			cprint(POP_Y+4, POP_X+6, "(2) Extended Tests");
			cprint(POP_Y+5, POP_X+6, "(3) All Tests");
			cprint(POP_Y+6, POP_X+6, "(4) Skip Current Test");
			cprint(POP_Y+7, POP_X+6, "(5) Select Test");
			cprint(POP_Y+8, POP_X+6, "(6) Print mode");
			cprint(POP_Y+9, POP_X+6, "(0) Cancel");
			if (v->testsel < 0) {
				cprint(POP_Y+3+v->xtst_flag, POP_X+5, ">");
			} else {
				cprint(POP_Y+7, POP_X+5, ">");
			}
			wait_keyup();
			while (!sflag) {
				switch(get_key()) {
				case 2:
					/* Default */
					v->xtst_flag = 0;
					if (v->test > DEFTESTS) {
						bail++;
					}
					v->testsel = -1;
					find_ticks();
					sflag++;
					cprint(LINE_INFO, COL_TST, "Std");
					break;
				case 3:
					/* Extended */
					v->xtst_flag = 1;
					if (v->test <= DEFTESTS) {
						bail++;
					}
					v->testsel = -1;
					find_ticks();
					sflag++;
					cprint(LINE_INFO, COL_TST, "Ext");
					break;
				case 4:
					/* All */
					v->xtst_flag = 2;
					v->testsel = -1;
					find_ticks();
					sflag++;
					cprint(LINE_INFO, COL_TST, "All");
					break;
				case 5:
					/* Skip test */
					bail++;
					sflag++;
					break;
				case 6:
					/* Select test */
					popclear();
					cprint(POP_Y+1, POP_X+2,
						"Test Selection:");
					cprint(POP_Y+3, POP_X+4,
						"Test Number [0-12]: ");
					i = getval(POP_Y+3, POP_X+24, 0);
					if (i <= 12) {
						if (i != v->testsel) {
							v->pass = 0;
							v->ecount = 0;
						}
						v->testsel = i;
					}
					v->test = -1;
					v->pass = -1;
					find_ticks();
					sflag++;
					bail++;
					cprint(LINE_INFO, COL_TST, "#");
					dprint(LINE_INFO, COL_TST+1, i, 2, 1);
					break;
				case 11:
				case 57:
					sflag++;
					break;
				}
			}
			popclear();
			break;
		case 4:
			/* 3 - Address Range */
			popclear();
			cprint(POP_Y+1, POP_X+2, "Test Address Range:");
			cprint(POP_Y+3, POP_X+6, "(1) Set Lower Limit");
			cprint(POP_Y+4, POP_X+6, "(2) Set Upper Limit");
			cprint(POP_Y+5, POP_X+6, "(3) Test All Memory");
			cprint(POP_Y+6, POP_X+6, "(0) Cancel");
			wait_keyup();
			while (!sflag) {
				switch(get_key()) {
				case 2:
					/* Lower Limit */
					popclear();
					cprint(POP_Y+2, POP_X+4,
						"Lower Limit: ");
					cprint(POP_Y+4, POP_X+4,
						"Current: ");
					aprint(POP_Y+4, POP_X+13, v->plim_lower);
					cprint(POP_Y+6, POP_X+4,
						"New: ");
					page = getval(POP_Y+6, POP_X+9, 12);
					if (page + 1 <= v->plim_upper) {
						v->plim_lower = page;
						bail++;
					}
					adj_mem();
					find_ticks();
					sflag++;
					break;
				case 3:
					/* Upper Limit */
					popclear();
					cprint(POP_Y+2, POP_X+4,
						"Upper Limit: ");
					cprint(POP_Y+4, POP_X+4,
						"Current: ");
					aprint(POP_Y+4, POP_X+13, v->plim_upper);
					cprint(POP_Y+6, POP_X+4,
						"New: ");
					page = getval(POP_Y+6, POP_X+9, 12);
					if  (page - 1 >= v->plim_lower) {
						v->plim_upper = page;
						bail++;
					}
					adj_mem();
					find_ticks();
					sflag++;
					break;
				case 4:
					/* All of memory */
					v->plim_lower = 0;
					v->plim_upper = v->pmap[v->msegs - 1].end;
					bail++;
					adj_mem();
					find_ticks();
					sflag++;
					break;
				case 11:
				case 57:
					/* 0/CR - Cancel */
					sflag++;
					break;
				}
			}
			popclear();
			break;
		case 5:
			/* 4 - Memory Sizing */
			popclear();
			cprint(POP_Y+1, POP_X+2, "Memory Sizing:");
			cprint(POP_Y+3, POP_X+6, "(1) BIOS - Std");
			if (e820_nr) {
				cprint(POP_Y+4, POP_X+6, "(2) BIOS - All");
				cprint(POP_Y+5, POP_X+6, "(3) Probe");
				cprint(POP_Y+6, POP_X+6, "(0) Cancel");
				cprint(POP_Y+2+memsz_mode, POP_X+5, ">");
			} else {
				cprint(POP_Y+4, POP_X+6, "(3) Probe");
				cprint(POP_Y+5, POP_X+6, "(0) Cancel");
				if (memsz_mode == SZ_MODE_BIOS) {
					cprint(POP_Y+3, POP_X+5, ">");
				} else {
					cprint(POP_Y+4, POP_X+5, ">");
				}
			}
			wait_keyup();
			while (!sflag) {
				switch(get_key()) {
				case 2:
					memsz_mode = SZ_MODE_BIOS;
					wait_keyup();
					restart();
/*
					mem_size();
					v->test = 0;
					v->pass = 0;
					v->total_ticks = 0;
					bail++;
					sflag++;
*/
					break;
				case 3:
					memsz_mode = SZ_MODE_BIOS_RES;
					wait_keyup();
					restart();
/*
					mem_size();
					v->test = 0;
					v->pass = 0;
					v->total_ticks = 0;
					bail++;
					sflag++;
*/
					break;
				case 4:
					memsz_mode = SZ_MODE_PROBE;
					wait_keyup();
					restart();
/*
					mem_size();
					v->test = 0;
					v->pass = 0;
					v->total_ticks = 0;
					bail++;
					sflag++;
*/
					break;
				case 11:
				case 57:
					/* 0/CR - Cancel */
					sflag++;
					break;
				}
			}
			popclear();
			break;
		case 6:
			/* 5 - Show error summary */
			popclear();
			for (i=0; tseq[i].msg != NULL; i++) {
				cprint(POP_Y+1+i, POP_X+2, "Test:");
				dprint(POP_Y+1+i, POP_X+8, i, 2, 1);
				cprint(POP_Y+1+i, POP_X+12, "Errors:");
				dprint(POP_Y+1+i, POP_X+20, tseq[i].errors,
					5, 1);
			}
			wait_keyup();
			while (get_key() == 0);
			popclear();
			break;
		case 7:
			/* 6 - Printing Mode */
			popclear();
			cprint(POP_Y+1, POP_X+2, "Printing Mode:");
			cprint(POP_Y+3, POP_X+6, "(1) Individual Errors");
			cprint(POP_Y+4, POP_X+6, "(2) BadRAM Patterns");
			cprint(POP_Y+5, POP_X+6, "(3) Error Counts Only");
			cprint(POP_Y+6, POP_X+6, "(4) Beep on Error");
			cprint(POP_Y+7, POP_X+6, "(0) Cancel");
			cprint(POP_Y+3+v->printmode, POP_X+5, ">");
			if (beepmode) { cprint(POP_Y+6, POP_X+5, ">"); }
			wait_keyup();
			while (!sflag) {
				switch(get_key()) {
				case 2:
					/* Separate Addresses */
					v->printmode=PRINTMODE_ADDRESSES;
					v->eadr = 0;
					sflag++;
					break;
				case 3:
					/* BadRAM Patterns */
					v->printmode=PRINTMODE_PATTERNS;
					sflag++;
					prt++;
					break;
				case 4:
					/* Error Counts Only */
					v->printmode=PRINTMODE_NONE;
					sflag++;
					break;
				case 5:
					/* Set Beep On Error mode */
					beepmode = !beepmode;
					sflag++;
					break;
				case 11:
				case 57:
					/* 0/CR - Cancel */
					sflag++;
					break;
				}
			}
			popclear();
			break;
		case 8:
			/* 7 - ECC Polling Mode */
			popclear();
			cprint(POP_Y+1, POP_X+2, "ECC Polling Mode:");
			cprint(POP_Y+3, POP_X+6, "(1) Recommended");
			cprint(POP_Y+4, POP_X+6, "(2) On");
			cprint(POP_Y+5, POP_X+6, "(3) Off");
			cprint(POP_Y+6, POP_X+6, "(0) Cancel");
			wait_keyup();
			while(!sflag) {
				switch(get_key()) {
				case 2:
					set_ecc_polling(-1);
					sflag++;
					break;
				case 3:
					set_ecc_polling(1);
					sflag++;
					break;
				case 4:
					set_ecc_polling(0);
					sflag++;
					break;
				case 11:
				case 57:
					/* 0/CR - Cancel */
					sflag++;
					break;
				}
			}
			popclear();
			break;
		case 9:
			wait_keyup();
			restart();
			break;
                case 10:
			reprint_screen = 1;
			flag++;
			break;
		case 11:
		case 57:
		case 28:
			/* 0/CR/SP - Cancel */
			flag++;
			break;
		}
	}
	popdown();
	if (prt) {
		printpatn();
	}
        if (reprint_screen){
            tty_print_screen();
        }
}


void popup()
{
	int i, j;
	char *pp;
	
	for (i=POP_Y; i<POP_Y + POP_H; i++) { 
		for (j=POP_X; j<POP_X + POP_W; j++) { 
			pp = (char *)(SCREEN_ADR + (i * 160) + (j * 2));
                        save[i-POP_Y][j-POP_X] = get_scrn_buf(i, j);	/* Save screen */
                        set_scrn_buf(i, j, ' ');
			*pp = ' ';		/* Clear */                        
			pp++;
			*pp = 0x07;		/* Change Background to black */
		}
	}
        tty_print_region(POP_Y, POP_X, POP_Y+POP_H, POP_X+POP_W);
}

void popdown()
{
	int i, j;
	char *pp;
	
	for (i=POP_Y; i<POP_Y + POP_H; i++) { 
		for (j=POP_X; j<POP_X + POP_W; j++) { 
			pp = (char *)(SCREEN_ADR + (i * 160) + (j * 2));
			*pp = save[i-POP_Y][j-POP_X];	/* Restore screen */
                        set_scrn_buf(i, j, save[i-POP_Y][j-POP_X]);
			pp++;
			*pp = 0x17;		/* Reset background color */
		}
	}
        tty_print_region(POP_Y, POP_X, POP_Y+POP_H, POP_X+POP_W);
}

void popclear()
{
	int i, j;
	char *pp;
	
	for (i=POP_Y; i<POP_Y + POP_H; i++) { 
		for (j=POP_X; j<POP_X + POP_W; j++) { 
			pp = (char *)(SCREEN_ADR + (i * 160) + (j * 2));
			*pp = ' ';		/* Clear screen */
                        set_scrn_buf(i, j, ' ');
			pp++;
		}
	}
        tty_print_region(POP_Y, POP_X, POP_Y+POP_H, POP_X+POP_W);
}

void clear_screen()
{
	int i;
	volatile char *pp;

	for(i=0, pp=(char *)(SCREEN_ADR); i<80*24; i++) {
		*pp++ = ' ';
		*pp++ = 0x07;
	}
}

void adj_mem(void)
{
	int i;

	v->selected_pages = 0;
	for (i=0; i< v->msegs; i++) {
		/* Segment inside limits ? */
		if (v->pmap[i].start >= v->plim_lower &&
				v->pmap[i].end <= v->plim_upper) {
			v->selected_pages += (v->pmap[i].end - v->pmap[i].start);
			continue;
		}
		/* Segment starts below limit? */
		if (v->pmap[i].start < v->plim_lower) {
			/* Also ends below limit? */
			if (v->pmap[i].end < v->plim_lower) {
				continue;
			}
			
			/* Ends past upper limit? */
			if (v->pmap[i].end > v->plim_upper) {
				v->selected_pages += 
					v->plim_upper - v->plim_lower;
			} else {
				/* Straddles lower limit */
				v->selected_pages += 
					(v->pmap[i].end - v->plim_lower);
			}
			continue;
		}
		/* Segment ends above limit? */
		if (v->pmap[i].end > v->plim_upper) {
			/* Also starts above limit? */
			if (v->pmap[i].start > v->plim_upper) {
				continue;
			}
			/* Straddles upper limit */
			v->selected_pages += 
				(v->plim_upper - v->pmap[i].start);
		}
	}
}
