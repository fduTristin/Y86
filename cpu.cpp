#include "y86.h"

extern Y86 CPU;

extern unsigned char DMEM[MEM_SIZE];

int main()
{
    input();
    while (CPU.state == 1)
    {
        decode_and_execute();
        create_output();
    }
    output();
}
