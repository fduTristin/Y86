#include "y86.h"

char inputfile[] = "test/new.yo";
char outputfile[] = "output/result.json";
nlohmann::json DATA; // output array
Y86 CPU;
unsigned char DMEM[MEM_SIZE];
string reg[15] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8 ", "r9 ", "r10", "r11", "r12", "r13", "r14"};

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
    fstream infile(inputfile);
    string line;
    while (getline(infile, line))
    {
        if (line[0] != '0' && line[0] != ' ')
            return;
        save_instructions(line);
    }
}

void create_output()
{
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
    ofstream outfile(outputfile);
    outfile << std::setw(4) << DATA << endl;
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
void STAT_HLT()
{
    CPU.state = HLT;
}

/*nop*/
void nop() {}

/*excepting instruction*/
void STAT_ADR()
{
    CPU.state = ADR;
}
void STAT_INS()
{
    CPU.state = INS;
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
        STAT_HLT();
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
            STAT_INS();
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
            STAT_INS();
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
            STAT_INS();
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
            STAT_INS();
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
            STAT_INS();
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
            STAT_INS();
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
            STAT_INS();
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
            STAT_INS();
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
                STAT_ADR();
        }
        else
            STAT_INS();
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
                STAT_ADR();
        }
        else
            STAT_INS();
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
            STAT_INS();
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
            STAT_INS();
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
            STAT_INS();
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
            STAT_INS();
        break;

    case 0x70:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jmp(dest);
        else
            STAT_ADR();
        break;

    case 0x71:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jle(dest);
        else
            STAT_ADR();
        break;

    case 0x72:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jl(dest);
        else
            STAT_ADR();
        break;

    case 0x73:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            je(dest);
        else
            STAT_ADR();
        break;

    case 0x74:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jne(dest);
        else
            STAT_ADR();
        break;

    case 0x75:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jge(dest);
        else
            STAT_ADR();
        break;

    case 0x76:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            jg(dest);
        else
            STAT_ADR();
        break;

    case 0x80:
        dest = fetch_from_memory(CPU.PC, 1);
        if (dest < MEM_SIZE)
            call(dest);
        else
            STAT_ADR();
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
                STAT_ADR();
        }
        else
            STAT_INS();
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
            STAT_INS();
        break;

    default:
        STAT_INS();
    }
}
