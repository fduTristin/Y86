#include "y86.h"
#define MININT -2147483648

// cache parameters
struct line
{
    int valid;
    int tag;
    int recently_used;
    long long block;
};

struct cache_
{
    int S;
    int E;
    int B;
    line **cache_line;
};
static cache_ cache;

void initialize_cache(int s, int e, int b, cache_ &c)
{
    c.S = s;
    c.E = e;
    c.B = b;
    c.cache_line = new line *[s];
    for (int i = 0; i != c.S; i++)
    {
        c.cache_line[i] = new line[e];
        for (int j = 0; j != c.E; j++)
        {
            c.cache_line[i][j].tag = MININT;
            c.cache_line[i][j].valid = 0;
            c.cache_line[i][j].recently_used = 0;
        }
    }
}

// final results
static int hits = 0;
static int misses = 0;
static int evictions = 0;

int full(int s)
{
    for (int i = 0; i != cache.E; i++)
    {
        if (cache.cache_line[s][i].valid == 0)
            return i;
    }
    return -1;
}

int find(int tag, int s)
{
    for (int i = 0; i != cache.E; i++)
    {
        if (cache.cache_line[s][i].tag == tag)
            return i;
    }
    return -1;
}

int find_LRU(int s)
{
    int idx = 0;
    for (int i = 0; i != cache.E; i++)
    {
        if (cache.cache_line[s][i].recently_used > cache.cache_line[s][idx].recently_used)
            idx = i;
    }
    return idx;
}

long long update_block(int idx, int s, int tag)
{
    for (int i = 0; i != cache.E; i++)
    {
        if (i != idx)
        {
            if (cache.cache_line[s][i].valid == 1)
                cache.cache_line[s][i].recently_used++;
        }
        else
        {
            cache.cache_line[s][i].tag = tag;
            cache.cache_line[s][i].recently_used = 0;
            cache.cache_line[s][i].valid = 1;
            long long addr = tag * cache.B * cache.S + s * cache.B;
            cache.cache_line[s][i].block = fetch_from_memory(addr, 0);
        }
    }
    return cache.cache_line[s][idx].block;
}

long long update(char op, int tag, int s)
{
    int idx = find(tag, s);
    if (idx != -1)
    {
        if (cache.cache_line[s][idx].valid == 1)
        {
            hits++;
            return cache.cache_line[s][idx].block;
        }

        else
        {
            misses++;
            return update_block(idx, s, tag);
        }
    }
    else
    {
        int id = full(s);
        if (id != -1)
        {
            misses++;
            return update_block(id, s, tag);
        }
        else
        {
            id = find_LRU(s);
            misses++;
            evictions++;
            return update_block(id, s, tag);
        }
    }
    if (op == 'M')
    {
        hits++;
    }
}

void mrmovq(long long addr, long long &r, long long offset, cache_ &ca)
{
    int tag = (addr + offset) / (cache.S * cache.B);
    int set = ((addr + offset) / cache.B) % cache.S;
    r = update('L', tag, set);
}

void rmmovq(long long r, long long addr, long long offset, cache_ &ca)
{
    save_to_memory(addr + offset, r);
    int tag = (addr + offset) / (cache.S * cache.B);
    int set = ((addr + offset) / cache.B) % cache.S;
    update('S', tag, set);
}

void decode_and_execute(cache_ &ca)
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
                rmmovq(CPU.REG[rA], CPU.REG[rB], offset, cache);
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
                mrmovq(CPU.REG[rB], CPU.REG[rA], offset, cache);
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