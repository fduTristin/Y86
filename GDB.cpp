#include "gdb.h"

gdb GDB;
extern Y86 CPU;
extern unsigned char DMEM[MEM_SIZE];
extern string reg[15];

void gdb::output_line()
{
    std::cout << CLEAR_SCREEN;
    // 0:rax 1:rcx 2:rdx 3:rbx 4:rsp 5:rbp 6:rsi 7:rdi 8:r8 9:r9 10:r10 11:r11 12:r12 13:r13 14:r14
    cout << "|--------------------|        |";
    for (int j = 0; j != 32; j++)
    {
        if (GDB.memory[j] != DMEM[j])
        {
            GDB.memory[j] = DMEM[j];
            cout << WHITE_BG << BLACK_TEXT;
        }

        cout << to_hex(GDB.memory[j]) << RESET << '|';
    }
    cout << '\n';
    for (int i = 0; i != 15; i++)
    {
        cout << "|";
        if (GDB.REG[i] != CPU.REG[i])
        {
            cout << WHITE_BG << BLACK_TEXT;
            GDB.REG[i] = CPU.REG[i];
        }
        cout << reg[i] << " " << GDB.REG[i] << string(16 - to_string(GDB.REG[i]).size(), ' ') << RESET << "|";
        cout << "        |";
        for (int j = 64 * i + 32; j != 64 * (i + 1); j++)
        {
            if (GDB.memory[j] != DMEM[j])
            {
                GDB.memory[j] = DMEM[j];
                cout << WHITE_BG << BLACK_TEXT;
            }

            cout << to_hex(GDB.memory[j]) << RESET << '|';
        }
        cout << '\n';
        cout << "|--------------------|        |";
        for (int j = 64 * (i + 1); j != 64 * i + 96; j++)
        {
            if (GDB.memory[j] != DMEM[j])
            {
                GDB.memory[j] = DMEM[j];
                cout << WHITE_BG << BLACK_TEXT;
            }

            cout << to_hex(GDB.memory[j]) << RESET << '|';
        }
        cout << '\n';
    }
    cout << string(30, ' ') << '|';
    for (int j = 64 * 15 + 32; j != 64 * 16; j++)
    {
        if (GDB.memory[j] != DMEM[j])
        {
            GDB.memory[j] = DMEM[j];
            cout << WHITE_BG << BLACK_TEXT;
        }

        cout << to_hex(GDB.memory[j]) << RESET << '|';
    }
    cout << endl;
    cout << "STAT:";
    if (CPU.state != GDB.state)
    {
        GDB.state = CPU.state;
        cout << WHITE_BG << BLACK_TEXT;
    }
    cout << GDB.state << RESET;
    cout << " PC:";
    if (CPU.PC != GDB.PC)
    {
        GDB.PC = CPU.PC;
        cout << WHITE_BG << BLACK_TEXT;
    }
    cout << GDB.PC << RESET;
    cout << " ZF:";
    if (CPU.ZF != GDB.ZF)
    {
        GDB.ZF = CPU.ZF;
        cout << WHITE_BG << BLACK_TEXT;
    }
    cout << GDB.ZF << RESET;
    cout << " SF:";
    if (CPU.SF != GDB.SF)
    {
        GDB.SF = CPU.SF;
        cout << WHITE_BG << BLACK_TEXT;
    }
    cout << GDB.SF << RESET;
    cout << " OF:";
    if (CPU.OF != GDB.OF)
    {
        GDB.OF = CPU.OF;
        cout << WHITE_BG << BLACK_TEXT;
    }
    cout << GDB.OF << RESET;

    cout << endl;

    if (GDB.breakpoint[CPU.PC])
    {
        cout << "breakpoint at:" << CPU.PC << endl;
        hit[CPU.PC] = true;
    }
}

void gdb::run()
{
    while (!(GDB.breakpoint[CPU.PC] && !hit[CPU.PC]) && CPU.state == 1)
    {
        hit[CPU.PC] = false;
        decode_and_execute();
    }
    GDB.output_line();
}