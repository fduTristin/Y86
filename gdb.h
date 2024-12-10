# ifndef GDB_H
# define GDB_H

#include"y86.h"

struct gdb
{
    int breakpoint[MEM_SIZE] = {0};
    bool hit[MEM_SIZE] = {0};
    int line = 0;
    void output_line(); // output the line of the breakpoint
    void run();         // run to breakpoint or the end
    long long REG[15];
    long long PC = 0;
    short ZF = 1;
    short SF = 0;
    short OF = 0;
    int state = 1;
    unsigned char memory[MEM_SIZE];
};

# endif