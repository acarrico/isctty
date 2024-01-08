AUTHOR: Anthony Carrico. This program is in the Public Domain.

The Posix procedure isatty tests whether a file descriptor refers to a terminal (see isatty(3)), but as far as I know there isn't a corresponding Posix procedure to check if a file descriptor refers to the process's controlling terminal.

bool isctty(int fd);

Test whether a file descriptor refers to the process's controlling terminal (ctty) under Linux.

dev_t get_proc_ctty_dev();

On Linux, return the device ID of the contolling terminal (ctty). This is not the same as the device ID of /dev/tty, which is always 5,0 under Linux, but if the /proc filesystem is not mounted, return 5,0 anyway (ISSUE: should fail in this case).
