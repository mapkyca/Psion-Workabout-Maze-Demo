/*
Maze : For the Psion workabout

In an effort to understand PLIB, I am writing a port for a maze game I wrote years ago
in pascal for DOS over to the Psion Workabout.

Since this is just an experiment into learning about doing file io and stuff on the workabout
it is very trivial. Currently only one level is supported by the loader, this would be a simple
calculation to seek to the beginning of a file, but currently I can not be bothered.

Author : Marcus Povey
Created : 7/8/2001
*/

#include <plib.h>
#include <p_cons.h>
#include <wskeys.h>

#define MAZEWIDTH 10 				// Characters wide
#define MAZEHEIGHT 7				// Characters high

#define MAZEFILE "a:\maze.dat"		// Location and name of the maze file

LOCAL_C VOID * g_pConsole = NULL;  	// Keyboard handle
LOCAL_C WORD g_KeybStat;			// Current keyboard status
LOCAL_C P_CON_KBREC g_KeyData;		// Storage structure for keypresses

extern void * winHandle;			// Pointer to default PLIB handle for console

TEXT g_Maze[MAZEWIDTH][MAZEHEIGHT]; // Storage for the maze
UINT g_CurrentLevel = 1;			// The current level

UINT g_PlayerX;						// Player position
UINT g_PlayerY;						// Player position

/** Initialise a console.
Inits a console, sets its size and redirects the standard plib console handle to point
to it. I am not sure if this is strictly correct, but it seems to be working.
*/
GLDEF_C INT InitConsole()
{
	INT ret;

	P_RECT rect;
    WORD func;

	if ((ret=p_open(&g_pConsole, "CON:", -1))!=0)
		p_printf(" Error %d. Failed to open keyboard channel", ret);

	rect.tl.x=rect.tl.y=0;
	rect.br.x=25;
	rect.br.y=9;
	func=P_SCR_WSET;
    p_iow4(g_pConsole,P_FSET,&func,&rect);

    winHandle = g_pConsole; // redirect default handle to the newly created console

	return ret;
}

/** Request a keypress.
Requests a key press from the console created above
*/
GLDEF_C VOID RequestKeyb()
{
	p_ioa4(g_pConsole, P_FREAD, &g_KeybStat, &g_KeyData);
}

/** Write some text to a specific area of the screen.
This has been shamelessly pinched from Events.c in sibosdk/demo
*/
GLDEF_C VOID Write(INT x,INT y,TEXT *pb,INT len)
{
    P_POINT pos;
    WORD func;

    pos.x=x;
    pos.y=y;
    func=P_SCR_POSA;
    p_iow4(g_pConsole,P_FSET,&func,&pos);
    p_write(g_pConsole,pb,len);
}

/** Load a maze from a file.
Loads maze Map from file FileName
*/
GLDEF_C INT LoadMaze(const char * FileName, UINT Map)
{
	VOID * f;
	LONG ppos = 0;
	INT n, na;

	if (p_open(&f, FileName, P_FSTREAM_TEXT|P_FSHARE|P_FRANDOM)!= 0)
	{
		p_printf("Could not data file %s", MAZEFILE); p_getch();
		p_exit(0);
	}


	// TODO:  fseek to map

	for (na=0; na<MAZEHEIGHT; na++)
	{
		for (n=0; n< MAZEWIDTH+2; n++)
		{
			if (n<MAZEWIDTH){
				INT ret;
				ret = p_read(f, &g_Maze[n][na], sizeof(TEXT));
				if ( ret<0)
					p_exit(0);
			}
			else
			{
				ppos = 1;
				p_seek(f,P_FCUR,&ppos);
			}

			// Set start point if applicable
			if (g_Maze[n][na]=='S')
			{
				g_PlayerX=n;
				g_PlayerY=na;
			}
		}
	}
	p_close(f);

	return 0;
}

/** Draw a maze.
Draws the player character and the maze surrounding it.
*/
GLDEF_C INT DrawMaze()
{
	INT n,na; // counters

	for (n = g_PlayerX-1; n<=g_PlayerX+1; n++)
	{
		for (na = g_PlayerY-1; na<=g_PlayerY+1; na++)
		{
			if ((n>=0) && (n<MAZEWIDTH))
			{
				if ((na>=0) && (na<MAZEHEIGHT))
				{
					Write(n,na,&g_Maze[n][na], 1);
				}
			}
		}
	}

	// Draw the player
	Write(g_PlayerX, g_PlayerY, "0", 1);

	return 0;
}


GLDEF_C VOID main ()
{
	InitConsole();
	RequestKeyb();

	LoadMaze(MAZEFILE, 1);

	DrawMaze();

	FOREVER
	{
		p_iowait();

		if (g_KeybStat!= E_FILE_PENDING)
		{
			// Handle keypress
			switch (g_KeyData.keycode)
			{
				case W_KEY_ESCAPE : p_exit(0); break; // quit game
				case W_KEY_LEFT : { // left
					if ((g_Maze[g_PlayerX-1][g_PlayerY]!='#') && (g_PlayerX-1>0))
						g_PlayerX--;
				} break;
				case W_KEY_RIGHT : { // right
					if ((g_Maze[g_PlayerX+1][g_PlayerY]!='#') && (g_PlayerX+1<MAZEWIDTH))
						g_PlayerX++;
				} break;
				case W_KEY_UP : { // up
					if ((g_Maze[g_PlayerX][g_PlayerY-1]!='#') && (g_PlayerY-1>0))
						g_PlayerY--;
				} break;
				case W_KEY_DOWN : { // down
					if ((g_Maze[g_PlayerX][g_PlayerY+1]!='#') && (g_PlayerY+1<MAZEHEIGHT))
						g_PlayerY++;
				} break;
			}

			DrawMaze();

			// test for goal
			if (g_Maze[g_PlayerX][g_PlayerY]=='E')
			{ // reached goal
				p_exit(0);

				// TODO : Replace p_exit(0) with LoadLevel(MAZEFILE, g_CurrentLevel++)
			}

			RequestKeyb();
		}
		else
		{
			p_printf("Stray Signal!\n");
		}
	}
}