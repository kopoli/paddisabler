#define _BSD_SOURCE
#define _POSIX_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <signal.h>

#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>

static char *pointer_device = "Logitech USB Receiver";
static char *pointer_property = "Device Enabled";

static int pointer_toggle(int on)
{
  char buf[256];

  on=!!on;

  snprintf(buf,sizeof(buf),"xinput set-prop \"%s\" \"%s\" %d",
    pointer_device,pointer_property,on);

  printf("running command [%s]\n",buf);

  return system(buf);
}

static void ioprint(char *buf,size_t size)
{
  int i;
  for(i=0;i<size;i++)
    printf("%02x%c",(char)buf[i],(i+1) % 20 ? ' ' : '\n');
  puts("");
}

static int keyboard_active(Display *disp)
{
  static char buf1[32],buf2[32];
  static char *map_old=buf1,*map_new=buf2;
  int beta;
  int ret=0;
  char *tmp;

  XQueryKeymap(disp,map_new);


  for(beta=0;beta<sizeof(buf1);beta++)
    if(map_new[beta] != 0)
    {
      ret=1;
      break;
    }
  /*
  if(memcmp(map_new,map_old,sizeof(buf1)) != 0)
    ret=1;
  */

  /*
  for(beta=0;beta<sizeof(buf1);beta++)
    if(map_new[beta] != map_old[beta])
    {
      ret=1;
      break;
    }
  */
  /*
  printf("old: %p\n",map_old);
  ioprint(map_old,sizeof(buf1));

  printf("new: %p\n", map_new);
  ioprint(map_new,sizeof(buf1));
  */

  tmp=map_old;
  map_old=map_new;
  map_new=tmp;

  return ret;
}

static void sighandler(int num)
{
  pointer_toggle(1);

  printf("enabling pointer device\n");
  exit(1);
}

static void sighandler_install()
{
  int signals[] = {
    SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT,
    SIGBUS, SIGFPE, SIGUSR1, SIGSEGV, SIGUSR2, SIGPIPE,
    SIGALRM, SIGTERM,
  };

  struct sigaction act;
  sigset_t set;
  int i;

  sigemptyset(&set);
  act.sa_handler=sighandler;
  act.sa_mask=set;
  act.sa_flags=0;

  for(i=0;i<sizeof(signals)/sizeof(*signals);i++)
    if(sigaction(signals[i],&act,NULL) < 0)
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
        printf("disabling mouse!\n");
      }

      frozen=1;
      freezecount=5;
    }

    if(frozen && --freezecount <= 0)
    {
      pointer_toggle(1);
      printf("enabling mouse!!\n");
      frozen=0;
    }
  }
}


int main(int argc, char *argv[])
{
  Display *disp;

  if(argc > 1)
    pointer_device=argv[1];

  disp=XOpenDisplay(NULL);

  main_loop(disp);

  return 0;
}
