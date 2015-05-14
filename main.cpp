#include "mem_check.h"

#pragma warning (disable: 4786)
#pragma warning (disable: 4788)
// disable warning (compare bool with int)
#pragma warning (disable: 4805)
#pragma warning (disable: 4503)

#include "keyboard.h"
#include "game.h"
#include "global.h"
#include <stdlib.h>
#include <curses.h>
#include <time.h>

#include <string>
using namespace std;

GAME game;

int main(int argc, char* argv[])
{
	if (argc>1)
	{
		game.name_of_player = argv[1];
		string letters="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";		
		for (int a=0;a<game.name_of_player.size();a++)
		{
			if (letters.find(game.name_of_player[a])==-1)
			{
				game.name_of_player = "";
				break;
			}
		}
	}

   game.Begin();
   game.Main();
   return 0;
}
