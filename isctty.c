/* AUTHOR: Anthony Carrico. This program is in the Public Domain. */

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

/* On Linux, return the device ID of the contolling terminal (ctty). This is not the same as the device ID of /dev/tty, which is always 5,0 under Linux, but if the /proc filesystem is not mounted, return 5,0 anyway (ISSUE: should fail in this case). */

dev_t get_proc_ctty_dev()
{
  /* NOTE: The seventh field of /proc/self/stat is called tty_nr and encodes the (pseudo)terminal device ID (see proc(5)). */

  /* NOTE: It is tempting to cache the device ID in a static variable, but initialization would be thread unsafe, and if the program starts a new session, the cache would be stale. */
  FILE *stat = fopen("/proc/self/stat", "r");
  if (stat == NULL)
    return makedev(5, 0);
  else
    {
      // see proc(5):
      int tty_nr;
      if (1 == fscanf(stat, "%*d %*s %*c %*d %*d %*d %d", &tty_nr))
        {
          fclose(stat);
          return  makedev(0xff & (tty_nr >> 8), (0xfff & (tty_nr >> 20)) | (0xff & tty_nr));
        }
      else
        {
          fclose(stat);
          return makedev(5, 0);
        }
    }
}

/* Test whether a file descriptor refers to the process's controlling terminal (ctty) under Linux.

   The Posix procedure isatty tests weather a file descriptor refers to a terminal (see isatty(3)), but as far as I know there isn't a corresponding Posix procedure to check if a file descriptor refers to the process's controlling terminal.

   ISSUE: What happens if a process in another process group opens /dev/tty and passes the file descriptor to you over a pipe, and then you call isctty on the received fd? Would this be a way to get an open fd to device 5,0 that doesn't actually connect to your ctty? Or does it magically now connect to your ctty? Which underlying (virtual) terminal does use for I/O? Curious right? */

bool isctty(int fd)
{
  /* First get the device ID of the fd */
  struct stat statbuf;
  if (fstat(fd, &statbuf) == -1)
    return false;

  /* Check if the device ID matches /dev/tty

     A portable way to get the device ID for /dev/tty is to open it and use stat, but on Linux this isn't necessary since it is defined to be 5,0. */
  if (statbuf.st_rdev == makedev(5, 0))
    return true;

  /* Check if the device ID matches the terminal aliased by /dev/tty */
  return (statbuf.st_rdev == get_proc_ctty_dev());
}

/* I'll move this into a library, but for now demo code follows. */

void dump(int fd)
{
  if (isatty(fd))
    {
      struct stat statbuf;
      fstat(fd, &statbuf);
      printf("%d is %s called %s device ID %d,%d\n",
             fd,
             isctty(fd) ? "the ctty" : "a tty",
             ttyname(fd),
             major(statbuf.st_rdev),
             minor(statbuf.st_rdev));
    };
}

int main ()
{
  printf("/proc/self/stat ctty dev is %d,%d\n",
         major(get_proc_ctty_dev()),
         minor(get_proc_ctty_dev()));
  printf("/dev/tty dev is 5,0\n");
  dump(0);
  dump(1);
  dump(2);
  dump(open("/dev/tty", O_RDWR));
}
