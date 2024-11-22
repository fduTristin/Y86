#ifndef Y86_H
#define Y86_H

#include <string>
#include <fstream>
#include <regex>
#include <iostream>
#include <functional>
#include <vector>
#include "json.hpp"
#define MEM_SIZE 0x400
#define WHITE_BG "\033[48;5;15m"     // 白色背景
#define GREEN_BG "\033[1;42m"        // 绿色背景
#define BLACK_TEXT "\033[38;5;0m"    // 黑色前景色
#define WHITE_TEXT "\033[1;37m"      // 白色前景色
#define CLEAR_SCREEN "\033[2J\033[H" // 清空屏幕并将光标移动到起始位置
#define RESET "\033[0m"

using namespace std;

static char *infile;
static char outfile[] = "result.json";
static nlohmann::json DATA; // output array

string reg[15] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8 ", "r9 ", "r10", "r11", "r12", "r13", "r14"};

struct Y86
{
    long long REG[15];
    // 0:rax 1:rcx 2:rdx 3:rbx 4:rsp 5:rbp 6:rsi 7:rdi 8:r8 9:r9 10:r10 11:r11 12:r12 13:r13 14:r14
    long long PC = 0;
    short ZF = 1;
    short SF = 0;
    short OF = 0;
    // state
    // AOK = 1,
    // HLT = 2
    // ADR = 3
    // INS = 4
    int state = 1;
};

static Y86 CPU;

static unsigned char DMEM[MEM_SIZE];

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

static gdb GDB;

string to_hex(int n)
{
    string s;
    int b03 = n & 15;
    int b47 = n >> 4;
    char s0 = b03 <= 9 ? '0' + b03 : 'A' + (b03 - 10);
    char s1 = b47 <= 9 ? '0' + b47 : 'A' + (b47 - 10);
    s.push_back(s0);
    s.push_back(s1);
    return s;
}

extern long long fetch_from_memory(long long, long long);

void save_instructions(string line)
{
    if (line[0] != '0')
        return;
    int addr = 0, i = 2;
    while (line[i] != ':')
    {
        addr *= 16;
        addr += line[i] >= '0' && line[i] <= '9' ? line[i] - '0' : (line[i] - 'a') + 10;
        i++;
    }
    i += 2;
    int offset = 0;
    while (line[i] != ' ')
    {
        int byte;
        byte = line[i] >= '0' && line[i] <= '9' ? line[i] - '0' : (line[i] - 'a') + 10;
        byte *= 16;
        i++;
        byte += (line[i] >= '0' && line[i] <= '9' ? line[i] - '0' : (line[i] - 'a') + 10);
        DMEM[addr + offset] = byte;
        i++;
        offset++;
    }
}

void input()
{
    string line;
    while (getline(cin, line))
    {
        if (line[0] != '0' && line[0] != ' ')
            return;
        save_instructions(line);
    }
}

void create_output()
{
    static int count = 0;
    nlohmann::json mem;
    for (int i = 0; i <= MEM_SIZE - 8; i += 8)
    {
        string s = to_string(i);
        long long val = fetch_from_memory(i, 0);
        if (val)
            mem[s] = val;
    }

    nlohmann::json data =
        {{"PC", CPU.PC},
         {"REG", {{"rax", CPU.REG[0]}, {"rcx", CPU.REG[1]}, {"rdx", CPU.REG[2]}, {"rbx", CPU.REG[3]}, {"rsp", CPU.REG[4]}, {"rbp", CPU.REG[5]}, {"rsi", CPU.REG[6]}, {"rdi", CPU.REG[7]}, {"r8", CPU.REG[8]}, {"r9", CPU.REG[9]}, {"r10", CPU.REG[10]}, {"r11", CPU.REG[11]}, {"r12", CPU.REG[12]}, {"r13", CPU.REG[13]}, {"r14", CPU.REG[14]}}},
         {"CC", {{"ZF", CPU.ZF}, {"SF", CPU.SF}, {"OF", CPU.OF}}},
         {"STAT", CPU.state},
         {"MEM", mem}};
    DATA.push_back(data);
}

void output()
{
    cout << std::setw(4) << DATA << endl;
}

