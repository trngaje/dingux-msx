/*

Copyright 2005-2011 - Ludovic Jacomme - All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY LUDOVIC JACOMME 'AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUDOVIC JACOMME OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Ludovic Jacomme.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include "SDL.h"

#include "global.h"
#include "psp_kbd.h"
#include "psp_sdl.h"
#include "psp_menu.h"
#include "psp_menu_help.h"

  static int psp_menu_dirty = 1;

# define MAX_HELP_LINE    4096 

# define HELP_LINE_BY_PAGE   24
# define HELP_CHAR_BY_LINE   53

  static char* psp_help[MAX_HELP_LINE];
  static int   psp_help_size    = -1;
  static int   psp_help_current = 0;

static void
psp_initialize_help(void)
{
  char  FileName[MAX_PATH+1];

  char  Buffer[512];
  char *Scan;
  FILE* FileDesc;

  /* Already done ? */
  if (psp_help_size > 0) return; 

#ifdef KORFONT
  strcpy(FileName, "./miyoomini-kor-msx.man.txt");
#else
  strcpy(FileName, "./dingux-msx.man.txt");
#endif
  FileDesc = fopen(FileName, "r");

  psp_help_current = 0;

  if (FileDesc == (FILE *)0 ) {
    psp_help[0] = strdup( "no help file found !");
    psp_help_size = 1;
    return;
  }

  psp_help_size = 0;
  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';

    psp_help[psp_help_size++] = strdup(Buffer);
    if (psp_help_size >= MAX_HELP_LINE) break;
  }
  fclose(FileDesc);
}

static void 
psp_display_screen_help(void)
{
  char buffer[512];

  int help_line = 0;
  int index     = 0;

  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  if (psp_menu_dirty) 
  {
    psp_sdl_blit_background();
    //psp_menu_dirty = 0;
  }

  x      = 0;
  y      = 0;
#ifdef KORFONT
  y_step = 10+2;
#else
  y_step = 10;
#endif

  help_line = psp_help_current;
  index     = 0;
  
  while ((index < HELP_LINE_BY_PAGE) && (help_line < psp_help_size))  {
    strcpy(buffer, psp_help[help_line]);
    string_fill_with_space(buffer, HELP_CHAR_BY_LINE);
    psp_sdl_back2_print(x, y, buffer, PSP_MENU_SEL_COLOR);
    y += y_step;
    index++;
    help_line++;
  }

  if (index != HELP_LINE_BY_PAGE) {
    buffer[0]=0;
    string_fill_with_space(buffer, HELP_CHAR_BY_LINE);
    while (index < HELP_LINE_BY_PAGE) {
      psp_sdl_back2_print(x, y, buffer, PSP_MENU_SEL_COLOR);
      y += y_step;
      index++;
    }
  }
}

int 
psp_help_menu()
{
  gp2xCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  psp_kbd_wait_no_button();

  psp_initialize_help();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  psp_menu_dirty = 1;

  while (! end_menu)
  {
    psp_display_screen_help();
    psp_sdl_flip();

    while (1)
    {
      gp2xCtrlReadBufferPositive(&c, 1);
      c.Buttons &= PSP_ALL_BUTTON_MASK;

      if (c.Buttons) break;
    }

    new_pad = c.Buttons;

    if ((old_pad != new_pad) || ((c.TimeStamp - last_time) > PSP_MENU_MIN_TIME)) {
      last_time = c.TimeStamp;
      old_pad = new_pad;

    } else continue;

    if(new_pad & GP2X_CTRL_SELECT) {
      /* Back to Main menu */
      end_menu = 1;
    } else
    if(new_pad & GP2X_CTRL_UP) {
      if (psp_help_current > 0) psp_help_current--;
    } else
    if(new_pad & GP2X_CTRL_DOWN) {
      if ((psp_help_current + 1) < psp_help_size) psp_help_current++;
    } else
    if(new_pad & GP2X_CTRL_LEFT) {
      if (psp_help_current > HELP_LINE_BY_PAGE) psp_help_current -= HELP_LINE_BY_PAGE;
      else                                      psp_help_current  = 0;
    } else
    if(new_pad & GP2X_CTRL_RIGHT) {
      if ((psp_help_current + HELP_LINE_BY_PAGE + 1) < psp_help_size) psp_help_current += HELP_LINE_BY_PAGE;
    }
  }
 
  psp_kbd_wait_no_button();

  return 1;
}

