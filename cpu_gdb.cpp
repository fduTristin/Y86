#include "y86.h"
#include "gdb.h"
#include "cache.h"

extern Y86 CPU;
extern unsigned char DMEM[MEM_SIZE];
extern gdb GDB;
extern cache_ cache;

extern int hits;
extern int misses;
extern int evictions;

int main()
{
    input();
    string cache_or_not;
    cout << "use a cache(yes/no): ";
    cin >> cache_or_not;
    char ins;
    long long bp;
    if (cache_or_not == "no")
        while (CPU.state == 1)
        {
            cin >> ins;
            if (ins == 'b')
            {
                cin >> bp;
                GDB.breakpoint[bp] = 1;
            }
            if (ins == 'r')
                GDB.run();
            if (ins == 's')
            {
                decode_and_execute();
                GDB.output_line();
            }
        }
    else
    {
        initialize_cache(8, 4, 8, cache);
        while (CPU.state == 1)
        {
            cin >> ins;
            if (ins == 'b')
            {
                cin >> bp;
                GDB.breakpoint[bp] = 1;
            }
            else if (ins == 'c')
            {
                while (!(GDB.breakpoint[CPU.PC] && !GDB.hit[CPU.PC]) && CPU.state == 1)
                {
                    GDB.hit[CPU.PC] = false;
                    decode_and_execute(cache);
                }
                GDB.output_line();
                cout << "hit:" << hits << " misses:" << misses << " evictions:" << evictions << endl;
            }
            else if (ins == 's')
            {
                decode_and_execute(cache);
                GDB.output_line();
                cout << "hit:" << hits << " misses:" << misses << " evictions:" << evictions << endl;
            }
            else if (ins == 'h')
            {
                cout << CLEAR_SCREEN << "Usage:\n";
                cout << "Options:\n";
                cout << "b <addr> : set a breakpoint at addr(decimal)\n";
                cout << "c : run till the first breakpoint / the end of program\n";
                cout << "s : execute one instruction\n";
            }
            else
            {
                cout << CLEAR_SCREEN << "type h for help\n";
            }
        }
    }
}