void save_to_memory(long long addr, long long info)
{
    for (int i = 0; i != 8; i++)
    {
        DMEM[addr + i] = (info >> (8 * i)) & (0xff);
    }
}

long long fetch_from_memory(long long addr, long long offset)
{
    long long info = 0;

    for (int j = 0; j != 8; j++)
    {
        info += ((long long)DMEM[addr + offset + j]) << (8 * j);
    }

    return info;
}

// functions which realize the instructions
/*four integer operations*/
void addq(long long &x, long long &y)
{
    long long t = x + y;
    CPU.ZF = t == 0;
    CPU.SF = t < 0;
    CPU.OF = (x < 0 == y < 0) && (t < 0 != x < 0);
    y = t;
}
void subq(long long &x, long long &y)
{
    long long t = y - x;
    CPU.ZF = t == 0;
    CPU.SF = t < 0;
    CPU.OF = (t < y) != (x > 0);
    y = t;
}
void andq(long long &x, long long &y)
{
    long long t = y & x;
    y = t;
    CPU.ZF = t == 0;
    CPU.SF = t < 0;
}
void xorq(long long &x, long long &y)
{
    long long t = y ^ x;
    y = t;
    CPU.ZF = t == 0;
    CPU.SF = t < 0;
}

/*seven jump instructions*/
void jmp(long long addr)
{
    CPU.PC = addr;
}
void jle(long long addr)
{
    if ((CPU.SF ^ CPU.OF) | CPU.ZF)
        CPU.PC = addr;
    else
        CPU.PC += 9;
}
void jl(long long addr)
{
    if (CPU.SF ^ CPU.OF)
        CPU.PC = addr;
    else
        CPU.PC += 9;
}
void je(long long addr)
{
    if (CPU.ZF)
        CPU.PC = addr;
    else
        CPU.PC += 9;
}
void jne(long long addr)
{
    if (!CPU.ZF)
        CPU.PC = addr;
    else
        CPU.PC += 9;
}
void jge(long long addr)
{
    if (!(CPU.SF ^ CPU.OF))
        CPU.PC = addr;
    else
        CPU.PC += 9;
}
void jg(long long addr)
{
    if (!(CPU.SF ^ CPU.OF) & !CPU.ZF)
        CPU.PC = addr;
    else
        CPU.PC += 9;
}

/*ten move instructions*/
void rrmovq(long long &x, long long &y)
{
    y = x;
}
void irmovq(long long &x, long long &y)
{
    y = x;
}
void rmmovq(long long r, long long addr, long long offset)
{
    save_to_memory(addr + offset, r);
}
void mrmovq(long long &addr, long long &r, long long offset)
{
    r = fetch_from_memory(addr, offset);
}
void cmovle(long long &x, long long &y)
{
    if ((CPU.SF ^ CPU.OF) | CPU.ZF)
        y = x;
}
void cmovl(long long &x, long long &y)
{
    if (CPU.SF ^ CPU.OF)
        y = x;
}
void cmove(long long &x, long long &y)
{
    if (CPU.ZF)
        y = x;
}
void cmovne(long long &x, long long &y)
{
    if (!CPU.ZF)
        y = x;
}
void cmovge(long long &x, long long &y)
{
    if (!(CPU.SF ^ CPU.OF))
        y = x;
}
void cmovg(long long &x, long long &y)
{
    if (!(CPU.SF ^ CPU.OF) & !CPU.ZF)
        y = x;
}

/*pushq & popq*/
void pushq(long long info)
{
    CPU.REG[4] -= 8;
    save_to_memory(CPU.REG[4], info);
}
void popq(long long &addr)
{
    CPU.REG[4] += 8;
    addr = fetch_from_memory(CPU.REG[4] - 8, 0);
}

/*call &ret*/
void call(long long addr)
{
    long long Addr = CPU.PC + 9;
    pushq(Addr);
    CPU.PC = addr;
}
void ret()
{
    long long Addr;
    popq(Addr);
    CPU.PC = Addr;
}

/*halt*/
void halt()
{
    CPU.state = 2;
}

/*nop*/
void nop() {}

/*excepting instruction*/
void ADR()
{
    CPU.state = 3;
}
void INS()
{
    CPU.state = 4;
}

