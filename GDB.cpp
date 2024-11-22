#include "y86.h"
#include "cache.h"

int main()
{
    input();
    string cache_or_not;
    cout << "use a cache(yes/no):";
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
            if (ins == 'r')
            {
                while (!(GDB.breakpoint[CPU.PC] && !GDB.hit[CPU.PC]) && CPU.state == 1)
                {
                    GDB.hit[CPU.PC] = false;
                    decode_and_execute(cache);
                }
                GDB.output_line();
                cout << "hit:" << hits << " misses:" << misses << " evictions:" << evictions << endl;
            }
            if (ins == 's')
            {
                decode_and_execute(cache);
                GDB.output_line();
                cout << "hit:" << hits << " misses:" << misses << " evictions:" << evictions << endl;
            }
        }
    }
}
