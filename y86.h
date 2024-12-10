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

typedef enum STAT{
    AOK = 1,HLT,ADR,INS
} STATE;

struct Y86
{
    long long REG[15];
    // 0:rax 1:rcx 2:rdx 3:rbx 4:rsp 5:rbp 6:rsi 7:rdi 8:r8 9:r9 10:r10 11:r11 12:r12 13:r13 14:r14
    long long PC = 0;
    short ZF = 1;
    short SF = 0;
    short OF = 0;
    STATE state = AOK;
};

string to_hex(int n);

void save_instructions(string line);

void input();

void create_output();

void output();

void save_to_memory(long long addr, long long info);

long long fetch_from_memory(long long addr, long long offset);

// functions which realize the instructions
/*four integer operations*/
void addq(long long &x, long long &y);
void subq(long long &x, long long &y);
void andq(long long &x, long long &y);
void xorq(long long &x, long long &y);

/*seven jump instructions*/
void jmp(long long addr);
void jle(long long addr);
void jl(long long addr);
void je(long long addr);
void jne(long long addr);
void jge(long long addr);
void jg(long long addr);

/*ten move instructions*/
void rrmovq(long long &x, long long &y);
void irmovq(long long &x, long long &y);
void rmmovq(long long r, long long addr, long long offset);
void mrmovq(long long &addr, long long &r, long long offset);
void cmovle(long long &x, long long &y);
void cmovl(long long &x, long long &y);
void cmove(long long &x, long long &y);
void cmovne(long long &x, long long &y);
void cmovge(long long &x, long long &y);
void cmovg(long long &x, long long &y);

/*pushq & popq*/
void pushq(long long info);
void popq(long long &addr);

/*call &ret*/
void call(long long addr);
void ret();

/*halt*/
void STAT_HLT();

/*nop*/
void nop();

/*excepting instruction*/
void STAT_ADR();
void STAT_INS();

// fetch,decode,execute
unsigned char *fetch();

void decode_and_execute();

#endif