// fetch,decode,execute
unsigned char *fetch()
{
    return DMEM + CPU.PC;
}

void decode_and_execute()
{
    unsigned char *code = fetch();
    int rA, rB, f;
    long long val, dest, offset;
    switch (code[0])
    {
    case 0x0:
        halt();
        break;

    case 0x10:
        nop();
        CPU.PC += 1;
        break;

    case 0x20:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14 && rA != rB)
        {
            rrmovq(CPU.REG[rA], CPU.REG[rB]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    case 0x21:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14 && rA != rB)
        {
            cmovle(CPU.REG[rA], CPU.REG[rB]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    case 0x22:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14 && rA != rB)
        {
            cmovl(CPU.REG[rA], CPU.REG[rB]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    case 0x23:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14 && rA != rB)
        {
            cmove(CPU.REG[rA], CPU.REG[rB]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    case 0x24:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14 && rA != rB)
        {
            cmovne(CPU.REG[rA], CPU.REG[rB]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    case 0x25:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14 && rA != rB)
        {
            cmovge(CPU.REG[rA], CPU.REG[rB]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    case 0x26:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14 && rA != rB)
        {
            cmovg(CPU.REG[rA], CPU.REG[rB]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    case 0x30:
        f = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (f == 0xf && rB >= 0 && rB <= 14)
        {
            val = fetch_from_memory(CPU.PC, 2);
            irmovq(val, CPU.REG[rB]);
            CPU.PC += 10;
        }
        else
            INS();
        break;

    case 0x40:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14 && rA != rB)
        {
            offset = fetch_from_memory(CPU.PC, 2);
            if (CPU.REG[rB] + offset < MEM_SIZE - 8)
            {
                rmmovq(CPU.REG[rA], CPU.REG[rB], offset);
                CPU.PC += 10;
            }
            else
                ADR();
        }
        else
            INS();
        break;

    case 0x50:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14 && rA != rB)
        {
            offset = fetch_from_memory(CPU.PC, 2);
            if (CPU.REG[rB] + offset < MEM_SIZE - 8)
            {
                mrmovq(CPU.REG[rB], CPU.REG[rA], offset);
                CPU.PC += 10;
            }
            else
                ADR();
        }
        else
            INS();
        break;

    case 0x60:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14)
        {
            addq(CPU.REG[rA], CPU.REG[rB]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    case 0x61:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14)
        {
            subq(CPU.REG[rA], CPU.REG[rB]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    case 0x62:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14)
        {
            andq(CPU.REG[rA], CPU.REG[rB]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    case 0x63:
        rA = (code[1] >> 4) & 0xf;
        rB = code[1] & 0xf;
        if (rA >= 0 && rA <= 14 && rB >= 0 && rB <= 14)
        {
            xorq(CPU.REG[rA], CPU.REG[rB]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    case 0x70:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jmp(dest);
        else
            ADR();
        break;

    case 0x71:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jle(dest);
        else
            ADR();
        break;

    case 0x72:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jl(dest);
        else
            ADR();
        break;

    case 0x73:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            je(dest);
        else
            ADR();
        break;

    case 0x74:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jne(dest);
        else
            ADR();
        break;

    case 0x75:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jge(dest);
        else
            ADR();
        break;

    case 0x76:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jg(dest);
        else
            ADR();
        break;

    case 0x80:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            call(dest);
        else
            ADR();
        break;

    case 0x90:
        ret();
        break;

    case 0xA0:
        rA = (code[1] >> 4) & 0xf;
        f = code[1] & 0xf;
        if (f == 0xf && rA >= 0 && rA <= 14)
        {
            pushq(CPU.REG[rA]);
            if (CPU.REG[4] >= 8 && CPU.REG[4] <= MEM_SIZE)
            {

                CPU.PC += 2;
            }
            else
                ADR();
        }
        else
            INS();
        break;

    case 0xB0:
        rA = (code[1] >> 4) & 0xf;
        f = code[1] & 0xf;
        if (f == 0xf && rA >= 0 && rA <= 14)
        {
            popq(CPU.REG[rA]);
            CPU.PC += 2;
        }
        else
            INS();
        break;

    default:
        INS();
    }
}

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

#endif