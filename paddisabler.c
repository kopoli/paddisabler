/*
  A very simplified clone of syndaemon. Upon keypress, disables a pointer
  device. The device can be given as an argument. For possible device names
  run: xinput list

  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


  Authors:
  Kalle Kankare <kalle.kankare@iki.fi>

 */

#define _BSD_SOURCE
#define _POSIX_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>

static int verbose=0;

static char *pointer_device = "Logitech USB Receiver";
static char *pointer_property = "Device Enabled";


static void dprintf(const char *format,...)
{
  va_list ap;

  if(!verbose)
    return;
    
  va_start(ap,format);
  vprintf(format,ap);
  va_end(ap);

  return;
}

/* use xinput program to enable and disable the pointer device */
static int pointer_toggle(int on)
{
  char buf[256];

  on=!!on;

  snprintf(buf,sizeof(buf),"xinput set-prop \"%s\" \"%s\" %d",
    pointer_device,pointer_property,on);

  dprintf("running command [%s]\n",buf);

  return system(buf);
}

/* probe if keyboard has been active */
static int keyboard_active(Display *disp)
{
  char map[32];
  int i;

  XQueryKeymap(disp,map);

  for(i=0;i<sizeof(map);i++)
    if(map[i] != 0)
      return 1;

  return 0;
}


/* quit and enable the pointer device */
static void sighandler(int num)
{
  pointer_toggle(1);

  dprintf("enabling pointer device\n");
  exit(1);
}

static void sighandler_install()
{
  int signals[] = {SIGHUP,SIGINT,SIGSEGV,SIGTERM,0};

  struct sigaction action;
  int i;

  action.sa_handler=sighandler;
  sigemptyset(&action.sa_mask);
  action.sa_flags=0;

  for(i=0;signals[i] != 0;i++)
    if(sigaction(signals[i],&action,NULL) < 0)
    {
      perror("sigaction");
      exit(1);
    }
}


int main_loop(Display *disp)
{
  int freezecount=0;
  int frozen=0;
  int act;

  sighandler_install();

  act=keyboard_active(disp);

  while(1)
  {
    usleep(200000);

    act=keyboard_active(disp);

    if(act)
    {
      if(!frozen)
      {
        pointer_toggle(0);
        dprintf("disabling mouse!\n");
      }

      frozen=1;
      freezecount=5; /* how long will the pointer be frozen */
    }

    if(frozen && --freezecount <= 0)
    {
      pointer_toggle(1);
      dprintf("enabling mouse!!\n");
      frozen=0;
    }
  }
}


int main(int argc, char *argv[])
{
  Display *disp;

  /* possible arguments: -v for verbose. every other is considered a xinput
     device name. The last will take effect. */
  if(argc > 1)
  {
    int i;
    for(i=1;i<argc;i++)
      if(strcmp(argv[i], "-v") == 0)
        verbose=1;
      else
        pointer_device=argv[i];
  }

  disp=XOpenDisplay(NULL);

  main_loop(disp);

  return 0;
}
