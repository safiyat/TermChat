TermChat
========

**TermChat** is a simple terminal based chat program for users logged into the same machine, using the curses library and shared-memory IPC.


### Pre-requisites

Ensure that your system has ncurses library installed.

In case it is not, run
   `sudo apt-get install libncurses5-dev` for Ubuntu-like systems and,

   `sudo dnf install ncurses-devel` for Fedora/Red Hat like systems.


### For compilation
Run the compiler with the `lcurses` or `lncurses` switch to link against the curses library.

On mac: `gcc -lncurses main.c`

Ensure that you add the switch after the name of the C program. This is because object files and libraries are linked in order in a single pass.
