#include "y86.h"
#include <conio.h> // 使用 _getch() 函数

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
