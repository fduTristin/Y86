#ifndef CACHE_H
#define CACHE_H

#include "y86.h"
#define MININT -2147483648

extern Y86 CPU;

extern unsigned char DMEM[MEM_SIZE];

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

void initialize_cache(int s, int e, int b, cache_ &c);

int full(int s);

int find(int tag, int s);

int find_LRU(int s);

long long update_block(int idx, int s, int tag);

long long update(char op, int tag, int s);

void mrmovq(long long addr, long long &r, long long offset, cache_ &ca);

void rmmovq(long long r, long long addr, long long offset, cache_ &ca);

void decode_and_execute(cache_ &ca);

#endif