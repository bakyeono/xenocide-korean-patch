#include "system.h"
#include <curses.h>
#include <time.h>

void make_screenshot(string filename,bool put_date)
{
	unsigned long colors[16] = { 		
			0x00c0c0c0,
			0x000000d2,
			0x0000c850,
			0x000064b4,
			0x00c80000,
			0x00800080,
			0x00803A00,
			0x00b0b0b0,
			0x00646464,
			0x000000ff,
			0x0000ff00,
			0x0000ffff,
			0x00f06464,
			0x00ff00ff,
			0x00ffff00,
			0x00ffffff };		


	FILE *fp;
	char character;
	int color,last_color=-1;
	int x,y;
	bool new_file = true;
	
	fp = fopen(string(filename + ".htm").c_str(),"r");
	if (fp!=NULL)
	{
		new_file = false;
		fclose(fp);
	}
	
	fp = fopen(string(filename + ".htm").c_str(),"at");
	if (fp==NULL)
		return;

	if (new_file && put_date)
	{
		fprintf(fp,"<html><head><title> Xenocide Screenshots </title></head>\n<body BGCOLOR=#000000 STYLE=\"line-height: 11pt\">\n");
	}

	fprintf(fp,"<hr>\n");

	struct tm *newtime;
	time_t aclock;
	
	time( &aclock );                 /* Get time in seconds */
	newtime = localtime( &aclock );  /* Convert time to struct */

	if (put_date)
	{
		fprintf(fp, "<center><h3 style=\"color:white;\">Screenshot taken on: %s</h3></center>\n", asctime( newtime ) );
		fprintf(fp,"<hr>\n");
	}
	

	fprintf(fp,"<table BORDER=1 CELLPADDING=0 CELLSPACING=0 ALIGN=center BORDERCOLOR=#444444><td>\n");
	fprintf(fp,"<font FACE=courier SIZE = 2><b>\n");
	
	for (y=0;y<LINES;y++)
	{
		for (x=0;x<COLS;x++)
		{
			character = get_char_from_screen(x,y);
			color = get_color_from_screen(x,y);
			if (color!=last_color)
			{
				if (last_color!=-1)

				{
					if (last_color<16)
						fprintf(fp,"</font>");
					else
					{
						fprintf(fp,"</b>");
					}
				}
				if (color<16)
				{
					fprintf(fp,"<font color=#%06x>",colors[color]);
				}
				else if (color>=16 && color<32)
				{  
					
					fprintf(fp,"<b style=\"color:#%06x;background-color:#%06x\">",colors[(color-16<8 ? color-8: color-16)],colors[4]);
				}
				else if (color>=32)
				{  
					
					fprintf(fp,"<b style=\"color:#%06x;background-color:#%06x\">",colors[(color-16<8 ? color-8: color-16)],colors[1]);
				}
			}
			// print of character
			last_color = color;
			if (character!=' ')				
				fprintf(fp,"%c",character);
			else
				fprintf(fp,"&nbsp;");
			
		}
		fprintf(fp,"<BR>\n");
	}
	fprintf(fp,"</td></table>\n");
	fflush(fp);	
	fclose(fp);
}

