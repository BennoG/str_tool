// Copyright (C) 2005  by Piotr He³ka (piotr.helka@nd.e-wro.pl)
// Linux C++ (not full) implementation of Borland's conio.h 
// v 1.01
// It uses Ncurses lib, so accept also its terms.



// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// ----------------------------- Brief description ------------------

// The library supports more or less in line:
//
// cgets()
// cputs()
// clreol()
// clrscr()
// cprintf()
// cscanf()
// getch() (chyba nie wszystkie kody tak jak w conio.h)
// getche()  
// gotoxy()
// kbhit()
// putch()
// textbackground()
// textcolor()
// wherex()
// wherey()
// window()
//
// compatible towards Linux conio.h -> DOS conio.h
// Will be preserved

// To compile
// $g++ nazwa_progsa.cpp -lncurses -o nazwa_progsa.o

// ------------------------------- define ---------------------

#ifndef __NCURSES_H
#include <ncurses.h>
#endif

#ifndef __CONIO_H
#define __CONIO_H
#endif

#define MAX_OKIEN 256

#define BLACK       0
#define RED         1
#define GREEN       2
#define BROWN       3
#define BLUE        4
#define MAGENTA     5
#define CYAN        6
#define LIGHTGRAY   7
#define DARKGRAY    0
#define LIGHTRED    1
#define LIGHTGREEN  2
#define YELLOW      3
#define LIGHTBLUE   4
#define PINK        5
#define LIGHTCYAN   6
#define WHITE       7

// -------------------------------- globals  ------------------

//int (* wsk_f)(void) = getch;

#undef getch
#define getch CURSgetch

#undef getche
#define getche CURSgetche


void initialize ();

class Startuj   //  Start class constructor and destructor will odpowiedzalni
{	public:     //  initialization settings for automagically;-)
Startuj(){ initialize (); }
~Startuj(){ endwin(); }
} Start;	    			// initialize !

typedef struct
{
	int 	xup;
	int 	yup;
	int 	xdown;
	int 	ydown;
	WINDOW*	okno;
} Okno;

bool	initializeDone = FALSE; // If already after initscr ()
int	znakSpecjalny = -1; //Needed to getch'a
int	n = 0; //The number of used boxes

short	colorTekst = COLOR_WHITE;
short	colorBackgr = COLOR_BLACK;
short	biezacaPara;

Okno	windowTable[MAX_OKIEN];	// Array of structures of active windows
WINDOW*	activeWindow = NULL;	// pointer on the active window



// ----------------------------- end global ------------

void initialize ()
{
	initscr();
	start_color(); // we gaan kleur gebruiken
	cbreak();	// geen control braak ?
	noecho();	//no echo
	//raw();	//nadpisywane i tak przez noecho
	keypad(stdscr, TRUE);
	scrollok(stdscr, TRUE);

	//the default dialog
	activeWindow = stdscr;
	initializeDone = TRUE;

	//We will create a 8x8 matrix of the background and text colors
	short kolor = 1;
	for(short i=0; i<8; i++)
	{
		for(short j=0; j<8; j++, kolor++)
		{
			init_pair(kolor,i,j);
			if(i == COLOR_WHITE && j == COLOR_BLACK)	
				//Set this black backgrounds and text bialey as standard
			{
				biezacaPara = kolor;
			}  
		}
	}

	wrefresh(activeWindow);
}

int simple_strlen(char* str)
{
	char* p;
	for(p = str; *p != 0; p++);
	return p-str;
}

void cputs(char* str)
{
	waddstr(activeWindow, str);
	wrefresh(activeWindow);
}

char* cgets(char* str)
{ // Do not know exactly how it works original f. Cgets because I have no reference to it
	if(str == NULL || *str == 0)
	{
		*(str+1) = 0;
		return NULL;
	}

	int max = (int)(*str);

	echo();

	if(wgetnstr(activeWindow, (str + 2), max) == ERR)
	{
		*(str+1) = 0;
		return NULL;
	}

	noecho();

	*(str+1) = (char)simple_strlen(str+2);

	return str+2;
}

void clreol()
{
	wclrtoeol(activeWindow);
	wrefresh(activeWindow);
}

void clrscr()
{
	if(!initializeDone) initialize ();
	wbkgd(activeWindow, COLOR_PAIR(biezacaPara));
	// You have to move them cursor? I do not think ..
	wclear(activeWindow);
}

int cprintf(char *fmt, ...)
	// Pure hardcore;-)
{
	if(!initializeDone) initialize ();

	va_list ap; 
	va_start(ap, fmt);

	int i = vwprintw(activeWindow,fmt, ap);	//jakie proste ;-)

	va_end(ap);

	wrefresh(activeWindow);

	return i;
}

int cscanf(char *fmt, ...)
{
	if(!initializeDone) initialize ();

	echo();

	va_list ap;
	va_start(ap, fmt);

	int i = vwscanw(activeWindow, fmt, ap);

	va_end(ap);

	wrefresh(activeWindow);
	noecho();

	return i;
}

int CURSgetch()
{
	if(!initializeDone) initialize ();

	int znak;

	if(znakSpecjalny>0) // The second part of the special character 0x00 and 0x ??
	{
		// Replace DOS code sign - conio.h
		znak = znakSpecjalny;
		znakSpecjalny = -1;

		return znak-265+59;
	}

	znak = wgetch(activeWindow);

	if(znak > 255) // we have a special character 0x00
	{
		znakSpecjalny = znak;
		return 0;
	}

	return znak;
}

int CURSgetche()
{
	echo();
	int znak = getch();
	noecho();
	return znak;
}

int gotoxy(int x, int y)
{
	if(!initializeDone) initialize ();
	wmove(activeWindow, y - 1, x - 1);
	return 0;
}

int kbhit()
{
	int znak;
	wtimeout(activeWindow, 0);
	znak = wgetch(activeWindow);
	//wtimeout(aktywneOkno, -1);
	nodelay(activeWindow, FALSE);
	if (znak == ERR) return 0;
	ungetch(znak);
	return 1;
}

int putch(int znak)
{
	wechochar(activeWindow,znak);
}

void textbackground(short kolor)
{
	if(!initializeDone) initialize ();
	colorBackgr = kolor%8;
	short k=1;
	for(short i=0; i<8; i++) // Search for the number of pairs of colors
	{
		for(short j=0; j<8; j++, k++)
		{
			if(colorTekst == i && colorBackgr == j)
			{
				biezacaPara = k;
				wbkgd(activeWindow, COLOR_PAIR(k));
			}
		}
	}

	wrefresh(activeWindow);
}

void textcolor(short kolor)
{
	if(!initializeDone) initialize ();
	colorTekst = kolor%8;

	short k=1;
	for(short i=0; i<8; i++)  // Search for the number of pairs of colors
	{
		for(short j=0; j<8; j++, k++)
		{
			if(colorTekst == i && colorBackgr == j)
			{
				biezacaPara = k;
				wcolor_set(activeWindow,k, NULL);
			}
		}
	}

	wrefresh(activeWindow);
}

int wherex(void)
{
	if(!initializeDone) initialize ();
	int x, y;
	getyx(activeWindow, y, x);
	return x + 1;
}

int wherey(void)
{
	if(!initializeDone) initialize ();
	int x, y;
	getyx(activeWindow, y, x);
	return y + 1;
}

void window(int xup, int yup, int xdown, int ydown)
{
	if( xup<1 || yup<1 || xdown>COLS || ydown>LINES)
	{ // If wrong information is given ...
		xdown = COLS - xup;
		ydown = LINES - yup;
		//return;
	}

	bool istnieje = FALSE;

	if(!initializeDone) initialize ();

	/*
	There is an alternative solution to create new windows,
	when you create a new window with the previous window are removed,
	frees memory ie the window, the command delwin (nzw_okna) and forms
	see a new window by setting the default-it as the current one. However
	as it may take a lot of time and unnecessary spowolniac,
	bearing in mind the size of today's memory, I decided, use the
	array, which is kept here wsk. windows addresses and uses
	already allocated space. However, it can change at any time.
	*/

	for(int i=0; i<n && !istnieje; i++) // check to see if the specified window is not already
		// Previously been created
	{
		if( windowTable[i].xup == xup && windowTable[i].yup == yup
			&& windowTable[i].xdown == xdown && windowTable[i].ydown == ydown)
		{
			activeWindow = windowTable[i].okno;
			istnieje = TRUE;
			clrscr();
		}
	}

	if(!istnieje && n < MAX_OKIEN)  // if there is no such window, we create them
	{
		activeWindow = newwin(ydown - yup + 1, xdown - xup + 1, yup - 1, xup - 1);
		// Do not give head and displays exactly the conio.h

		// Save to an array of ...
		windowTable[n].okno = activeWindow;
		windowTable[n].xup = xup;
		windowTable[n].yup = yup;
		windowTable[n].xdown = xdown;
		windowTable[n].ydown = ydown;

		wcolor_set(activeWindow,biezacaPara, NULL);
		wbkgd(activeWindow, COLOR_PAIR(biezacaPara));

		// Restoration of key settings
		cbreak(); // Cache entry's exit
		noecho(); // Do not wish to display
		keypad(activeWindow, TRUE); // Full key codes
		scrollok(activeWindow, TRUE);

		n++;
	}

	wrefresh(activeWindow);

	return;
}
